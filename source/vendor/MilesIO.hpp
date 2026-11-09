#ifndef SOURCE_VENDOR_MILES_IO_HPP
#define SOURCE_VENDOR_MILES_IO_HPP

struct MilesAsyncRead;
struct SoundBank;
struct MilesMarkerList;
struct _DIG_DRIVER;
class EventSystem;

struct _MILESBANKSOUNDINFO
{
    int m_channels;
    int m_channelMask;
    int m_sampleRate;
    int m_dataSize;
    int m_maxInstances;
    int m_unbuffered;
    int m_reserved0;
    int m_streamBufferSize;
    int m_adpcm;
    int m_adpcmBlockSize;
    int m_extraParam;
};

typedef int (*MilesContainerGetSoundCallback)(const char* name, char* path, _MILESBANKSOUNDINFO* info);
typedef void* (*MilesContainerGetPresetCallback)(const char* name);
typedef void (*MilesAuditionAsyncQueuedCallback)(unsigned long long eventId, int auditionId, const char* name);
typedef void (*MilesAuditionAsyncLoadCallback)(int auditionId, int size);
typedef void (*MilesAuditionAsyncCancelCallback)(int auditionId);
typedef void (*MilesAuditionSetPersistCallback)(unsigned long long eventId, const char* name, const char* preset);
typedef void (*MilesAuditionRampStateCallback)(const char* name, unsigned long long eventId, int target);
typedef int (*MilesAuditionStatusCallback)();
typedef int (*MilesAuditionPumpCallback)();
typedef SoundBank* (*MilesAuditionOpenBankCallback)(const char* name);
typedef int (*MilesAuditionOpenCompleteCallback)(SoundBank* bank);
typedef void (*MilesAuditionCloseBankCallback)(SoundBank* bank);
typedef void (*MilesAuditionSuppressCallback)(int suppress);
typedef void (*MilesAuditionFrameCallback)();
typedef void (*MilesAuditionEventCallback)(const char* name, unsigned long long serial, unsigned long long context,
                                           int fromBank, const void* initBlock, int initBlockSize);
typedef void (*MilesAuditionSoundCallback)(float volume, float pitch, unsigned long long serial,
                                           unsigned long long soundId, const char* name, const char* labels, int delay);
typedef void (*MilesAuditionSoundIdCallback)(unsigned long long soundId);
typedef void (*MilesAuditionSoundFlagsCallback)(unsigned long long soundId, unsigned char flags);
typedef void (*MilesAuditionSoundLimitedCallback)(unsigned long long soundId, const char* limit);
typedef void (*MilesAuditionControlCallback)(unsigned long long serial, const char* name, int action,
                                             unsigned long long context);
typedef void (*MilesAuditionSoundBusCallback)(unsigned long long soundId, int bus);
typedef void (*MilesAuditionAsyncErrorCallback)(int auditionId);
typedef void (*MilesAuditionAsyncCompleteCallback)(int auditionId, int size);
typedef void (*MilesAuditionListenerPositionCallback)(float x, float y, float z);
typedef void (*MilesAuditionSoundPositionCallback)(float x, float y, float z, unsigned long long soundId);
typedef void (*MilesAuditionSendCPUCallback)(_DIG_DRIVER* driver);
typedef void (*MilesAuditionCountCallback)(int count);
typedef void (*MilesAuditionHandleSystemLoadCallback)(int used, int total);
typedef void (*MilesAuditionVarStateCallback)(const char* name, unsigned long long soundId, int isInteger,
                                              const void* value);
typedef void (*MilesAuditionSoundStateCallback)(float volume, float lowPass, float blendVolume, float blendPitch,
                                                float levelA, float levelB, float rate, float levelC,
                                                unsigned long long soundId);
typedef void (*MilesAuditionClearStateCallback)();
typedef void (*MilesAuditionCompletionEventCallback)(unsigned long long completionEvent, unsigned long long soundId);
typedef void (*MilesAuditionAddRampCallback)(unsigned long long soundId, int target, const char* name,
                                             const char* labels, int global, unsigned long long eventId);

typedef void (*MilesContainerFreeAllCallback)();
typedef void* (*MilesContainerGetEnvironmentCallback)(const char* name);
typedef const unsigned char* (*MilesContainerGetEventCallback)(const char* name);
typedef const MilesMarkerList* (*MilesContainerGetMarkerListCallback)(const char* name);
typedef int (*MilesContainerGetLoadedCountCallback)();
typedef int (*MilesContainerGetMemUsageCallback)();
typedef const char* (*MilesContainerGetLoadedNameCallback)(int index);

typedef unsigned int (*AIL_FILE_OPEN_CALLBACK)(const char* pFileName, unsigned int* puFileHandle);
typedef void (*AIL_FILE_CLOSE_CALLBACK)(unsigned int uFileHandle);
typedef int (*AIL_FILE_SEEK_CALLBACK)(unsigned int uFileHandle, int iOffset, unsigned int uType);
typedef int (*AIL_FILE_READ_CALLBACK)(unsigned int uFileHandle, void* pBuffer, unsigned int uBytes);

extern "C" AIL_FILE_OPEN_CALLBACK AIL_file_open;
extern "C" AIL_FILE_CLOSE_CALLBACK AIL_file_close;
extern "C" AIL_FILE_SEEK_CALLBACK AIL_file_seek;
extern "C" AIL_FILE_READ_CALLBACK AIL_file_read;

unsigned int DefaultFileOpen(const char* pFileName, unsigned int* puFileHandle);
void DefaultFileClose(unsigned int uFileHandle);
int DefaultFileSeek(unsigned int uFileHandle, int iOffset, unsigned int uType);
int DefaultFileRead(unsigned int uFileHandle, void* pBuffer, unsigned int uBytes);

void CloseAsync(MilesAsyncRead* pRead);

extern MilesContainerGetSoundCallback g_milesContainerGetSoundCallback;
extern MilesContainerGetPresetCallback g_milesContainerGetPresetCallback;
extern MilesAuditionAsyncQueuedCallback g_milesAuditionAsyncQueuedCallback;
extern MilesAuditionAsyncLoadCallback g_milesAuditionAsyncLoadCallback;
extern MilesAuditionAsyncCancelCallback g_milesAuditionAsyncCancelCallback;
extern MilesAuditionSetPersistCallback g_milesAuditionSetPersistCallback;
extern MilesAuditionRampStateCallback g_milesAuditionRampStateCallback;
extern MilesAuditionStatusCallback g_milesAuditionStatusCallback;
extern MilesAuditionPumpCallback g_milesAuditionPumpCallback;
extern MilesAuditionOpenBankCallback g_milesAuditionOpenBankCallback;
extern MilesAuditionOpenCompleteCallback g_milesAuditionOpenCompleteCallback;
extern MilesAuditionCloseBankCallback g_milesAuditionCloseBankCallback;
extern MilesAuditionSuppressCallback g_milesAuditionSuppressCallback;
extern MilesAuditionFrameCallback g_milesAuditionFrameStartCallback;
extern MilesAuditionFrameCallback g_milesAuditionFrameEndCallback;
extern MilesAuditionFrameCallback g_milesAuditionDefragStartCallback;
extern MilesAuditionEventCallback g_milesAuditionEventCallback;
extern MilesAuditionSoundCallback g_milesAuditionSoundCallback;
extern MilesAuditionSoundIdCallback g_milesAuditionSoundCompleteCallback;
extern MilesAuditionSoundIdCallback g_milesAuditionSoundPlayingCallback;
extern MilesAuditionSoundFlagsCallback g_milesAuditionSoundFlagsCallback;
extern MilesAuditionSoundLimitedCallback g_milesAuditionSoundLimitedCallback;
extern MilesAuditionControlCallback g_milesAuditionControlCallback;
extern MilesAuditionSoundBusCallback g_milesAuditionSoundBusCallback;
extern MilesAuditionAsyncErrorCallback g_milesAuditionAsyncErrorCallback;
extern MilesAuditionAsyncCompleteCallback g_milesAuditionAsyncCompleteCallback;
extern MilesAuditionListenerPositionCallback g_milesAuditionListenerPositionCallback;
extern MilesAuditionSoundPositionCallback g_milesAuditionSoundPositionCallback;
extern MilesAuditionSendCPUCallback g_milesAuditionSendCPUCallback;
extern MilesAuditionCountCallback g_milesAuditionUpdateDataCountCallback;
extern MilesAuditionCountCallback g_milesAuditionSendCountCallback;
extern MilesAuditionHandleSystemLoadCallback g_milesAuditionHandleSystemLoadCallback;
extern MilesAuditionVarStateCallback g_milesAuditionVarStateCallback;
extern MilesAuditionSoundStateCallback g_milesAuditionSoundStateCallback;
extern MilesAuditionClearStateCallback g_milesAuditionClearStateCallback;
extern MilesAuditionCompletionEventCallback g_milesAuditionCompletionEventCallback;
extern MilesAuditionAddRampCallback g_milesAuditionAddRampCallback;
extern MilesContainerFreeAllCallback g_milesContainerFreeAllCallback;
extern MilesContainerGetEnvironmentCallback g_milesContainerGetEnvironmentCallback;
extern MilesContainerGetEventCallback g_milesContainerGetEventCallback;
extern MilesContainerGetMarkerListCallback g_milesContainerGetMarkerListCallback;
extern MilesContainerGetLoadedCountCallback g_milesContainerGetLoadedCountCallback;
extern MilesContainerGetMemUsageCallback g_milesContainerGetMemUsageCallback;
extern MilesContainerGetLoadedNameCallback g_milesContainerGetLoadedNameCallback;
extern EventSystem* g_eventSystem;

SoundBank* GetContainer(const char* name, const char** assetName);
int Default_GetSound(const char* name, char* path, _MILESBANKSOUNDINFO* info);
void* Default_GetPreset(const char* name);
void* Default_GetEnvironment(const char* name);
const unsigned char* Default_GetEvent(const char* name);
const MilesMarkerList* Default_GetMarkerList(const char* name);
int Default_GetLoadedCount();
int Default_GetMemUsage();
const char* Default_GetLoadedName(int index);
void Default_FreeAll();
void EvalFunc(void* instance, const char* name, int index, void* value, int flags);

extern "C" int Container_GetSound(const char* name, char* path, _MILESBANKSOUNDINFO* info);
extern "C" void* Container_GetPreset(const char* name);
extern "C" void* Container_GetEnvironment(const char* name);
extern "C" const unsigned char* Container_GetEvent(const char* name);
extern "C" const MilesMarkerList* Container_GetMarkerList(const char* name);
extern "C" int Audition_Status();
extern "C" int Audition_Pump();
extern "C" SoundBank* Audition_OpenBank(const char* name);
extern "C" int Audition_OpenComplete(SoundBank* bank);
extern "C" void Audition_CloseBank(SoundBank* bank);
extern "C" void Audition_Suppress(int suppress);
extern "C" void Audition_FrameStart();
extern "C" void Audition_FrameEnd();
extern "C" void Audition_DefragStart();
extern "C" void Audition_Event(const char* name, unsigned long long serial, unsigned long long context, int fromBank,
                               const void* initBlock, int initBlockSize);
extern "C" void Audition_Sound(float volume, float pitch, unsigned long long serial, unsigned long long soundId,
                               const char* name, const char* labels, int delay);
extern "C" void Audition_SoundComplete(unsigned long long soundId);
extern "C" void Audition_SoundPlaying(unsigned long long soundId);
extern "C" void Audition_SoundFlags(unsigned long long soundId, unsigned char flags);
extern "C" void Audition_SoundLimited(unsigned long long soundId, const char* limit);
extern "C" void Audition_Control(unsigned long long serial, const char* name, int action, unsigned long long context);
extern "C" void Audition_SoundBus(unsigned long long soundId, int bus);
extern "C" void Audition_AsyncError(int auditionId);
extern "C" void Audition_AsyncComplete(int auditionId, int size);
extern "C" void Audition_ListenerPosition(float x, float y, float z);
extern "C" void Audition_SoundPosition(float x, float y, float z, unsigned long long soundId);
extern "C" void Audition_SendCPU(_DIG_DRIVER* driver);
extern "C" void Audition_UpdateDataCount(int count);
extern "C" void Audition_SendCount(int count);
extern "C" void Audition_HandleSystemLoad(int used, int total);
extern "C" void Audition_VarState(const char* name, unsigned long long soundId, int isInteger, const void* value);
extern "C" void Audition_SoundState(float volume, float lowPass, float blendVolume, float blendPitch, float levelA,
                                    float levelB, float rate, float levelC, unsigned long long soundId);
extern "C" void Audition_ClearState();
extern "C" void Audition_CompletionEvent(unsigned long long completionEvent, unsigned long long soundId);
extern "C" void Audition_AddRamp(unsigned long long soundId, int target, const char* name, const char* labels,
                                 int global, unsigned long long eventId);
extern "C" void Audition_AsyncQueued(unsigned long long eventId, int auditionId, const char* name);
extern "C" void Audition_AsyncLoad(int auditionId, int size);
extern "C" void Audition_AsyncCancel(int auditionId);
extern "C" void Audition_SetPersist(unsigned long long eventId, const char* name, const char* preset);
extern "C" void Audition_RampState(const char* name, unsigned long long eventId, int target);

#endif
