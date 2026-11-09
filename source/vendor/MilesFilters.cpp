#include "vendor/MilesFilters.hpp"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesApi.hpp"

extern "C" void SS_set_speaker_configuration(SpeakerState* pState, const float* pPositions, unsigned int uCount,
                                             unsigned int uSpeakerConfig);

struct ReverbPreset
{
    float fWetLevel;
    float fReverbTime;
    float fDamping;
    float fPreDelay;
};

static const int FALLOFF_VOLUME = 0;
static const int FALLOFF_EXCLUSION = 1;
static const int FALLOFF_LOWPASS = 2;
static const float DRY_LEVEL = 1.0f;
static const float VECTOR_EPSILON = 0.0001f;

static ReverbPreset ReverbPresets[] = {
    {0.0f, 1.493f, 0.5f, 0.011f},     {0.25f, 0.1f, 0.1f, 0.002f},      {0.417f, 0.4f, 0.666f, 0.003f},
    {0.75f, 1.499f, 0.166f, 0.011f},  {0.208f, 0.478f, 0.1f, 0.004f},   {0.3f, 2.309f, 0.68f, 0.017f},
    {0.403f, 4.279f, 0.5f, 0.03f},    {0.5f, 3.961f, 0.5f, 0.029f},     {0.5f, 2.886f, 1.304f, 0.022f},
    {0.361f, 7.284f, 0.332f, 0.03f},  {0.5f, 10.0f, 0.3f, 0.03f},       {0.153f, 0.259f, 0.55f, 0.03f},
    {0.361f, 1.493f, 0.5f, 0.011f},   {0.444f, 2.697f, 0.638f, 0.02f},  {0.25f, 1.752f, 0.776f, 0.011f},
    {0.111f, 3.145f, 0.472f, 0.088f}, {0.111f, 2.767f, 0.224f, 0.011f}, {0.2f, 0.8f, 0.2f, 0.3f},
    {0.55f, 3.0f, 0.55f, 0.043f},     {0.12f, 0.8f, 0.18f, 0.1f},       {0.208f, 1.652f, 1.5f, 0.012f},
    {0.652f, 2.886f, 0.25f, 0.021f},  {1.0f, 1.499f, 0.1f, 0.011f},     {0.875f, 8.392f, 1.388f, 0.011f},
    {0.139f, 10.0f, 1.5f, 0.011f},    {0.486f, 7.563f, 2.5f, 0.011f},
};

FLT_INSTANCE* FLT_list = NULL;

extern "C" void AIL_API_set_room_type(_DIG_DRIVER* pDriver, int iBus, int iRoomType)
{
    if (pDriver == NULL)
    {
        return;
    }

    pDriver->buses[iBus].roomType = iRoomType;
    AIL_set_digital_master_reverb_levels(pDriver, iBus, DRY_LEVEL, ReverbPresets[iRoomType].fWetLevel);
    AIL_set_digital_master_reverb(pDriver, iBus, ReverbPresets[iRoomType].fReverbTime,
                                  ReverbPresets[iRoomType].fPreDelay, ReverbPresets[iRoomType].fDamping);
}

extern "C" void AIL_API_set_3D_rolloff_factor(_DIG_DRIVER* pDriver, float fFactor)
{
    if (pDriver == NULL)
    {
        return;
    }

    pDriver->speakers.rolloffFactor = fFactor;
}

extern "C" void AIL_API_set_3D_doppler_factor(_DIG_DRIVER* pDriver, float fFactor)
{
    if (pDriver == NULL)
    {
        return;
    }

    pDriver->speakers.dopplerFactor = fFactor;
}

extern "C" void AIL_API_set_listener_3D_position(_DIG_DRIVER* pDriver, float fX, float fY, float fZ)
{
    if (pDriver == NULL)
    {
        return;
    }

    pDriver->speakers.listenerPosition[0] = fX;
    pDriver->speakers.listenerPosition[1] = fY;
    pDriver->speakers.listenerPosition[2] = fZ;
}

extern "C" void AIL_API_set_listener_3D_orientation(_DIG_DRIVER* pDriver, float fFaceX, float fFaceY, float fFaceZ,
                                                    float fUpX, float fUpY, float fUpZ)
{
    if (pDriver == NULL)
    {
        return;
    }

    float fLength = (float)sqrt(fFaceX * fFaceX + fFaceY * fFaceY + fFaceZ * fFaceZ);
    if (fLength > VECTOR_EPSILON)
    {
        fFaceX = fFaceX / fLength;
        fFaceY = fFaceY / fLength;
        fFaceZ = fFaceZ / fLength;
    }

    fLength = (float)sqrt(fUpX * fUpX + fUpY * fUpY + fUpZ * fUpZ);
    if (fLength > VECTOR_EPSILON)
    {
        fUpX = fUpX / fLength;
        fUpY = fUpY / fLength;
        fUpZ = fUpZ / fLength;
    }

    float fRightX = fFaceZ * fUpY - fFaceY * fUpZ;
    float fRightY = fFaceX * fUpZ - fFaceZ * fUpX;
    float fRightZ = fFaceY * fUpX - fFaceX * fUpY;

    pDriver->speakers.listenerUp[2] = fUpZ;
    pDriver->speakers.listenerFace[0] = fFaceX;
    pDriver->speakers.listenerFace[1] = fFaceY;
    pDriver->speakers.listenerFace[2] = fFaceZ;
    pDriver->speakers.listenerUp[0] = fUpX;
    pDriver->speakers.listenerCross[2] = fRightZ;
    pDriver->speakers.listenerUp[1] = fUpY;
    pDriver->speakers.listenerCross[0] = fRightX;
    pDriver->speakers.listenerCross[1] = fRightY;

    fLength = (float)sqrt(fRightX * fRightX + fRightY * fRightY + fRightZ * fRightZ);
    if (fLength < VECTOR_EPSILON)
    {
        pDriver->speakers.listenerCross[0] = 1.0f;
        pDriver->speakers.listenerCross[1] = 0.0f;
        pDriver->speakers.listenerCross[2] = 0.0f;
    }
    else
    {
        pDriver->speakers.listenerCross[0] = pDriver->speakers.listenerCross[0] / fLength;
        pDriver->speakers.listenerCross[1] = pDriver->speakers.listenerCross[1] / fLength;
        pDriver->speakers.listenerCross[2] = pDriver->speakers.listenerCross[2] / fLength;
    }

    SS_set_speaker_configuration(&pDriver->speakers, NULL, 0, pDriver->speakerConfig);
}

extern "C" void AIL_API_set_sample_3D_volume_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iCount)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iCount > SAMPLE_FALLOFF_POINT_COUNT)
    {
        return;
    }

    pSample->falloffPointCounts[FALLOFF_VOLUME] = (char)iCount;
    memcpy(pSample->falloffPoints[FALLOFF_VOLUME], pPoints, iCount * sizeof(FalloffGraphPoint));
}

extern "C" void AIL_API_set_sample_3D_lowpass_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iCount)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iCount > SAMPLE_FALLOFF_POINT_COUNT)
    {
        return;
    }

    pSample->falloffPointCounts[FALLOFF_LOWPASS] = (char)iCount;
    memcpy(pSample->falloffPoints[FALLOFF_LOWPASS], pPoints, iCount * sizeof(FalloffGraphPoint));
}

extern "C" void AIL_API_set_sample_3D_exclusion_falloff(_SAMPLE* pSample, const FalloffGraphPoint* pPoints, int iCount)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iCount > SAMPLE_FALLOFF_POINT_COUNT)
    {
        return;
    }

    pSample->falloffPointCounts[FALLOFF_EXCLUSION] = (char)iCount;
    memcpy(pSample->falloffPoints[FALLOFF_EXCLUSION], pPoints, iCount * sizeof(FalloffGraphPoint));
}

extern "C" void AIL_API_set_sample_3D_position(_SAMPLE* pSample, float fX, float fY, float fZ)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->is3D = 1;
    pSample->x = fX;
    pSample->y = fY;
    pSample->z = fZ;
}

extern "C" void AIL_API_set_sample_3D_distances(_SAMPLE* pSample, float fMaximum, float fMinimum,
                                                int iAutomaticWetAttenuation)
{
    if (pSample != NULL)
    {
        if (fMinimum > fMaximum)
        {
            float fSwap = fMinimum;
            fMinimum = fMaximum;
            fMaximum = fSwap;
        }

        InMilesMutex();
        pSample->maximumDistance = fMaximum;
        pSample->minimumDistance = fMinimum;
        pSample->distancesChanged = 1;
        pSample->automaticWetAttenuation = iAutomaticWetAttenuation;
        OutMilesMutex();
    }
}

extern "C" void FLT_init_list()
{
    FLT_list = NULL;
}

extern "C" void FLT_disconnect_driver(_DIG_DRIVER* pDriver)
{
    FLT_INSTANCE* pFilter = FLT_list;
    while (pFilter != NULL)
    {
        if (pFilter->driver == pDriver)
        {
            AIL_close_filter(pFilter);
            pFilter = FLT_list;
        }
        else
        {
            pFilter = pFilter->next;
        }
    }
}

extern "C" void FLT_call_premix_processors(_DIG_DRIVER* pDriver)
{
    for (FLT_INSTANCE* pFilter = FLT_list; pFilter != NULL; pFilter = pFilter->next)
    {
        if (pFilter->driver == pDriver)
        {
            pFilter->premixProcess(pFilter->driverState);
        }
    }
}

extern "C" int FLT_call_postmix_processors(_DIG_DRIVER* pDriver, short* pBuffer)
{
    int iProcessed = 0;
    for (FLT_INSTANCE* pFilter = FLT_list; pFilter != NULL; pFilter = pFilter->next)
    {
        if (pFilter->driver == pDriver && pFilter->postmixProcess(pFilter->driverState, pBuffer) != 0)
        {
            iProcessed = 1;
        }
    }

    return iProcessed;
}

extern "C" void AIL_API_close_filter(FLT_INSTANCE* pFilter)
{
    if (pFilter == NULL)
    {
        return;
    }

    pFilter->closeDriver(pFilter->driverState);
    if (pFilter->driverState != NULL)
    {
        AIL_mem_free_lock(pFilter->driverState);
        pFilter->driverState = NULL;
    }

    if (FLT_list == pFilter)
    {
        FLT_list = pFilter->next;
    }
    else
    {
        FLT_INSTANCE* pPrevious = NULL;
        for (FLT_INSTANCE* pCurrent = FLT_list; pCurrent != pFilter; pCurrent = pCurrent->next)
        {
            pPrevious = pCurrent;
        }

        pPrevious->next = pFilter->next;
    }

    _DIG_DRIVER* pDriver = pFilter->driver;
    if (pDriver->matrixFilter == pFilter)
    {
        pDriver->matrixFilter = NULL;
    }

    AIL_mem_free_lock(pFilter);
}
