#include "platform/pc/PcVoice.hpp"

#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcSettings.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "platform/wiiu/CProfile.hpp"

#include <math.h>
#include <opus.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace
{
const unsigned int LOOPBACK_MAX = kPcVoiceRate / 4;
const unsigned int LOOPBACK_START = kPcVoiceRate / 50;
const unsigned int FILE_CHUNK = kPcVoiceRate / 100;
const float LEVEL_DECAY = 0.97f;
const float SAMPLE_SCALE = 32767.0f;
const unsigned int FRAME_SAMPLES = kPcVoiceRate / 50;
const int MAX_PACKET_BYTES = 400;
const int BITRATE = 24000;
const float GATE_THRESHOLD = 0.01f;
const int GATE_HOLD_FRAMES = 20;
const size_t MAX_OUTGOING = 25;
const size_t SPEAKER_START = FRAME_SAMPLES * 3;
const size_t SPEAKER_MAX = kPcVoiceRate * 2 / 5;
const unsigned long long TALKING_NS = 250000000ULL;
const unsigned long long SPEAKER_IDLE_NS = 10000000000ULL;

const float FULL_VOLUME_DIST = 3.0f;
const float PAN_DEPTH = 0.6f;
const float GAIN_SMOOTHING = 0.002f;
const float PI_F = 3.14159265f;

struct Speaker
{
    Speaker() : decoder(0), playing(false), lastPacket(0), left(1.0f), right(1.0f), targetLeft(1.0f), targetRight(1.0f)
    {
    }

    OpusDecoder* decoder;
    std::deque<float> pcm;
    bool playing;
    unsigned long long lastPacket;
    float left;
    float right;
    float targetLeft;
    float targetRight;
};

pthread_mutex_t voiceMutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<PcOsMicDevice> micDevices;
bool devicesListed = false;
bool testing = false;
bool inputOpen = false;
bool fileOpen = false;
bool testAtStart = false;
std::string filePath;
std::vector<float> fileSamples;
pthread_t fileThread;
volatile bool fileStop = false;
std::deque<float> loopback;
bool loopbackPlaying = false;
volatile float micLevel = 0.0f;
volatile bool transmitting = false;
volatile bool listening = false;
OpusEncoder* encoder = 0;
std::vector<float> pendingFrame;
int gateHold = 0;
std::deque<std::vector<unsigned char> > outgoing;
std::map<int, Speaker> speakers;
unsigned int sentFrames = 0;
unsigned int receivedFrames = 0;
bool forceChat = false;
std::set<int> mutedSpeakers;
volatile bool pushToTalkHeld = false;
volatile unsigned long long lastSentTime = 0;
std::vector<PcVoicePosition> playerPositions;
volatile int hostOverrideMode = -1;
volatile int hostOverrideRange = 0;
pthread_mutex_t positionMutex = PTHREAD_MUTEX_INITIALIZER;

bool FindPosition(int smallId, PcVoicePosition* out)
{
    for (size_t i = 0; i < playerPositions.size(); ++i)
    {
        if (playerPositions[i].smallId == smallId)
        {
            *out = playerPositions[i];
            return true;
        }
    }
    return false;
}
unsigned long long nextReportTime = 0;
const unsigned long long REPORT_NS = 10000000000ULL;

void Encode(const std::vector<float>& samples)
{
    pendingFrame.insert(pendingFrame.end(), samples.begin(), samples.end());
    while (pendingFrame.size() >= FRAME_SAMPLES)
    {
        float energy = 0.0f;
        for (unsigned int i = 0; i < FRAME_SAMPLES; ++i)
            energy += pendingFrame[i] * pendingFrame[i];
        if (sqrtf(energy / (float)FRAME_SAMPLES) >= GATE_THRESHOLD)
            gateHold = GATE_HOLD_FRAMES;
        else if (gateHold > 0)
            --gateHold;
        const bool open = PcSettingsVoiceMode() == kPcVoiceModePushToTalk ? pushToTalkHeld : gateHold > 0;
        if (open && encoder != 0)
        {
            unsigned char pkt[MAX_PACKET_BYTES];
            const int n = opus_encode_float(encoder, &pendingFrame[0], (int)FRAME_SAMPLES, pkt,
                                                MAX_PACKET_BYTES);
            if (n > 0)
            {
                pthread_mutex_lock(&voiceMutex);
                outgoing.push_back(std::vector<unsigned char>(pkt, pkt + n));
                while (outgoing.size() > MAX_OUTGOING)
                    outgoing.pop_front();
                ++sentFrames;
                lastSentTime = PcOsMonotonicNanoseconds();
                pthread_mutex_unlock(&voiceMutex);
            }
        }
        pendingFrame.erase(pendingFrame.begin(), pendingFrame.begin() + FRAME_SAMPLES);
    }
}

void Deliver(const float* samples, unsigned int n)
{
    const float gain = (float)PcSettingsMicVolume() / 100.0f;
    float peak = 0.0f;
    std::vector<float> buf(n);
    for (unsigned int i = 0; i < n; ++i)
    {
        float s = samples[i] * gain;
        s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s);
        const float mag = fabsf(s);
        if (mag > peak)
            peak = mag;
        buf[i] = s;
    }
    if (testing)
    {
        pthread_mutex_lock(&voiceMutex);
        loopback.insert(loopback.end(), buf.begin(), buf.end());
        while (loopback.size() > LOOPBACK_MAX)
            loopback.pop_front();
        pthread_mutex_unlock(&voiceMutex);
    }
    const float decayed = micLevel * LEVEL_DECAY;
    micLevel = peak > decayed ? peak : decayed;
    if (transmitting)
        Encode(buf);
}

unsigned int ReadLittleEndian(const unsigned char* p, int n)
{
    unsigned int v = 0;
    for (int i = n - 1; i >= 0; --i)
        v = (v << 8) | p[i];
    return v;
}

bool LoadWave(const std::string& path, std::vector<float>* out)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return false;
    std::vector<unsigned char> data;
    unsigned char buf[65536];
    size_t bytes = 0;
    while ((bytes = fread(buf, 1, sizeof(buf), fp)) > 0)
        data.insert(data.end(), buf, buf + bytes);
    fclose(fp);
    if (data.size() < 12 || memcmp(&data[0], "RIFF", 4) != 0 || memcmp(&data[8], "WAVE", 4) != 0)
        return false;
    unsigned int channels = 0;
    unsigned int rate = 0;
    unsigned int bits = 0;
    size_t pos = 12;
    while (pos + 8 <= data.size())
    {
        const unsigned int len = ReadLittleEndian(&data[pos + 4], 4);
        const size_t body = pos + 8;
        if (body + len > data.size())
            break;
        if (memcmp(&data[pos], "fmt ", 4) == 0 && len >= 16)
        {
            channels = ReadLittleEndian(&data[body + 2], 2);
            rate = ReadLittleEndian(&data[body + 4], 4);
            bits = ReadLittleEndian(&data[body + 14], 2);
        }
        else if (memcmp(&data[pos], "data", 4) == 0 && channels > 0 && bits == 16)
        {
            const unsigned int frames = len / (2 * channels);
            std::vector<float> mono(frames);
            for (unsigned int i = 0; i < frames; ++i)
            {
                float sum = 0.0f;
                for (unsigned int ch = 0; ch < channels; ++ch)
                    sum += (float)((short)
                        ReadLittleEndian(&data[body + (i * channels + ch) * 2], 2)) / 32768.0f;
                mono[i] = sum / (float)channels;
            }
            out->clear();
            const double step = (double)rate / (double)kPcVoiceRate;
            for (double at = 0.0; at + 1.0 < (double)frames; at += step)
            {
                const size_t i = (size_t)at;
                const float t = (float)(at - (double)i);
                out->push_back(mono[i] + (mono[i + 1] - mono[i]) * t);
            }
            return !out->empty();
        }
        pos = body + len + (len & 1);
    }
    return false;
}

void* FileThread(void*)
{
    PcOsSetCurrentThreadName("Microphone file");
    const unsigned long long t0 = PcOsMonotonicNanoseconds();
    unsigned long long done = 0;
    size_t pos = 0;
    std::vector<float> buf(FILE_CHUNK);
    while (!fileStop)
    {
        const unsigned long long due = (PcOsMonotonicNanoseconds() - t0) * kPcVoiceRate / 1000000000ULL;
        while (done + FILE_CHUNK <= due)
        {
            for (unsigned int i = 0; i < FILE_CHUNK; ++i)
            {
                buf[i] = fileSamples[pos];
                pos = (pos + 1) % fileSamples.size();
            }
            Deliver(&buf[0], FILE_CHUNK);
            done += FILE_CHUNK;
        }
        PcOsSleepMilliseconds(5);
    }
    return 0;
}

std::string SelectedId()
{
    return PcSettingsVoiceInput();
}

void OpenInput()
{
    if (inputOpen)
        return;
    if (!filePath.empty())
    {
        if (fileSamples.empty() && !LoadWave(filePath, &fileSamples))
        {
            printf("[pc] voice: %s is not a 16-bit PCM WAV file\n", filePath.c_str());
            fflush(stdout);
            return;
        }
        pendingFrame.clear();
        gateHold = 0;
        fileStop = false;
        fileOpen = pthread_create(&fileThread, 0, FileThread, 0) == 0;
        inputOpen = fileOpen;
        printf("[pc] voice: microphone input from %s\n", filePath.c_str());
        fflush(stdout);
        return;
    }
    std::string micName;
    pendingFrame.clear();
    gateHold = 0;
    inputOpen = PcOsMicOpen(SelectedId(), kPcVoiceRate, Deliver, &micName);
    if (!inputOpen && !SelectedId().empty())
    {
        printf("[pc] voice: the selected microphone is missing; using the default one\n");
        fflush(stdout);
        inputOpen = PcOsMicOpen(std::string(), kPcVoiceRate, Deliver, &micName);
    }
}

void CloseInput()
{
    if (!inputOpen)
        return;
    if (fileOpen)
    {
        fileStop = true;
        pthread_join(fileThread, 0);
        fileOpen = false;
    }
    else
    {
        PcOsMicClose();
    }
    inputOpen = false;
    micLevel = 0.0f;
    printf("[pc] voice: microphone closed\n");
    fflush(stdout);
}

void UpdateInput()
{
    if (testing || transmitting)
        OpenInput();
    else
        CloseInput();
}

void ReleaseSpeaker(Speaker& sp)
{
    if (sp.decoder != 0)
        opus_decoder_destroy(sp.decoder);
    sp.decoder = 0;
}
}

void PcVoiceSetFileInput(const char* path)
{
    filePath = path;
}

void PcVoiceForceChat()
{
    forceChat = true;
}

void PcVoiceRequestTestAtStart()
{
    testAtStart = true;
}

void PcVoiceTick()
{
    if (testAtStart)
    {
        testAtStart = false;
        PcVoiceSetTest(true);
    }
    pushToTalkHeld = PcInputPushToTalkHeld();
    if (forceChat && gApp.GetGameStarted() && !gApp.IsVoiceChatEnabled())
        GameSettings::SetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_GameChat, 1);
    const bool chatOn = gApp.IsVoiceChatEnabled() && !PcIsDedicatedServer();
    listening = chatOn;
    const bool want = chatOn && PcNetworkSession::VoicePeerCount() > 0;
    if (want != transmitting)
    {
        if (want && encoder == 0)
        {
            int err = 0;
            encoder = opus_encoder_create((opus_int32)kPcVoiceRate, 1, OPUS_APPLICATION_VOIP, &err);
            if (encoder != 0)
                opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
        }
        transmitting = want && encoder != 0;
        printf("[pc] voice: %s\n", transmitting ? "sending the microphone to the game" : "not sending the microphone");
        fflush(stdout);
        if (!transmitting)
        {
            pthread_mutex_lock(&voiceMutex);
            outgoing.clear();
            pthread_mutex_unlock(&voiceMutex);
        }
        UpdateInput();
    }
    const unsigned long long now = PcOsMonotonicNanoseconds();
    pthread_mutex_lock(&voiceMutex);
    if ((transmitting || !speakers.empty()) && now >= nextReportTime)
    {
        if (nextReportTime != 0)
            printf("[pc] voice: %u frame(s) sent, %u received, %u speaker(s)\n", sentFrames, receivedFrames,
                   (unsigned int)speakers.size());
        fflush(stdout);
        nextReportTime = now + REPORT_NS;
    }
    for (std::map<int, Speaker>::iterator it = speakers.begin(); it != speakers.end();)
    {
        if (now - it->second.lastPacket > SPEAKER_IDLE_NS || !chatOn)
        {
            ReleaseSpeaker(it->second);
            speakers.erase(it++);
        }
        else
        {
            ++it;
        }
    }
    pthread_mutex_unlock(&voiceMutex);
}

bool PcVoiceTakeOutgoing(std::vector<unsigned char>* out)
{
    pthread_mutex_lock(&voiceMutex);
    const bool ok = !outgoing.empty();
    if (ok)
    {
        out->swap(outgoing.front());
        outgoing.pop_front();
    }
    pthread_mutex_unlock(&voiceMutex);
    return ok;
}

void PcVoiceSetPositions(const std::vector<PcVoicePosition>& list)
{
    pthread_mutex_lock(&positionMutex);
    playerPositions = list;
    pthread_mutex_unlock(&positionMutex);
}

void PcVoiceSetHostOverride(int mode, int range)
{
    hostOverrideMode = mode;
    hostOverrideRange = range;
}

void PcVoiceClearHostOverride()
{
    hostOverrideMode = -1;
    hostOverrideRange = 0;
}

bool PcVoiceHostOverride(int* mode, int* range)
{
    const int m = hostOverrideMode;
    if (m < 0)
        return false;
    *mode = m;
    *range = hostOverrideRange;
    return true;
}

bool PcVoiceEffectiveProximity(int* range)
{
    int mode = kPcHostProximityUsersChoose;
    int hostRange = 0;
    PcVoiceHostOverride(&mode, &hostRange);
    *range = hostRange != 0 ? hostRange : PcSettingsProximityRange();
    if (mode == kPcHostProximityForce)
        return true;
    if (mode == kPcHostProximityDisable)
        return false;
    return PcSettingsProximityChat();
}

bool PcVoiceRoute(int from, int to, bool proximity, int maxBlocks, unsigned char* gain,
                  unsigned char* pan)
{
    *gain = kPcVoiceFullGain;
    *pan = kPcVoiceCentrePan;
    if (!proximity)
        return true;
    PcVoicePosition src;
    PcVoicePosition dst;
    pthread_mutex_lock(&positionMutex);
    const bool found = FindPosition(from, &src) && FindPosition(to, &dst);
    pthread_mutex_unlock(&positionMutex);
    if (!found || src.dimension != dst.dimension)
        return false;
    const float range = (float)maxBlocks;
    const float dx = (float)(src.x - dst.x);
    const float dy = (float)(src.y - dst.y);
    const float dz = (float)(src.z - dst.z);
    const float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist >= range)
        return false;
    float vol = 1.0f;
    if (dist > FULL_VOLUME_DIST)
        vol = 1.0f - (dist - FULL_VOLUME_DIST) / (range - FULL_VOLUME_DIST);
    *gain = (unsigned char)(vol * (float)kPcVoiceFullGain + 0.5f);
    const float flatDist = sqrtf(dx * dx + dz * dz);
    if (flatDist > 0.01f)
    {
        const float yaw = dst.yaw * PI_F / 180.0f;
        const float side = (dx * -cosf(yaw) + dz * -sinf(yaw)) / flatDist;
        *pan = (unsigned char)((side + 1.0f) * (float)kPcVoiceCentrePan + 0.5f);
    }
    return true;
}

void PcVoiceReceive(int id, unsigned char gain, unsigned char pan, const unsigned char* data,
                    unsigned int len)
{
    if (!listening || len == 0)
        return;
    pthread_mutex_lock(&voiceMutex);
    if (mutedSpeakers.count(id) != 0)
    {
        pthread_mutex_unlock(&voiceMutex);
        return;
    }
    Speaker& sp = speakers[id];
    if (sp.decoder == 0)
    {
        int err = 0;
        sp.decoder = opus_decoder_create((opus_int32)kPcVoiceRate, 1, &err);
        sp.playing = false;
    }
    if (sp.decoder != 0)
    {
        float buf[kPcVoiceRate / 10];
        const int n = opus_decode_float(sp.decoder, data, (opus_int32)len, buf,
                                             (int)(kPcVoiceRate / 10), 0);
        if (n > 0)
        {
            const float vol = (float)gain / (float)kPcVoiceFullGain;
            const float side = ((float)pan - (float)kPcVoiceCentrePan)
                               / (float)kPcVoiceCentrePan;
            sp.targetLeft = vol * (1.0f - PAN_DEPTH * (side > 0.0f ? side : 0.0f));
            sp.targetRight = vol * (1.0f - PAN_DEPTH * (side < 0.0f ? -side : 0.0f));
            if (sp.pcm.empty() && !sp.playing)
            {
                sp.left = sp.targetLeft;
                sp.right = sp.targetRight;
            }
            sp.pcm.insert(sp.pcm.end(), buf, buf + n);
            while (sp.pcm.size() > SPEAKER_MAX)
                sp.pcm.pop_front();
            sp.lastPacket = PcOsMonotonicNanoseconds();
            ++receivedFrames;
        }
    }
    pthread_mutex_unlock(&voiceMutex);
}

bool PcVoiceSpeakerTalking(int id)
{
    pthread_mutex_lock(&voiceMutex);
    std::map<int, Speaker>::iterator it = speakers.find(id);
    const bool talking = it != speakers.end()
                         && PcOsMonotonicNanoseconds() - it->second.lastPacket < TALKING_NS;
    pthread_mutex_unlock(&voiceMutex);
    return talking;
}

bool PcVoiceSending()
{
    return transmitting;
}

bool PcVoiceLocalTalking()
{
    return transmitting && PcOsMonotonicNanoseconds() - lastSentTime < TALKING_NS;
}

void PcVoiceSetMuted(int id, bool muted)
{
    pthread_mutex_lock(&voiceMutex);
    if (muted)
    {
        mutedSpeakers.insert(id);
        std::map<int, Speaker>::iterator it = speakers.find(id);
        if (it != speakers.end())
        {
            ReleaseSpeaker(it->second);
            speakers.erase(it);
        }
    }
    else
    {
        mutedSpeakers.erase(id);
    }
    pthread_mutex_unlock(&voiceMutex);
}

bool PcVoiceMuted(int id)
{
    pthread_mutex_lock(&voiceMutex);
    const bool muted = mutedSpeakers.count(id) != 0;
    pthread_mutex_unlock(&voiceMutex);
    return muted;
}

void PcVoiceForgetSpeaker(int id)
{
    pthread_mutex_lock(&voiceMutex);
    mutedSpeakers.erase(id);
    std::map<int, Speaker>::iterator it = speakers.find(id);
    if (it != speakers.end())
    {
        ReleaseSpeaker(it->second);
        speakers.erase(it);
    }
    pthread_mutex_unlock(&voiceMutex);
}

void PcVoiceStatistics(unsigned int* sent, unsigned int* received)
{
    pthread_mutex_lock(&voiceMutex);
    *sent = sentFrames;
    *received = receivedFrames;
    pthread_mutex_unlock(&voiceMutex);
}

void PcVoiceRefreshDevices()
{
    PcOsMicListDevices(&micDevices);
    devicesListed = true;
    printf("[pc] voice: %u microphone(s)\n", (unsigned int)micDevices.size());
    for (size_t i = 0; i < micDevices.size(); ++i)
        printf("[pc] voice:   %s\n", micDevices[i].name.c_str());
    fflush(stdout);
}

int PcVoiceDeviceCount()
{
    if (!devicesListed)
        PcVoiceRefreshDevices();
    return (int)micDevices.size() + 1;
}

std::wstring PcVoiceDeviceName(int idx)
{
    if (idx <= 0 || idx > (int)micDevices.size())
        return micDevices.empty() ? std::wstring(L"Default (none found)") : std::wstring(L"Default");
    const std::string& name = micDevices[(size_t)(idx - 1)].name;
    std::wstring out;
    for (size_t pos = 0; pos < name.size();)
    {
        unsigned int cp = (unsigned char)name[pos];
        int extra = cp >= 0xF0 ? 3 : cp >= 0xE0 ? 2 : cp >= 0xC0 ? 1 : 0;
        cp &= extra == 3 ? 0x07 : extra == 2 ? 0x0F : extra == 1 ? 0x1F : 0x7F;
        ++pos;
        for (int i = 0; i < extra && pos < name.size(); ++i, ++pos)
            cp = (cp << 6) | ((unsigned char)name[pos] & 0x3F);
        out += cp > 0xFFFF ? L'?' : (wchar_t)cp;
    }
    return out;
}

int PcVoiceSelectedDevice()
{
    PcVoiceDeviceCount();
    const std::string id = SelectedId();
    for (size_t i = 0; i < micDevices.size(); ++i)
    {
        if (micDevices[i].id == id)
            return (int)i + 1;
    }
    return 0;
}

void PcVoiceSelectDevice(int idx)
{
    PcVoiceDeviceCount();
    std::string id;
    if (idx > 0 && idx <= (int)micDevices.size())
        id = micDevices[(size_t)(idx - 1)].id;
    if (id == SelectedId())
        return;
    PcSettingsSetVoiceInput(id);
    PcSettingsSave();
    if (inputOpen && !fileOpen)
    {
        CloseInput();
        UpdateInput();
    }
}

void PcVoiceSetTest(bool on)
{
    if (on == testing)
        return;
    pthread_mutex_lock(&voiceMutex);
    testing = on;
    loopback.clear();
    loopbackPlaying = false;
    pthread_mutex_unlock(&voiceMutex);
    UpdateInput();
}

bool PcVoiceTesting()
{
    return testing;
}

bool PcVoiceInputOpen()
{
    return inputOpen;
}

int PcVoiceInputLevel()
{
    const float lv = micLevel;
    return (int)(lv * 100.0f + 0.5f);
}

void PcVoiceMix(float* out, unsigned int n)
{
    pthread_mutex_lock(&voiceMutex);
    if (!loopbackPlaying && loopback.size() >= LOOPBACK_START)
        loopbackPlaying = true;
    if (loopbackPlaying)
    {
        for (unsigned int i = 0; i < n && !loopback.empty(); ++i)
        {
            const float s = loopback.front() * SAMPLE_SCALE;
            loopback.pop_front();
            out[i * 2] += s;
            out[i * 2 + 1] += s;
        }
        if (loopback.empty())
            loopbackPlaying = false;
    }
    float volume = PcSettingsVoiceVolume() / 100.0f;
    for (std::map<int, Speaker>::iterator it = speakers.begin(); it != speakers.end(); ++it)
    {
        Speaker& sp = it->second;
        if (!sp.playing && sp.pcm.size() >= SPEAKER_START)
            sp.playing = true;
        if (!sp.playing)
            continue;
        for (unsigned int i = 0; i < n && !sp.pcm.empty(); ++i)
        {
            const float s = sp.pcm.front() * SAMPLE_SCALE * volume;
            sp.pcm.pop_front();
            sp.left += (sp.targetLeft - sp.left) * GAIN_SMOOTHING;
            sp.right += (sp.targetRight - sp.right) * GAIN_SMOOTHING;
            out[i * 2] += s * sp.left;
            out[i * 2 + 1] += s * sp.right;
        }
        if (sp.pcm.empty())
            sp.playing = false;
    }
    pthread_mutex_unlock(&voiceMutex);
}
