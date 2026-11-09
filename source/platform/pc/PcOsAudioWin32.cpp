#ifdef _WIN32

#include "platform/pc/PcOs.hpp"

#include <windows.h>

#include <mmsystem.h>
#include <string.h>

namespace
{
const unsigned int kBufferCount = 4;

HWAVEOUT s_device;
HANDLE s_bufferDone;
WAVEHDR s_headers[kBufferCount];
short* s_samples[kBufferCount];
unsigned int s_framesPerBuffer;
PcOsAudioFillFn s_fill;

void Submit(unsigned int index)
{
    s_fill(s_samples[index], s_framesPerBuffer);
    waveOutWrite(s_device, &s_headers[index], sizeof(WAVEHDR));
}

DWORD WINAPI AudioThread(LPVOID)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    PcOsSetCurrentThreadName("Audio output");
    for (;;)
    {
        WaitForSingleObject(s_bufferDone, INFINITE);
        for (unsigned int index = 0; index < kBufferCount; ++index)
        {
            if ((s_headers[index].dwFlags & WHDR_DONE) != 0)
                Submit(index);
        }
    }
    return 0;
}
}

bool PcOsAudioOpen(unsigned int rate, unsigned int framesPerBuffer, PcOsAudioFillFn fill)
{
    WAVEFORMATEX format;
    memset(&format, 0, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = rate;
    format.wBitsPerSample = 16;
    format.nBlockAlign = 4;
    format.nAvgBytesPerSec = rate * 4;
    s_bufferDone = CreateEventW(0, FALSE, FALSE, 0);
    if (s_bufferDone == 0)
        return false;
    if (waveOutOpen(&s_device, WAVE_MAPPER, &format, reinterpret_cast<DWORD_PTR>(s_bufferDone), 0, CALLBACK_EVENT)
        != MMSYSERR_NOERROR)
    {
        CloseHandle(s_bufferDone);
        return false;
    }
    s_fill = fill;
    s_framesPerBuffer = framesPerBuffer;
    for (unsigned int index = 0; index < kBufferCount; ++index)
    {
        s_samples[index] = new short[framesPerBuffer * 2];
        memset(&s_headers[index], 0, sizeof(WAVEHDR));
        s_headers[index].lpData = reinterpret_cast<LPSTR>(s_samples[index]);
        s_headers[index].dwBufferLength = framesPerBuffer * 4;
        waveOutPrepareHeader(s_device, &s_headers[index], sizeof(WAVEHDR));
        Submit(index);
    }
    HANDLE thread = CreateThread(0, 0, AudioThread, 0, 0, 0);
    if (thread != 0)
        CloseHandle(thread);
    return thread != 0;
}

#endif
