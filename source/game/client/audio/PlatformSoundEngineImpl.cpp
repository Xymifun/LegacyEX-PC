#include "game/client/audio/PlatformSoundEngineImpl.hpp"

#include <stdio.h>
#include <string.h>

#include "platform/wiiu/C4JThreadImpl.hpp"

#include "vendor/MilesRib.hpp"

#ifdef MINECRAFT_PC
_DIG_DRIVER* g_pcMilesDriver = NULL;
static const int kPcSamplePlaying = 4;
static const unsigned int kPcSampleLimit = 16;
#endif

extern "C" void AXInit();
extern "C" void MIXInit();

extern "C" int BinkADec_RIB_Main(unsigned int uiProvider, int iUpDown,
                                 RIB_ALLOC_PROVIDER_HANDLE_CALLBACK pAllocProviderHandle,
                                 RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                                 RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface);

#include "platform/wiiu/WiiUMilesFileSystem.hpp"
#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesEventExec.hpp"

extern "C" void AIL_startup();
extern "C" void AIL_shutdown();
extern "C" const char* AIL_last_error();
extern "C" int AIL_set_sample_is_3D(_SAMPLE* pSample, int iIs3D);
extern "C" unsigned int AIL_enqueue_event_start();
extern "C" unsigned int AIL_enqueue_event_buffer(unsigned int* puiQueue, void* pData, int iDataSize, int iFlags);
extern "C" void AIL_enqueue_event_end_named(unsigned int uiQueue, const char* pEventName);

extern "C" void MilesSetEventErrorCallback(void (*pCallback)(long long, const char*));

static char szSoundPath[] = "WiiU/Sound/";

#include <coreinit/memdefaultheap.h>

#if defined(MINECRAFT_PC) && __SIZEOF_POINTER__ == 8
extern "C" void* PcLowHeapAlloc(size_t size, size_t alignment);
#endif

extern "C" void* platform_alloc(unsigned int uiSize)
{
#if defined(MINECRAFT_PC) && __SIZEOF_POINTER__ == 8
    return PcLowHeapAlloc(uiSize, 0x40);
#else
    return MEMAllocFromDefaultHeapEx(uiSize, 0x40);
#endif
}

extern "C" void platform_free(void* pBlock)
{
    MEMFreeToDefaultHeap(pBlock);
}

PlatformSoundEngineImpl::PlatformSoundEngineImpl()
{
    m_pStream = NULL;
    m_pStreamDRC = NULL;
    m_pDigitalDriver = NULL;
}

PlatformSoundEngineImpl::~PlatformSoundEngineImpl()
{
}

int PlatformSoundEngineImpl::initAX(void* pArg)
{
    AXInit();
    return 0;
}

void PlatformSoundEngineImpl::ErrorCallback(long long iErrorCode, const char* pErrorText)
{
    AIL_last_error();
}

float PlatformSoundEngineImpl::custom_falloff_function(_SAMPLE* pSample, float fDistance, float fRolloffFactor,
                                                       float fMinDistance, float fMaxDistance)
{
    if (fMaxDistance == 10000.0f)
    {
        return 1.0f;
    }

    float fVolume = 1.0f - fDistance / fMaxDistance;

    if (fVolume < 0.0f)
    {
        fVolume = 0.0f;
    }
    else if (fVolume > 1.0f)
    {
        fVolume = 1.0f;
    }

    return fVolume;
}

void PlatformSoundEngineImpl::init(char* pSoundBankPath, SoundBankLoadedCallback pSoundBankLoaded)
{
    C4JThreadImpl* pAXInitThread;
    RADSS_OPEN_DRIVER_CALLBACK pProvider;
    char szSoundBankFile[256];

    RIB_load_static_provider_library(BinkADec_RIB_Main, "BinkADec");

    pAXInitThread = new C4JThreadImpl(initAX, NULL, "AXInitThread", 0);
    pAXInitThread->SetProcessor(2);
    pAXInitThread->Run();
    pAXInitThread->WaitForCompletion(-1);
    delete pAXInitThread;

    MIXInit();

    AIL_set_file_callbacks(file_open_callback, file_close_callback, file_seek_callback, file_read_callback);
    AIL_startup();

    pProvider = AIL_output_ax(3);

    m_pDigitalDriver = AIL_open_generic_digital_driver(32000, 16, 128, 0, pProvider);

    if (m_pDigitalDriver == NULL)
    {
        AIL_shutdown();
        return;
    }

#ifdef MINECRAFT_PC
    g_pcMilesDriver = m_pDigitalDriver;
#endif
    MilesSetEventErrorCallback(ErrorCallback);
    AIL_set_3D_rolloff_factor(m_pDigitalDriver, 1.0f);

    if (MilesStartupEventSystem(m_pDigitalDriver, 0x20000, 0, 0x40000) == 0)
    {
        AIL_close_digital_driver(m_pDigitalDriver);
        AIL_shutdown();
        return;
    }

    sprintf(szSoundBankFile, "%s", szSoundPath);
    strcat(szSoundBankFile, "Minecraft.msscmp");

    m_pSoundBank = MilesAddSoundBank(szSoundBankFile, 0);

    if (m_pSoundBank == NULL)
    {
        AIL_last_error();
        AIL_close_digital_driver(m_pDigitalDriver);
        AIL_shutdown();
        return;
    }

    MilesEnqueueEventByName("Minecraft/CacheSounds");
}

void PlatformSoundEngineImpl::tick(void* pUserData, float fListenerX, float fListenerY, float fListenerZ,
                                   float fListenerForwardX, float fListenerForwardY, float fListenerForwardZ,
                                   ConfigureSoundCallback pConfigureSound, UpdateSoundCallback pUpdateSound,
                                   RouteToDRCCallback pRouteToDRC, bool* pbSystemMusicPlaying)
{
    SoundInstanceInfo info;
    unsigned int uiNext;

    *pbSystemMusicPlaying = false;

    AIL_set_listener_3D_position(m_pDigitalDriver, fListenerX, fListenerY, fListenerZ);
    AIL_set_listener_3D_orientation(m_pDigitalDriver, fListenerForwardX, fListenerForwardY, fListenerForwardZ, 0.0f,
                                    1.0f, 0.0f);

    MilesBeginEventQueueProcessing();

    uiNext = 0xFFFFFFFF;

    while (MilesEnumerateSoundInstances(NULL, &uiNext, 0, NULL, 0, 0, &info) != 0)
    {
        info.m_sample->outputToDRC = pRouteToDRC(pUserData, const_cast<void*>(info.m_selectionData));

        switch (info.m_state)
        {
        case 1:
        {
            bool bIs3D;
            float fMinDistance;
            float fPlaybackRate;
            float fX;
            float fY;
            float fZ;
            float fVolume;

            pConfigureSound(pUserData, const_cast<void*>(info.m_selectionData), &bIs3D, &fMinDistance, &fPlaybackRate,
                            &fX, &fY, &fZ, &fVolume);

            AIL_register_falloff_function_callback(info.m_sample, custom_falloff_function);

            if (fPlaybackRate != 0.0f)
            {
                AIL_set_sample_playback_rate_factor(info.m_sample, fPlaybackRate);
            }

            if (bIs3D)
            {
                AIL_set_sample_is_3D(info.m_sample, 1);
                AIL_set_sample_3D_distances(info.m_sample, fMinDistance, 1.0f, 0);
                AIL_set_sample_3D_position(info.m_sample, fX, fY, fZ);
            }
            else
            {
                AIL_set_sample_is_3D(info.m_sample, 0);
            }

            AIL_set_sample_volume_levels(info.m_sample, fVolume, fVolume);

            pUpdateSound(pUserData, const_cast<void*>(info.m_selectionData), &bIs3D, 0, &fX, &fY, &fZ, &fVolume);
            break;
        }
        case 4:
        {
            bool bIs3D;
            float fX;
            float fY;
            float fZ;
            float fVolume;

            pUpdateSound(pUserData, const_cast<void*>(info.m_selectionData), &bIs3D, 2, &fX, &fY, &fZ, &fVolume);
            break;
        }
        default:
        {
            bool bIs3D;
            float fX;
            float fY;
            float fZ;
            float fVolume;

            pUpdateSound(pUserData, const_cast<void*>(info.m_selectionData), &bIs3D, 1, &fX, &fY, &fZ, &fVolume);

            if (bIs3D)
            {
                AIL_set_sample_3D_position(info.m_sample, fX, fY, fZ);
            }

            AIL_set_sample_volume_levels(info.m_sample, fVolume, fVolume);
            break;
        }
        }
    }

    MilesCompleteEventQueueProcessing();
#ifdef MINECRAFT_PC
    for (std::vector<_SAMPLE*>::iterator it = m_pcSamples.begin(); it != m_pcSamples.end();)
    {
        if (AIL_sample_status(*it) != kPcSamplePlaying)
        {
            AIL_release_sample_handle(*it);
            it = m_pcSamples.erase(it);
        }
        else
        {
            ++it;
        }
    }
#endif
}

#ifdef MINECRAFT_PC
void PlatformSoundEngineImpl::startSampleFile(const void* pFile, bool bIs3D, float fMinDistance, float fPlaybackRate,
                                              float fX, float fY, float fZ, float fVolume)
{
    if (m_pDigitalDriver == NULL || m_pcSamples.size() >= kPcSampleLimit)
        return;
    _SAMPLE* pSample = AIL_allocate_sample_handle(m_pDigitalDriver);
    if (pSample == NULL)
        return;
    if (AIL_set_sample_file(pSample, pFile, 0) == 0)
    {
        AIL_release_sample_handle(pSample);
        return;
    }
    pSample->outputToDRC = false;
    AIL_register_falloff_function_callback(pSample, custom_falloff_function);
    if (fPlaybackRate != 0.0f)
        AIL_set_sample_playback_rate_factor(pSample, fPlaybackRate);
    if (bIs3D)
    {
        AIL_set_sample_is_3D(pSample, 1);
        AIL_set_sample_3D_distances(pSample, fMinDistance, 1.0f, 0);
        AIL_set_sample_3D_position(pSample, fX, fY, fZ);
    }
    else
    {
        AIL_set_sample_is_3D(pSample, 0);
    }
    AIL_set_sample_volume_levels(pSample, fVolume, fVolume);
    AIL_start_sample(pSample);
    m_pcSamples.push_back(pSample);
}
#endif

void PlatformSoundEngineImpl::startSound(void* pSoundData, int iSoundDataSize, char* pEventName)
{
    unsigned int uiQueue;

    uiQueue = AIL_enqueue_event_start();
    AIL_enqueue_event_buffer(&uiQueue, pSoundData, iSoundDataSize, 0);
    AIL_enqueue_event_end_named(uiQueue, pEventName);
}

bool PlatformSoundEngineImpl::openStream(char* pFileName)
{
    m_pStreamDRC = AIL_open_stream(m_pDigitalDriver, pFileName, 0);

    if (m_pStreamDRC == NULL)
    {
        return false;
    }

    m_pStream = AIL_open_stream(m_pDigitalDriver, pFileName, 0);

    if (m_pStream == NULL)
    {
#ifdef MINECRAFT_PC
        AIL_close_stream(m_pStreamDRC);
        m_pStreamDRC = NULL;
#endif
        return false;
    }

    return true;
}

void PlatformSoundEngineImpl::startStream(void* pUserData, ConfigureSoundCallback pConfigureSound)
{
    _SAMPLE* pSample;
    _SAMPLE* pSampleDRC;
    bool bIs3D;
    float fMinDistance;
    float fPlaybackRate;
    float fX;
    float fY;
    float fZ;
    float fVolume;

    pConfigureSound(pUserData, NULL, &bIs3D, &fMinDistance, &fPlaybackRate, &fX, &fY, &fZ, &fVolume);

    pSample = AIL_stream_sample_handle(m_pStream);
    pSampleDRC = AIL_stream_sample_handle(m_pStreamDRC);
    pSampleDRC->outputToDRC = true;

    AIL_register_falloff_function_callback(pSample, custom_falloff_function);

    if (bIs3D)
    {
        AIL_set_sample_is_3D(pSample, 1);
        AIL_set_sample_3D_distances(pSample, fMinDistance, 1.0f, 0);
        AIL_set_sample_3D_position(pSample, fX, fY, fZ);
        AIL_set_sample_is_3D(pSampleDRC, 1);
        AIL_set_sample_3D_distances(pSampleDRC, fMinDistance, 1.0f, 0);
        AIL_set_sample_3D_position(pSampleDRC, fX, fY, fZ);
    }
    else
    {
        AIL_set_sample_is_3D(pSample, 0);
        AIL_set_sample_is_3D(pSampleDRC, 0);
    }

    AIL_set_sample_playback_rate_factor(pSample, fPlaybackRate);
    AIL_set_sample_volume_levels(pSample, fVolume, fVolume);
    AIL_start_stream(m_pStream);

    AIL_set_sample_playback_rate_factor(pSampleDRC, fPlaybackRate);
    AIL_set_sample_volume_levels(pSampleDRC, fVolume, fVolume);
    AIL_start_stream(m_pStreamDRC);
}

void PlatformSoundEngineImpl::setStreamVol(float fVolume)
{
    _SAMPLE* pSample;
    _SAMPLE* pSampleDRC;

    pSample = AIL_stream_sample_handle(m_pStream);
    AIL_set_sample_volume_levels(pSample, fVolume, fVolume);

    pSampleDRC = AIL_stream_sample_handle(m_pStreamDRC);
    pSampleDRC->outputToDRC = true;
    AIL_set_sample_volume_levels(pSampleDRC, fVolume, fVolume);
}

void PlatformSoundEngineImpl::stopStream()
{
    AIL_pause_stream(m_pStream, 1);
    AIL_close_stream(m_pStream);
    m_pStream = NULL;

    AIL_pause_stream(m_pStreamDRC, 1);
    AIL_close_stream(m_pStreamDRC);
    m_pStreamDRC = NULL;
}

void PlatformSoundEngineImpl::updateStreamPos(void* pUserData, UpdateSoundCallback pUpdateSound)
{
    bool bIs3D;
    float fX;
    float fY;
    float fZ;
    float fVolume;

    pUpdateSound(pUserData, NULL, &bIs3D, 1, &fX, &fY, &fZ, &fVolume);

    if (bIs3D)
    {
        _SAMPLE* pSample = AIL_stream_sample_handle(m_pStream);
        AIL_set_sample_3D_position(pSample, fX, fY, fZ);
    }
}

bool PlatformSoundEngineImpl::isStreamComplete()
{
    if (m_pStream != NULL && AIL_stream_status(m_pStream) == 2)
    {
        AIL_close_stream(m_pStream);
        m_pStream = NULL;
        AIL_close_stream(m_pStreamDRC);
        m_pStreamDRC = NULL;
        return true;
    }

    return false;
}

void PlatformSoundEngineImpl::shutdown()
{
    AIL_shutdown();
}
