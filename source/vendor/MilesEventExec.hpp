#ifndef SOURCE_VENDOR_MILES_EVENT_EXEC_HPP
#define SOURCE_VENDOR_MILES_EVENT_EXEC_HPP

#include "game/client/audio/EventSystem.hpp"
#include "vendor/MilesAsync.hpp"
#if __SIZEOF_POINTER__ == 8
#include <stddef.h>

#include "game/client/audio/FalloffGraph.hpp"
#endif

struct SoundBank;
struct _DIG_DRIVER;

const int SOUND_LOAD_SLOT_COUNT = 4;
#ifndef MINECRAFT_PC
const int SOUND_ENTRY_SIZE = 0x3c;
#endif
const int SOUND_EXTENSION_LIMIT = 8;
const int RAMP_NAME_SIZE = 0x40;
const int RAMP_LABELS_SIZE = 0x100;
const int RAMP_VALUE_SIZE = 0x40;
const int RAMP_SIZE = 0x1ac;
const int EVENT_QUEUE_SCRATCH_SIZE = 0x4000;
const int EVENT_STEP_BLEND_LIMIT = 10;

enum EventStepKind
{
    EVENT_STEP_END = 0,
    EVENT_STEP_PLAY = 1,
    EVENT_STEP_CONTROL = 2,
    EVENT_STEP_ENVIRONMENT = 3,
    EVENT_STEP_SOUND_ADD = 4,
    EVENT_STEP_CACHE = 5,
    EVENT_STEP_UNCACHE = 6,
    EVENT_STEP_LIMIT = 7,
    EVENT_STEP_PERSIST = 8,
    EVENT_STEP_VERSION = 9,
    EVENT_STEP_RAMP = 10,
    EVENT_STEP_BLEND = 11,
    EVENT_STEP_RESET = 12,
    EVENT_STEP_ENQUEUE = 13,
    EVENT_STEP_LIMIT_SELECT = 14,
    EVENT_STEP_LFO = 15,
    EVENT_STEP_TWEEN = 16
};

struct EventListNode
{
    Handle m_next;
    Handle m_self;
};

struct LoadedSound
{
    Handle m_name;
    unsigned int m_provider;
    void* m_data;
    unsigned int m_nameLength;
    int m_size;
    int m_loadFailed;
    int m_refCount;
    int m_loadSlot;
    int m_auditionId;
    int m_maxInstances;
    int m_playCount;
    int m_extraParam;
    char m_extension[SOUND_EXTENSION_LIMIT];
};

#ifdef MINECRAFT_PC
const int SOUND_ENTRY_SIZE = sizeof(LoadedSound);
#endif

struct EventSystemVariable
{
    Handle m_next;
    Handle m_self;
    union
    {
        int m_intValue;
        float m_floatValue;
    } m_value;
    int m_isFloat;
    Handle m_curve;
    char m_name[1];
};

struct Ramp
{
    Handle m_next;
    Handle m_self;
    float m_elapsed;
    float m_duration;
    float m_startValue;
    float m_cachedValue;
    int m_cacheValid;
    int m_curve;
    int m_target;
    char m_value[RAMP_VALUE_SIZE];
    char m_name[RAMP_NAME_SIZE];
    char m_labels[RAMP_LABELS_SIZE];
};

struct PersistNode
{
    Handle m_next;
    Handle m_self;
    Handle m_preset;
    Handle m_labels;
    int m_active;
    char m_name[1];
};

struct LimitList
{
    Handle m_next;
    Handle m_self;
    Handle m_entries;
    char m_name[1];
};

struct LimitEntry
{
    Handle m_next;
    Handle m_self;
    short m_limit;
    short m_count;
    char m_query[1];
};

struct LfoNode
{
    Handle m_next;
    Handle m_self;
    float m_center;
    float m_amplitude;
    float m_frequency;
    Handle m_centerText;
    Handle m_amplitudeText;
    Handle m_frequencyText;
    float m_time;
    unsigned char m_inverted;
    unsigned char m_mode;
    unsigned char m_flip;
    unsigned char m_offset;
    char m_name[1];
};

struct BusNode
{
    Handle m_next;
    Handle m_self;
    Handle m_preset;
    _SAMPLE* m_sample;
    unsigned char m_index;
    unsigned char m_presetApplied;
    unsigned char m_presetEnabled;
    char m_name[1];
};

const int TWEEN_SEGMENT_COUNT = 2;
const int TWEEN_VALUE_COUNT = 3;

struct TweenNode
{
    Handle m_next;
    Handle m_self;
    float m_values[TWEEN_VALUE_COUNT];
    float m_times[TWEEN_SEGMENT_COUNT];
    int m_curves[TWEEN_SEGMENT_COUNT];
    float m_elapsed;
    char m_name[1];
};

enum InitRecordTag
{
    INIT_RECORD_END = 0,
    INIT_RECORD_POSITION = 1,
    INIT_RECORD_SUFFIX = 2,
    INIT_RECORD_SELECTION = 3,
    INIT_RECORD_VARIABLE = 4,
    INIT_RECORD_VELOCITY = 0xb
};

struct InitRecord
{
    int tag;
    int size;
    union
    {
        float values[4];
        int integers[4];
    };
};

struct BlendListNode
{
    Handle m_next;
    Handle m_self;
    float m_curves[6 * MAX_BLEND_POINTS];
    char m_name[1];
};

#if __SIZEOF_POINTER__ == 8
const int SAMPLE_MARKER_LIST_OFFSET = offsetof(_SAMPLE, pcMarkerList);
#else
const int SAMPLE_MARKER_LIST_OFFSET = 0x224;
#endif

struct SampleEventState
{
    unsigned char m_reserved0[SAMPLE_MARKER_LIST_OFFSET];
    const MilesMarkerList* m_markerList;
};

struct SelectionRecord
{
    unsigned int tag;
    unsigned int size;
    int isPointer;
    int dataSize;
    unsigned char data[1];
};

struct QueuedEvent
{
    unsigned int committedSize;
    unsigned int size;
    const void* event;
    EventSystem* eventSystem;
    unsigned long long serial;
    unsigned long long context;
    unsigned long long selection;
    unsigned int flags;
    unsigned int dataSize;
    unsigned char data[1];
};

const unsigned int QUEUED_EVENT_OWNS_EVENT = 2;
const unsigned int QUEUED_EVENT_NAMED = 4;
const unsigned int QUEUED_EVENT_AUDITIONED = 8;

struct EventSettings
{
    int magic;
    int limitCount;
    int blendCount;
    int rampCount;
    int lfoCount;
    int variableCount;
    int busCount;
    int curveVariableCount;
    int stepOffsets[1];
};

struct EventSettingsBus
{
    int nameOffset;
    int presetOffset;
    unsigned int flags;
};

struct EventSettingsCurveVariable
{
    int nameOffset;
    int size;
    int data[1];
};

struct EventSettingsVariable
{
    int nameOffset;
    float value;
};

const int EVENT_SETTINGS_MAGIC = 3;

struct SoundInstanceInfo
{
    unsigned long long m_selection;
    unsigned long long m_soundId;
    unsigned long long m_serial;
    _SAMPLE* m_sample;
    _STREAM* m_stream;
    const void* m_selectionData;
    int m_selectionSize;
    unsigned int m_state;
    unsigned int m_flags;
    int m_delay;
    float m_gain;
    float m_pitch;
    const char* m_name;
    int m_active;
};

typedef void (*ExternalSampleReleaseFunc)(_SAMPLE* sample);
typedef int (*ExternalSampleOpenFunc)(const char* name, char* path, _DIG_DRIVER* driver, const void* initBlock,
                                      int initBlockSize);

extern unsigned long long g_soundSerial;
extern unsigned long long g_randomState;
extern EventSystem* g_eventSystemList;
extern void* g_eventMemory;
extern unsigned int (*g_randomOverride)();
extern int g_lastFrameTime;
extern int g_nextAuditionId;
extern ExternalSampleReleaseFunc g_externalSampleRelease;
extern ExternalSampleOpenFunc g_externalSampleOpen;
extern MilesAsyncRead g_soundLoadRequests[SOUND_LOAD_SLOT_COUNT];
extern int g_soundLoadBusy[SOUND_LOAD_SLOT_COUNT];
extern int g_soundLoadCancelled[SOUND_LOAD_SLOT_COUNT];
extern EventSystem* g_soundLoadOwners[SOUND_LOAD_SLOT_COUNT];

int Handle_AllocStringInfo(Handle* handle, int* length, const char* text, const char* file, int line);
unsigned int HorribleRand();
Handle* Var_GetRootFromContext(void* context);
int ExecuteCacheSounds(const EVENT_STEP_INFO* step, EventSystem* eventSystem, unsigned long long eventId, int load);
int StartSingle(const EVENT_STEP_INFO* step, EventSystem* eventSystem, const char* name, int nameLength,
                void* initBlock, unsigned int initBlockSize, unsigned long long serial, unsigned long long selection,
                signed char blendIndex);
EventSystem* EventSystem_Allocate(_DIG_DRIVER* driver);

void Handle_Init(void* memory, int size);
void Handle_Shutdown();
void Handle_Defrag();
void Handle_MemStats(int* used, int* total);

extern "C" void AIL_set_event_settings(const EventSettings* settings);
extern "C" void MilesSetVarIInternal(void* context, const char* name, int value);
extern "C" int MilesEnumerateSoundInstancesInternal(EventSystem* eventSystem, unsigned int* cursor,
                                                    unsigned int typeMask, const char* labels, int eventIndex,
                                                    int eventSerial, SoundInstanceInfo* info);
extern "C" unsigned int MilesStartupEventSystemInternal(_DIG_DRIVER* driver, int queueBytes, int memoryAddress,
                                                        int memoryBytes);
extern "C" int MilesBeginEventQueueProcessingInternal();
extern "C" int MilesCompleteEventQueueProcessingInternal();

#endif
