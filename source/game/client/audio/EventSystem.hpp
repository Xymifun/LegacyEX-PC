#ifndef GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP
#define GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP

#include "game/core/Handle.hpp"
#include "vendor/MilesMarker.hpp"
#include "game/core/PODHandleArray.hpp"

struct _SAMPLE;
struct _STREAM;
struct _LISTENER;
struct _DIG_DRIVER;

const unsigned int EVENT_SYSTEM_MAGIC = 0x45535953;

struct StateVar
{
    void* key;
    int value;
};

class EventSystem
{
public:
    unsigned int m_magic;
    EventSystem* m_next;
    PODHandleArray<Handle> m_sounds;
    PODHandleArray<Handle> m_instances;
    PODHandleArray<StateVar> m_stateVars;
    Handle m_ramps;
    Handle m_variables;
    Handle m_blends;
    Handle m_persists;
    Handle m_limits;
    Handle m_lfos;
    Handle m_tweens;
    Handle m_buses;
    union
    {
        _LISTENER* m_listener;
        _DIG_DRIVER* m_driver;
    };
    Handle m_environment;
    int m_environmentActive;
    int m_soundBytes;
};

class SoundInstance;

const int SOUND_STATE_PENDING = 1;
const int SOUND_STATE_PLAYING = 2;
const int SOUND_STATE_FINISHED = 4;

const unsigned char SOUND_INSTANCE_FLAG_UNNAMED = 1;
const unsigned char SOUND_INSTANCE_FLAG_EVICTED = 2;
const unsigned char SOUND_INSTANCE_FLAG_WAITING_FOR_DATA = 4;
const unsigned char SOUND_INSTANCE_FLAG_WAITING_FOR_STREAM = 8;
const unsigned char SOUND_INSTANCE_FLAG_SILENT = 0x10;
const unsigned char SOUND_INSTANCE_FLAG_FAILED = 0x20;

struct SoundInstanceData
{
    unsigned long long m_serial;
    unsigned long long m_selection;
    unsigned long long m_eventId;
    _SAMPLE* m_sample;
    _STREAM* m_stream;
    EventSystem* m_eventSystem;
    Handle m_self;
    Handle m_source;
    Handle m_name;
    Handle m_labels;
    Handle m_ramps;
    Handle m_context;
    Handle m_blend;
    Handle m_preset;
    Handle m_initBlock;
    int m_initBlockSize;
    int m_selectionOffset;
    int m_nameLength;
    int m_state;
    int m_startTime;
    int m_startPosition;
    int m_startPositionType;
    int m_loopStart;
    int m_loopEnd;
    float m_pitch;
    float m_gain;
    int m_delay;
    float m_volume;
    float m_fadeRate;
    float m_delayRemaining;
    unsigned short m_markerFlags;
    unsigned char m_priority;
    unsigned char m_flags;
    signed char m_blendIndex;
    unsigned char m_externalSample;
    unsigned char m_paused;
    unsigned char m_reserved0;
};

const int MAX_BLEND_POINTS = 10;

struct BlendStep
{
    const char* m_name;
    int m_nameLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    float m_rangeStart[MAX_BLEND_POINTS];
    float m_fadeInEnd[MAX_BLEND_POINTS];
    float m_fadeOutStart[MAX_BLEND_POINTS];
    float m_rangeEnd[MAX_BLEND_POINTS];
    float m_pitchAtStart[MAX_BLEND_POINTS];
    float m_pitchAtEnd[MAX_BLEND_POINTS];
    unsigned char m_pointCount;
};

struct RampStep
{
    const char* m_name;
    int m_nameLength;
    const char* m_labels;
    int m_labelsLength;
    const char* m_value;
    int m_valueLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    float m_duration;
    unsigned char m_target;
    unsigned char m_global;
    unsigned char m_curve;
};

struct LfoStep
{
    const char* m_name;
    int m_nameLength;
    const char* m_center;
    int m_centerLength;
    const char* m_amplitude;
    int m_amplitudeLength;
    const char* m_frequency;
    int m_frequencyLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    int m_inverted;
    int m_flip;
    int m_mode;
    int m_offset;
};

struct PersistStep
{
    const char* m_name;
    int m_nameLength;
    const char* m_preset;
    int m_presetLength;
    const char* m_labels;
    int m_labelsLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    unsigned char m_active;
};

struct LimitStep
{
    const char* m_limits;
    int m_limitsLength;
    const char* m_name;
    int m_nameLength;
};

struct PlayStep
{
    const char* m_sounds;
    int m_soundsLength;
    const char* m_preset;
    int m_presetLength;
    const char* m_completionEvent;
    int m_completionEventLength;
    const char* m_labels;
    int m_labelsLength;
    const char* m_loopStartMarker;
    int m_loopStartMarkerLength;
    const char* m_loopEndMarker;
    int m_loopEndMarkerLength;
    const char* m_positionMarker;
    int m_positionMarkerLength;
    const char* m_selector;
    int m_selectorLength;
    const char* m_variables;
    int m_variablesLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    int m_stream;
    float m_volumeMinimum;
    float m_volumeMaximum;
    float m_pitchMinimum;
    float m_pitchMaximum;
    float m_fadeTime;
    unsigned short m_delayMinimum;
    unsigned short m_delayMaximum;
    unsigned char m_loadOnDemand;
    unsigned char m_priority;
    signed char m_loopCount;
    unsigned char m_limitPolicy;
    unsigned char m_modeFlags;
    unsigned char m_presetPersistent;
};

struct ControlStep
{
    const char* m_labels;
    int m_labelsLength;
    const char* m_loopStartMarker;
    int m_loopStartMarkerLength;
    const char* m_loopEndMarker;
    int m_loopEndMarkerLength;
    const char* m_positionMarker;
    int m_positionMarkerLength;
    const char* m_preset;
    int m_presetLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    float m_fadeTime;
    unsigned char m_presetPersistent;
    signed char m_loopCount;
    unsigned char m_action;
};

struct EnvironmentStep
{
    const char* m_name;
    int m_nameLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    unsigned char m_enabled;
};

struct CacheStep
{
    const char* m_bank;
    int m_bankLength;
    const char** m_names;
    int m_nameCount;
};

struct NameStep
{
    const char* m_name;
    int m_nameLength;
};

struct TweenStep
{
    const char* m_name;
    int m_nameLength;
#if __SIZEOF_POINTER__ == 8
    int m_pcStringPadding;
#endif
    float m_firstTime;
    float m_secondTime;
    int m_firstCurve;
    int m_secondCurve;
    float m_start;
    float m_middle;
    float m_end;
};

struct EVENT_STEP_INFO
{
    unsigned int m_kind;
    union
    {
        PlayStep m_play;
        ControlStep m_control;
        EnvironmentStep m_environment;
        CacheStep m_cache;
        NameStep m_name;
        BlendStep m_blend;
        RampStep m_ramp;
        LfoStep m_lfo;
        PersistStep m_persist;
        LimitStep m_limit;
        TweenStep m_tween;
    } m_data;
};

struct Ramp;

int Var_LookupWithContext(EventSystem* eventSystem, Handle handle, const char* name, int flags, void* value);

int FindMarker(EventSystem* eventSystem, SoundInstance* soundInstance, const MilesMarkerList* markerList,
               const char* markerName, int* markerIndex, int* markerType);

Handle Sound_Load(EventSystem* eventSystem, unsigned long long eventId, const char* name);
void Sound_Free(EventSystem* eventSystem, Handle sound);
int Persist_Create(EventSystem* eventSystem, unsigned long long eventId, const EVENT_STEP_INFO* step);
void Persist_Free(EventSystem* eventSystem, unsigned long long eventId, const char* name);
void Persist_Apply(EventSystem* eventSystem, SoundInstance* soundInstance, _SAMPLE* sample, const char* labels,
                   int unusedArgument, int onlyActive);
int Limit_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step, long long eventId);
int LFO_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step);
int Ramp_Add(Handle* ramps, EventSystem* eventSystem, const EVENT_STEP_INFO* step);
float Ramp_GetCurrentValue(EventSystem* eventSystem, Ramp* ramp, int applyCurve);
void Ramp_ApplyAll(EventSystem* eventSystem, SoundInstance* soundInstance, float elapsed);
void Var_SetInstance(Handle* variables, const char* name, void* value, int isFloat);

#endif
