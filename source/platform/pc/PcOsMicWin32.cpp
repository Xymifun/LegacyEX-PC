#ifdef _WIN32

#include "platform/pc/PcOs.hpp"

#include <windows.h>

#include <initguid.h>

#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <math.h>
#include <mmreg.h>
#include <stdio.h>
#include <string.h>

#include <vector>

namespace
{
const REFERENCE_TIME BUFFER_DURATION = 1000000;
const unsigned int POLL_MS = 10;
const DWORD OPEN_TIMEOUT_MS = 5000;

enum SampleFormat
{
    eSampleFormat_Unsupported,
    eSampleFormat_Float32,
    eSampleFormat_Int16,
    eSampleFormat_Int32
};

struct OpenRequest
{
    std::wstring id;
    unsigned int rate;
    PcOsMicDeliverFn deliver;
    bool opened;
    std::string name;
    HANDLE ready;
};

HANDLE micThread = 0;
volatile LONG stopFlag = 0;

std::string Utf8(const wchar_t* s)
{
    if (s == 0 || s[0] == 0)
        return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, s, -1, 0, 0, 0, 0);
    if (len <= 1)
        return std::string();
    std::string out((size_t)(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s, -1, &out[0], len, 0, 0);
    return out;
}

std::wstring Wide(const std::string& s)
{
    if (s.empty())
        return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, 0, 0);
    if (len <= 1)
        return std::wstring();
    std::wstring out((size_t)(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], len);
    return out;
}

template <typename T> void Release(T*& obj)
{
    if (obj != 0)
    {
        obj->Release();
        obj = 0;
    }
}

std::string FriendlyName(IMMDevice* device)
{
    std::string name;
    IPropertyStore* store = 0;
    if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &store)))
    {
        PROPVARIANT prop;
        PropVariantInit(&prop);
        if (SUCCEEDED(store->GetValue(PKEY_Device_FriendlyName, &prop)) && prop.vt == VT_LPWSTR)
            name = Utf8(prop.pwszVal);
        PropVariantClear(&prop);
        Release(store);
    }
    return name;
}

bool RunningOnWine()
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    return ntdll != 0 && GetProcAddress(ntdll, "wine_get_version") != 0;
}

std::wstring InterfaceName(IPropertyStore* store)
{
    std::wstring name;
    PROPVARIANT prop;
    PropVariantInit(&prop);
    if (SUCCEEDED(store->GetValue(PKEY_DeviceInterface_FriendlyName, &prop)) && prop.vt == VT_LPWSTR)
        name = prop.pwszVal;
    PropVariantClear(&prop);
    return name;
}

bool RecordsSoundOutput(IMMDevice* device)
{
    bool loopback = false;
    IPropertyStore* store = 0;
    if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &store)))
    {
        loopback = InterfaceName(store).compare(0, 11, L"Monitor of ") == 0;
        PROPVARIANT prop;
        PropVariantInit(&prop);
        if (!loopback && RunningOnWine() && SUCCEEDED(store->GetValue(PKEY_AudioEndpoint_FormFactor, &prop))
            && prop.vt == VT_UI4)
            loopback = prop.ulVal == LineLevel;
        PropVariantClear(&prop);
        Release(store);
    }
    return loopback;
}

bool WineDefaultInput(IMMDevice* device)
{
    bool alias = false;
    IPropertyStore* store = 0;
    if (RunningOnWine() && SUCCEEDED(device->OpenPropertyStore(STGM_READ, &store)))
    {
        alias = InterfaceName(store) == L"PulseAudio Input";
        Release(store);
    }
    return alias;
}

SampleFormat FormatOf(const WAVEFORMATEX* fmt)
{
    WORD tag = fmt->wFormatTag;
    if (tag == WAVE_FORMAT_EXTENSIBLE && fmt->cbSize >= 22)
        tag = (WORD)(reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(fmt)->SubFormat.Data1);
    if (tag == WAVE_FORMAT_IEEE_FLOAT && fmt->wBitsPerSample == 32)
        return eSampleFormat_Float32;
    if (tag == WAVE_FORMAT_PCM && fmt->wBitsPerSample == 16)
        return eSampleFormat_Int16;
    if (tag == WAVE_FORMAT_PCM && fmt->wBitsPerSample == 32)
        return eSampleFormat_Int32;
    return eSampleFormat_Unsupported;
}

float SampleAt(const BYTE* data, SampleFormat fmt, unsigned int i)
{
    switch (fmt)
    {
    case eSampleFormat_Float32:
        return reinterpret_cast<const float*>(data)[i];
    case eSampleFormat_Int16:
        return (float)(reinterpret_cast<const short*>(data)[i]) / 32768.0f;
    case eSampleFormat_Int32:
        return (float)(reinterpret_cast<const int*>(data)[i]) / 2147483648.0f;
    default:
        return 0.0f;
    }
}

class Resampler
{
public:
    Resampler(unsigned int from, unsigned int to)
        : m_step((double)from / (double)to), m_position(0.0), m_previous(0.0f)
    {
    }

    void Push(const std::vector<float>& in, std::vector<float>* out)
    {
        out->clear();
        if (in.empty())
            return;
        const long long n = (long long)in.size();
        for (;;)
        {
            const double base = floor(m_position);
            const long long i = (long long)base;
            if (i + 1 >= n)
                break;
            const float a = i < 0 ? m_previous : in[(size_t)i];
            const float b = in[(size_t)(i + 1)];
            out->push_back(a + (b - a) * (float)(m_position - base));
            m_position += m_step;
        }
        m_position -= (double)n;
        m_previous = in.back();
    }

private:
    double m_step;
    double m_position;
    float m_previous;
};

DWORD WINAPI CaptureThread(LPVOID param)
{
    OpenRequest* req = static_cast<OpenRequest*>(param);
    PcOsSetCurrentThreadName("Microphone");
    const PcOsMicDeliverFn deliver = req->deliver;
    const unsigned int rate = req->rate;
    HRESULT comInit = CoInitializeEx(0, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* enumerator = 0;
    IMMDevice* device = 0;
    IAudioClient* client = 0;
    IAudioCaptureClient* capture = 0;
    WAVEFORMATEX* format = 0;
    SampleFormat sampleFmt = eSampleFormat_Unsupported;
    bool ok = SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                         reinterpret_cast<void**>(&enumerator)));
    if (ok)
    {
        if (req->id.empty())
            ok = SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &device));
        else
            ok = SUCCEEDED(enumerator->GetDevice(req->id.c_str(), &device));
    }
    if (ok && RecordsSoundOutput(device))
    {
        printf("[pc] voice: \"%s\" records the computer's sound output, not a microphone; not opened\n",
               FriendlyName(device).c_str());
        fflush(stdout);
        ok = false;
    }
    if (ok)
        ok = SUCCEEDED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, reinterpret_cast<void**>(&client)));
    if (ok)
        ok = SUCCEEDED(client->GetMixFormat(&format));
    if (ok)
    {
        sampleFmt = FormatOf(format);
        ok = sampleFmt != eSampleFormat_Unsupported && format->nChannels > 0;
    }
    if (ok)
        ok = SUCCEEDED(client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, BUFFER_DURATION, 0, format, 0));
    if (ok)
        ok = SUCCEEDED(client->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&capture)));
    if (ok)
        ok = SUCCEEDED(client->Start());
    if (ok)
    {
        req->name = FriendlyName(device);
        printf("[pc] voice: microphone \"%s\" opened (%lu Hz, %u channel(s), %u-bit)\n", req->name.c_str(),
               (unsigned long)format->nSamplesPerSec, (unsigned int)format->nChannels,
               (unsigned int)format->wBitsPerSample);
        fflush(stdout);
    }
    req->opened = ok;
    SetEvent(req->ready);

    if (ok)
    {
        const unsigned int channels = format->nChannels;
        Resampler resampler(format->nSamplesPerSec, rate);
        std::vector<float> mono;
        std::vector<float> resampled;
        while (InterlockedCompareExchange(&stopFlag, 0, 0) == 0)
        {
            UINT32 pkt = 0;
            while (SUCCEEDED(capture->GetNextPacketSize(&pkt)) && pkt > 0)
            {
                BYTE* data = 0;
                UINT32 frames = 0;
                DWORD flags = 0;
                if (FAILED(capture->GetBuffer(&data, &frames, &flags, 0, 0)))
                    break;
                mono.assign(frames, 0.0f);
                if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) == 0)
                {
                    for (UINT32 i = 0; i < frames; ++i)
                    {
                        float sum = 0.0f;
                        for (unsigned int c = 0; c < channels; ++c)
                            sum += SampleAt(data, sampleFmt, i * channels + c);
                        mono[i] = sum / (float)channels;
                    }
                }
                capture->ReleaseBuffer(frames);
                resampler.Push(mono, &resampled);
                if (!resampled.empty())
                    deliver(&resampled[0], (unsigned int)resampled.size());
            }
            PcOsSleepMilliseconds(POLL_MS);
        }
        client->Stop();
    }
    else
    {
        printf("[pc] voice: the microphone could not be opened\n");
        fflush(stdout);
    }
    if (format != 0)
        CoTaskMemFree(format);
    Release(capture);
    Release(client);
    Release(device);
    Release(enumerator);
    if (SUCCEEDED(comInit))
        CoUninitialize();
    return 0;
}

DWORD WINAPI ListThread(LPVOID param)
{
    std::vector<PcOsMicDevice>* devices = static_cast<std::vector<PcOsMicDevice>*>(param);
    HRESULT comInit = CoInitializeEx(0, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* enumerator = 0;
    IMMDeviceCollection* collection = 0;
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                   reinterpret_cast<void**>(&enumerator)))
        && SUCCEEDED(enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &collection)))
    {
        UINT count = 0;
        collection->GetCount(&count);
        for (UINT i = 0; i < count; ++i)
        {
            IMMDevice* device = 0;
            if (FAILED(collection->Item(i, &device)))
                continue;
            if (RecordsSoundOutput(device) || WineDefaultInput(device))
            {
                printf("[pc] voice: not listing \"%s\" (%s)\n", FriendlyName(device).c_str(),
                       WineDefaultInput(device) ? "the same as Default" : "it records the computer's sound output");
                Release(device);
                continue;
            }
            LPWSTR id = 0;
            if (SUCCEEDED(device->GetId(&id)))
            {
                PcOsMicDevice e;
                e.id = Utf8(id);
                e.name = FriendlyName(device);
                if (e.name.empty())
                    e.name = e.id;
                devices->push_back(e);
                CoTaskMemFree(id);
            }
            Release(device);
        }
    }
    Release(collection);
    Release(enumerator);
    if (SUCCEEDED(comInit))
        CoUninitialize();
    return 0;
}
}

void PcOsMicListDevices(std::vector<PcOsMicDevice>* devices)
{
    devices->clear();
    HANDLE t = CreateThread(0, 0, ListThread, devices, 0, 0);
    if (t == 0)
        return;
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}

bool PcOsMicOpen(const std::string& id, unsigned int rate, PcOsMicDeliverFn deliver, std::string* outName)
{
    PcOsMicClose();
    OpenRequest* req = new OpenRequest();
    req->id = Wide(id);
    req->rate = rate;
    req->deliver = deliver;
    req->opened = false;
    req->ready = CreateEventW(0, TRUE, FALSE, 0);
    InterlockedExchange(&stopFlag, 0);
    micThread = CreateThread(0, 0, CaptureThread, req, 0, 0);
    if (micThread == 0)
    {
        CloseHandle(req->ready);
        delete req;
        return false;
    }
    bool ok = WaitForSingleObject(req->ready, OPEN_TIMEOUT_MS) == WAIT_OBJECT_0 && req->opened;
    if (ok && outName != 0)
        *outName = req->name;
    if (!ok)
        PcOsMicClose();
    CloseHandle(req->ready);
    delete req;
    return ok;
}

void PcOsMicClose()
{
    if (micThread == 0)
        return;
    InterlockedExchange(&stopFlag, 1);
    WaitForSingleObject(micThread, INFINITE);
    CloseHandle(micThread);
    micThread = 0;
}

#endif
