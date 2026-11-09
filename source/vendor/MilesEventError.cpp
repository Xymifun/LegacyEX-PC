#include "vendor/MilesEventError.hpp"
#include <stddef.h>
#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesRib.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesMem.hpp"

MilesEventErrorCallback g_milesEventErrorCallback = 0;
MilesEventErrorCallback g_milesAuditionErrorCallback = 0;
MilesAuditionSoundEvictedCallback g_milesAuditionSoundEvictedCallback = 0;

namespace
{
const unsigned int RIB_PROPERTY = 1;
}

extern "C" void MilesSetEventErrorCallbackInternal(MilesEventErrorCallback callback)
{
    g_milesEventErrorCallback = callback;
}

extern "C" void MilesSetEventErrorCallback(MilesEventErrorCallback callback)
{
    ++AIL_debug_depth;
    if (AIL_debug_enabled && (AIL_debug_depth == 1 || AIL_debug_nested) && AIL_API_background() == 0)
        AIL_debug_log("AIL_set_event_error_callback(0x%lX)", callback);
    InMilesMutex();
    MilesSetEventErrorCallbackInternal(callback);
    OutMilesMutex();
    --AIL_debug_depth;
}

extern "C" void Audition_Error(long long context, const char* text)
{
    if (g_milesAuditionErrorCallback)
        g_milesAuditionErrorCallback(context, text);
}

extern "C" void Audition_SoundEvicted(unsigned long long instanceId, unsigned long long eventId, unsigned int policy)
{
    if (g_milesAuditionSoundEvictedCallback)
        g_milesAuditionSoundEvictedCallback(instanceId, eventId, policy);
}

void ErrorHandler(ErrorType type, unsigned long long context, const char* text)
{
    switch (type)
    {
    case ERROR_TYPE_OUT_OF_MEMORY:
        AIL_set_error("MilesEvent: Out Of Memory (AIL_mem_alloc_lock failed).");
        break;
    case ERROR_TYPE_BAD_STRING_PASSED_TO_SET_LIMITS:
        AIL_set_error("MilesEvent: Bad string passed to SetLimits.");
        break;
    case ERROR_TYPE_COMMAND_BUFFER_FULL:
        AIL_set_error("MilesEvent: The command buffer was full.");
        break;
    case ERROR_TYPE_MISSING_VARIABLE:
        AIL_set_error("MilesEvent: Missing Variable.");
        break;
    case ERROR_TYPE_SOUND_NOT_FOUND:
        AIL_set_error("MilesEvent: An asset requested by an event was not found in memory.");
        break;
    case ERROR_TYPE_ASYNC_LOAD_FAILED:
        AIL_set_error("MilesEvent: An asynchronous load returned failure (file doesn't exist?).");
        break;
    case ERROR_TYPE_TOO_MANY_BLEND_SOUNDS:
        AIL_set_error("MilesEvent: Too many sounds were start in a blended sound.");
        break;
    case ERROR_TYPE_BLEND_NOT_SET:
        AIL_set_error("MilesEvent: The requested blend has not been set as it doesn't exist.");
        break;
    case ERROR_TYPE_ENQUEUE_FULL:
        AIL_set_error("MilesEvent: The enqueue buffer is full.");
        break;
    case ERROR_TYPE_SELECTION_MISSING:
        AIL_set_error("MilesEvent: The event expected a selection that did not exist.");
        break;
    case ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY:
        AIL_set_error("MilesEvent: Out of event system memory (pool passed to event system startup "
                      "exhausted).");
        break;
    }
    if (g_milesEventErrorCallback)
        g_milesEventErrorCallback(static_cast<long long>(context), text);
    Audition_Error(static_cast<long long>(context), text);
}

extern "C" void AIL_set_sample_3D_cone(_SAMPLE* pSample, float fInnerAngle, float fOuterAngle, float fOuterVolume)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_cone(0x%lX,%f,%f,%f)", pSample, fInnerAngle, fOuterAngle, fOuterVolume);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_cone(pSample, fInnerAngle, fOuterAngle, fOuterVolume);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_3D_orientation(_SAMPLE* pSample, float fForwardX, float fForwardY, float fForwardZ,
                                              float fUpX, float fUpY, float fUpZ)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_orientation(0x%lX,%f,%f,%f,%f,%f,%f)", pSample, fForwardX, fForwardY,
                      fForwardZ, fUpX, fUpY, fUpZ);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_orientation(pSample, fForwardX, fForwardY, fForwardZ, fUpX, fUpY, fUpZ);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_3D_velocity(_SAMPLE* pSample, float fDirectionX, float fDirectionY, float fDirectionZ,
                                           float fMagnitude)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_3D_velocity(0x%lX,%f,%f,%f,%f)", pSample, fDirectionX, fDirectionY, fDirectionZ,
                      fMagnitude);
    }

    InMilesMutex();
    AIL_API_set_sample_3D_velocity(pSample, fDirectionX, fDirectionY, fDirectionZ, fMagnitude);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_occlusion(_SAMPLE* pSample, float fOcclusion)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_occlusion(0x%lX,%s)", pSample, fOcclusion);
    }

    InMilesMutex();
    AIL_API_set_sample_occlusion(pSample, fOcclusion);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_exclusion(_SAMPLE* pSample, float fExclusion)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_exclusion(0x%lX,%f)", pSample, fExclusion);
    }

    InMilesMutex();
    AIL_API_set_sample_exclusion(pSample, fExclusion);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void AIL_set_sample_obstruction(_SAMPLE* pSample, float fObstruction)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_set_sample_obstruction(0x%lX,%f)", pSample, fObstruction);
    }

    InMilesMutex();
    AIL_API_set_sample_obstruction(pSample, fObstruction);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_sample_stage_property(_SAMPLE* pSample, int iStage, const char* pName, int iChannel, void* pBefore,
                                         const void* pNewValue, void* pAfter)
{
    int iResult = 0;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_stage_property(0x%lX,%ld,%s,%ld,0x%lX,0x%lX,0x%lX)", pSample, iStage, pName,
                      iChannel, pBefore, pNewValue, pAfter);
    }

    InMilesMutex();

    if (pSample != NULL && static_cast<unsigned int>(iStage - 1) < SAMPLE_FILTER_STAGE_COUNT)
    {
        FilterSamplePropertyFunc pProperty;
        unsigned int uToken;
        if (pSample->stages[iStage].filter.active != 0 && pSample->stages[iStage].filter.instance != NULL
            && (pProperty = pSample->stages[iStage].filter.instance->sampleProperty) != NULL
            && RIB_request_interface_entry(pSample->stages[iStage].filter.provider, "Pipeline filter sample services",
                                           RIB_PROPERTY, pName, &uToken)
                   == 0)
        {
            if (iChannel == -1)
            {
                for (int iIndex = 0; iIndex < pSample->channelCount; ++iIndex)
                {
                    iResult = pProperty(pSample->stages[iStage].filter.handles[iIndex], uToken, pBefore, pNewValue,
                                        pAfter);
                }
            }
            else
            {
                if (pSample->channelMap != NULL)
                {
                    iChannel = pSample->channelMap[iChannel];
                }
                else if (iChannel >= pSample->channelCount)
                {
                    iChannel = -1;
                }
                if (iChannel != -1)
                {
                    iResult = pProperty(pSample->stages[iStage].filter.handles[iChannel], uToken, pBefore, pNewValue,
                                        pAfter);
                }
            }
        }
    }

    OutMilesMutex();

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}
