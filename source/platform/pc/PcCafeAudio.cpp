#include "platform/pc/PcAudio.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcVoice.hpp"

#include "game/core/CriticalSection.hpp"

#include <map>
#include <math.h>
#include <pthread.h>
#include <sndcore2/core.h>
#include <sndcore2/voice.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

namespace
{
const double kRendererRate = 32000.0;
const unsigned int kOutputRate = 48000;
const unsigned int kFramesPerBuffer = 1024;
const unsigned int kMaxVoices = 96;
const float kUnityVolume = 32768.0f;
const float kDownmix = 0.70710678f;

enum TvChannel
{
    eTvChannel_FrontLeft,
    eTvChannel_FrontRight,
    eTvChannel_RearLeft,
    eTvChannel_RearRight,
    eTvChannel_Centre,
    eTvChannel_Lfe,
    eTvChannel_Count
};

struct PcAxVoice
{
    AXVoiceOffsets offsets;
    bool playing;
    double ratio;
    double position;
    float volume;
    float mix[eTvChannel_Count];
};

OSFastMutex s_voiceSection;
bool s_initialised = false;
unsigned int s_voiceCount = 0;
std::vector<float> s_mixBuffer;
std::string s_capturePath;
FILE* s_capture = 0;
unsigned int s_capturedBytes = 0;

std::map<AXVoice*, PcAxVoice*>& Voices()
{
    static std::map<AXVoice*, PcAxVoice*>* s_voices = 0;
    if (s_voices == 0)
        s_voices = new std::map<AXVoice*, PcAxVoice*>;
    return *s_voices;
}

PcAxVoice* FindVoice(AXVoice* voice)
{
    std::map<AXVoice*, PcAxVoice*>::iterator it = Voices().find(voice);
    return it != Voices().end() ? it->second : 0;
}

void WriteLittleEndian(unsigned char* bytes, unsigned int value, int size)
{
    for (int index = 0; index < size; ++index)
        bytes[index] = static_cast<unsigned char>(value >> (8 * index));
}

void WriteCaptureHeader()
{
    unsigned char header[44];
    memcpy(header, "RIFF", 4);
    WriteLittleEndian(header + 4, 36 + s_capturedBytes, 4);
    memcpy(header + 8, "WAVEfmt ", 8);
    WriteLittleEndian(header + 16, 16, 4);
    WriteLittleEndian(header + 20, 1, 2);
    WriteLittleEndian(header + 22, 2, 2);
    WriteLittleEndian(header + 24, kOutputRate, 4);
    WriteLittleEndian(header + 28, kOutputRate * 4, 4);
    WriteLittleEndian(header + 32, 4, 2);
    WriteLittleEndian(header + 34, 16, 2);
    memcpy(header + 36, "data", 4);
    WriteLittleEndian(header + 40, s_capturedBytes, 4);
    fseek(s_capture, 0, SEEK_SET);
    fwrite(header, 1, sizeof(header), s_capture);
    fseek(s_capture, 0, SEEK_END);
}

void Capture(const short* frames, unsigned int count)
{
    if (s_capture == 0)
        return;
    unsigned char bytes[kFramesPerBuffer * 4];
    for (unsigned int index = 0; index < count * 2; ++index)
        WriteLittleEndian(bytes + index * 2, static_cast<unsigned short>(frames[index]), 2);
    fwrite(bytes, 1, count * 4, s_capture);
    s_capturedBytes += count * 4;
    WriteCaptureHeader();
    fflush(s_capture);
}

bool MixVoice(PcAxVoice* voice, float* output, unsigned int count)
{
    const short* data = static_cast<const short*>(voice->offsets.data);
    double end = static_cast<double>(voice->offsets.endOffset) + 1.0;
    double loopStart = static_cast<double>(voice->offsets.loopOffset);
    bool looping = voice->offsets.loopingEnabled != 0 && end > loopStart;
    double step = voice->ratio * kRendererRate / static_cast<double>(kOutputRate);
    float left = voice->volume
                 * (voice->mix[eTvChannel_FrontLeft]
                    + kDownmix * (voice->mix[eTvChannel_Centre] + voice->mix[eTvChannel_RearLeft]));
    float right = voice->volume
                  * (voice->mix[eTvChannel_FrontRight]
                     + kDownmix * (voice->mix[eTvChannel_Centre] + voice->mix[eTvChannel_RearRight]));
    double position = voice->position;
    if (position >= end)
    {
        if (!looping)
            return false;
        position = loopStart;
    }
    for (unsigned int frame = 0; frame < count; ++frame)
    {
        unsigned int index = static_cast<unsigned int>(position);
        double next = static_cast<double>(index) + 1.0;
        if (next >= end)
            next = looping ? loopStart : static_cast<double>(index);
        float fraction = static_cast<float>(position - static_cast<double>(index));
        float first = static_cast<float>(data[index]);
        float sample = first + (static_cast<float>(data[static_cast<unsigned int>(next)]) - first) * fraction;
        output[frame * 2] += sample * left;
        output[frame * 2 + 1] += sample * right;
        position += step;
        if (position >= end)
        {
            if (!looping)
            {
                voice->position = static_cast<double>(voice->offsets.endOffset);
                return false;
            }
            position = loopStart + fmod(position - end, end - loopStart);
        }
    }
    voice->position = position;
    return true;
}

void Mix(short* frames, unsigned int count)
{
    EnterCriticalSection(&s_voiceSection);
    s_mixBuffer.assign(count * 2, 0.0f);
    for (std::map<AXVoice*, PcAxVoice*>::iterator it = Voices().begin(); it != Voices().end(); ++it)
    {
        PcAxVoice* voice = it->second;
        if (!voice->playing || voice->offsets.data == 0 || voice->offsets.dataType != AX_VOICE_FORMAT_LPCM16)
            continue;
        if (!MixVoice(voice, &s_mixBuffer[0], count))
        {
            voice->playing = false;
            it->first->state = AX_VOICE_STATE_STOPPED;
        }
    }
    PcVoiceMix(&s_mixBuffer[0], count);
    for (unsigned int index = 0; index < count * 2; ++index)
    {
        float sample = s_mixBuffer[index];
        frames[index] = static_cast<short>(sample > 32767.0f ? 32767.0f : (sample < -32768.0f ? -32768.0f : sample));
    }
    Capture(frames, count);
    LeaveCriticalSection(&s_voiceSection);
}

void* SilentOutputThread(void*)
{
    PcOsSetCurrentThreadName("Audio clock");
    std::vector<short> frames(kFramesPerBuffer * 2);
    unsigned long long start = PcOsMonotonicNanoseconds();
    unsigned long long produced = 0;
    for (;;)
    {
        PcOsSleepMilliseconds(5);
        unsigned long long due = (PcOsMonotonicNanoseconds() - start) * kOutputRate / 1000000000ULL;
        while (produced + kFramesPerBuffer <= due)
        {
            Mix(&frames[0], kFramesPerBuffer);
            produced += kFramesPerBuffer;
        }
    }
    return 0;
}

void StartOutput()
{
    if (!s_capturePath.empty())
    {
        s_capture = fopen(s_capturePath.c_str(), "wb");
        if (s_capture != 0)
            WriteCaptureHeader();
    }
    if (PcGetDisplayMode() == ePcDisplay_Window && PcOsAudioOpen(kOutputRate, kFramesPerBuffer, Mix))
    {
        printf("[pc] audio: %u Hz stereo output\n", kOutputRate);
    }
    else
    {
        printf("[pc] audio: no output device; sound runs silently\n");
        pthread_t thread;
        pthread_create(&thread, 0, SilentOutputThread, 0);
        pthread_detach(thread);
    }
    fflush(stdout);
}
}

void PcSetAudioCapture(const char* path)
{
    s_capturePath = path;
}

void AXInit()
{
    EnterCriticalSection(&s_voiceSection);
    bool start = !s_initialised;
    s_initialised = true;
    LeaveCriticalSection(&s_voiceSection);
    if (start)
        StartOutput();
}

extern "C" void MIXInit()
{
}

BOOL AXIsInit()
{
    return s_initialised ? TRUE : FALSE;
}

AXVoice* AXAcquireVoice(uint32_t, AXVoiceCallbackFn, void*)
{
    EnterCriticalSection(&s_voiceSection);
    if (Voices().size() >= kMaxVoices)
    {
        LeaveCriticalSection(&s_voiceSection);
        return 0;
    }
    AXVoice* voice = new AXVoice;
    memset(voice, 0, sizeof(AXVoice));
    voice->index = s_voiceCount++;
    PcAxVoice* state = new PcAxVoice;
    memset(state, 0, sizeof(*state));
    state->ratio = 1.0;
    state->volume = 1.0f;
    Voices()[voice] = state;
    LeaveCriticalSection(&s_voiceSection);
    return voice;
}

void AXFreeVoice(AXVoice* voice)
{
    EnterCriticalSection(&s_voiceSection);
    std::map<AXVoice*, PcAxVoice*>::iterator it = Voices().find(voice);
    if (it != Voices().end())
    {
        delete it->second;
        Voices().erase(it);
        delete voice;
    }
    LeaveCriticalSection(&s_voiceSection);
}

void AXGetVoiceOffsets(AXVoice* voice, AXVoiceOffsets* offsets)
{
    EnterCriticalSection(&s_voiceSection);
    PcAxVoice* state = FindVoice(voice);
    if (state != 0)
    {
        *offsets = state->offsets;
        offsets->currentOffset = static_cast<uint32_t>(state->position);
    }
    else
    {
        memset(offsets, 0, sizeof(*offsets));
    }
    LeaveCriticalSection(&s_voiceSection);
}

void AXSetVoiceOffsets(AXVoice* voice, AXVoiceOffsets* offsets)
{
    EnterCriticalSection(&s_voiceSection);
    PcAxVoice* state = FindVoice(voice);
    if (state != 0)
    {
        state->offsets = *offsets;
        state->position = static_cast<double>(offsets->currentOffset);
    }
    LeaveCriticalSection(&s_voiceSection);
}

AXResult AXSetVoiceDeviceMix(AXVoice* voice, AXDeviceType device, uint32_t id, AXVoiceDeviceMixData* mix)
{
    if (device != AX_DEVICE_TYPE_TV || id != 0)
        return AX_RESULT_SUCCESS;
    EnterCriticalSection(&s_voiceSection);
    PcAxVoice* state = FindVoice(voice);
    if (state != 0)
    {
        for (int channel = 0; channel < eTvChannel_Count; ++channel)
            state->mix[channel] = static_cast<float>(mix[channel].bus[0].volume) / kUnityVolume;
    }
    LeaveCriticalSection(&s_voiceSection);
    return AX_RESULT_SUCCESS;
}

AXVoiceSrcRatioResult AXSetVoiceSrcRatio(AXVoice* voice, float ratio)
{
    EnterCriticalSection(&s_voiceSection);
    PcAxVoice* state = FindVoice(voice);
    if (state != 0)
        state->ratio = ratio;
    LeaveCriticalSection(&s_voiceSection);
    return AX_VOICE_RATIO_RESULT_SUCCESS;
}

void AXSetVoiceSrcType(AXVoice*, AXVoiceSrcType)
{
}

void AXSetVoiceState(AXVoice* voice, AXVoiceState newState)
{
    EnterCriticalSection(&s_voiceSection);
    PcAxVoice* state = FindVoice(voice);
    if (state != 0)
    {
        state->playing = newState == AX_VOICE_STATE_PLAYING;
        voice->state = newState;
    }
    LeaveCriticalSection(&s_voiceSection);
}

void AXSetVoiceVe(AXVoice* voice, AXVoiceVeData* ve)
{
    EnterCriticalSection(&s_voiceSection);
    PcAxVoice* state = FindVoice(voice);
    if (state != 0)
        state->volume = static_cast<float>(ve->volume) / kUnityVolume;
    LeaveCriticalSection(&s_voiceSection);
}
