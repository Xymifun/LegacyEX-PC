#ifndef SOURCE_VENDOR_MILES_EVENT_ERROR_HPP
#define SOURCE_VENDOR_MILES_EVENT_ERROR_HPP

struct _SAMPLE;

enum ErrorType
{
    ERROR_TYPE_OUT_OF_MEMORY = 0,
    ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY = 1,
    ERROR_TYPE_NO_PROVIDER = 2,
    ERROR_TYPE_BAD_STRING_PASSED_TO_SET_LIMITS = 3,
    ERROR_TYPE_COMMAND_BUFFER_FULL = 4,
    ERROR_TYPE_MISSING_VARIABLE = 5,
    ERROR_TYPE_SOUND_NOT_FOUND = 6,
    ERROR_TYPE_ASYNC_LOAD_FAILED = 7,
    ERROR_TYPE_TOO_MANY_BLEND_SOUNDS = 8,
    ERROR_TYPE_BLEND_NOT_SET = 9,
    ERROR_TYPE_ENQUEUE_FULL = 10,
    ERROR_TYPE_SELECTION_MISSING = 11
};

typedef void (*MilesEventErrorCallback)(long long context, const char* text);
typedef void (*MilesAuditionSoundEvictedCallback)(unsigned long long instanceId, unsigned long long eventId,
                                                  unsigned int policy);
extern MilesEventErrorCallback g_milesEventErrorCallback;
extern MilesEventErrorCallback g_milesAuditionErrorCallback;
extern MilesAuditionSoundEvictedCallback g_milesAuditionSoundEvictedCallback;
extern "C" void MilesSetEventErrorCallback(MilesEventErrorCallback callback);
extern "C" void MilesSetEventErrorCallbackInternal(MilesEventErrorCallback callback);
extern "C" void Audition_Error(long long context, const char* text);
extern "C" void Audition_SoundEvicted(unsigned long long instanceId, unsigned long long eventId, unsigned int policy);
extern "C" void AIL_set_sample_3D_cone(_SAMPLE* pSample, float fInnerAngle, float fOuterAngle, float fOuterVolume);
extern "C" void AIL_set_sample_3D_orientation(_SAMPLE* pSample, float fForwardX, float fForwardY, float fForwardZ,
                                              float fUpX, float fUpY, float fUpZ);
extern "C" void AIL_set_sample_3D_velocity(_SAMPLE* pSample, float fDirectionX, float fDirectionY, float fDirectionZ,
                                           float fMagnitude);
extern "C" void AIL_set_sample_occlusion(_SAMPLE* pSample, float fOcclusion);
extern "C" void AIL_set_sample_exclusion(_SAMPLE* pSample, float fExclusion);
extern "C" void AIL_set_sample_obstruction(_SAMPLE* pSample, float fObstruction);
extern "C" int AIL_sample_stage_property(_SAMPLE* pSample, int iStage, const char* pName, int iChannel, void* pBefore,
                                         const void* pNewValue, void* pAfter);
void ErrorHandler(ErrorType type, unsigned long long context, const char* text);

#endif
