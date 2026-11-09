#ifndef GAME_CLIENT_AUDIO_FALLOFFGRAPH_HPP
#define GAME_CLIENT_AUDIO_FALLOFFGRAPH_HPP

#include "game/core/Handle.hpp"

class EventSystem;
class SoundInstance;
class SoundSource;
struct QueuedEvent;
struct EVENT_STEP_INFO;
struct SoundLimitEntry;
struct StateVar;

const int QUEUE_BUFFER_COUNT = 8;
const int QUEUE_BUFFER_SIZE = 0x200;

struct QueueBuffer
{
    unsigned char bytes[QUEUE_BUFFER_SIZE];
};

struct _MSSSTRINGC
{
    const char* text;
    int length;
};

struct _SAMPLE;
struct _DIG_DRIVER;
struct _STREAM;
struct _RadSoundSystem;
struct FLT_INSTANCE;

const int SAMPLE_BUFFER_COUNT = 8;
const int SAMPLE_STAGE_COUNT = 10;
const int SAMPLE_FILTER_STAGE_COUNT = 8;
const int SAMPLE_FILTER_HANDLE_COUNT = 8;
const int SAMPLE_STAGE_DECODER = 0;
const int SAMPLE_STAGE_MERGE = 9;
const int SAMPLE_STAGE_ALL = 11;
const int DRIVER_BUS_COUNT = 4;
const int DRIVER_OUTPUT_COUNT = 8;
const int DRIVER_PROCESSOR_COUNT = 6;
const int DRIVER_REVERB_POINTER_COUNT = 6;
const int DRIVER_PROFILE_TIMER_COUNT = 16;
const int DRIVER_PROFILE_SLOT_COUNT = 11;
const int SPEAKER_MAXIMUM_COUNT = 8;
const int SPEAKER_DIRECTION_COUNT = 32;
const int SPEAKER_PLAIN_COUNT = 9;
const int SPEAKER_MEMBER_COUNT = 8;
const int SAMPLE_PATH_POINT_COUNT = 10;
const int SAMPLE_FALLOFF_POINT_COUNT = 5;
const int SAMPLE_FALLOFF_GRAPH_COUNT = 4;

typedef int (*FilterPropertyFunc)(unsigned int property, void* before, const void* newValue, void* after);
typedef int (*FilterStartupFunc)();
typedef const char* (*FilterErrorFunc)();
typedef void (*FilterShutdownFunc)();
typedef void* (*MssAllocFunc)(unsigned int size, unsigned int user, const char* file, int line);
typedef void (*MssFreeFunc)(void* block);
typedef void* (*FilterOpenDriverFunc)(MssAllocFunc allocator, MssFreeFunc deallocator, unsigned int reserved,
                                      _DIG_DRIVER* driver, void* state);
typedef void (*FilterCloseDriverFunc)(void* driverState);
typedef void (*FilterPremixFunc)(void* driverState);
typedef int (*FilterPostmixFunc)(void* driverState, void* buffer);
typedef void* (*FilterOpenSampleFunc)(void* driverState, _SAMPLE* sample, void* state);
typedef void (*FilterCloseSampleFunc)(void* handle);
typedef int (*FilterSampleProcessFunc)(void* handle, void* source, void* destination, unsigned int count, int channel);
typedef int (*FilterSamplePropertyFunc)(void* handle, unsigned int property, void* before, const void* newValue,
                                        void* after);
typedef int (*AsiStreamCallback)(unsigned int user, void* destination, int count, int offset);
typedef void* (*AsiStreamOpenFunc)(MssAllocFunc allocator, MssFreeFunc deallocator, unsigned int user,
                                   AsiStreamCallback callback, unsigned int totalSize);
typedef int (*AsiStreamProcessFunc)(void* stream, void* destination, int size);
typedef void (*AsiStreamSeekFunc)(void* stream, int offset);
typedef void (*AsiStreamCloseFunc)(void* stream);
typedef int (*AsiStreamPropertyFunc)(void* stream, unsigned int property, void* before, const void* newValue,
                                     void* after);
typedef void (*MixerMergeFunc)(void* source, unsigned int* fraction, void* sourceEnd, void** destination,
                               void* destinationEnd, int* left, int* right, unsigned int step, int leftVolume,
                               int rightVolume, unsigned int flags);
typedef void (*DriverFlushFunc)(void* buffer, unsigned int length);
typedef void (*DriverCopyFunc)(void* source, unsigned int length, void* destination, unsigned int flags,
                               int suppressSwap);
typedef void (*DriverMultiChannelCopyFunc)(void* channels, int channelCount, void* destination, unsigned int flags,
                                           unsigned int frames, int stride);
typedef int (*DriverAdpcmDecodeFunc)(void** source, void** destination, int sourceBytes, int destinationBytes,
                                     unsigned int flags, void* state);
typedef void (*DriverFillCallback)(_DIG_DRIVER* driver);
typedef float (*FalloffFunc)(_SAMPLE* sample, float distance, float rolloff, float minimum, float maximum);

struct FLT_INSTANCE
{
    unsigned int flags;
    unsigned int driverStateSize;
    unsigned int sampleStateSize;
    FilterPropertyFunc property;
    FilterStartupFunc startup;
    FilterErrorFunc error;
    FilterShutdownFunc shutdown;
    FilterOpenDriverFunc openDriver;
    FilterCloseDriverFunc closeDriver;
    FilterPremixFunc premixProcess;
    FilterPostmixFunc postmixProcess;
    FilterOpenSampleFunc openSample;
    FilterCloseSampleFunc closeSample;
    FilterSampleProcessFunc sampleProcess;
    FilterSamplePropertyFunc sampleProperty;
    unsigned char reserved0[0x54 - 0x3c];
    _DIG_DRIVER* driver;
    unsigned int provider;
    void* driverState;
    FLT_INSTANCE* next;
};

struct FilterStage
{
    int active;
    unsigned int provider;
    FLT_INSTANCE* instance;
    void* handles[SAMPLE_FILTER_HANDLE_COUNT];
};

struct StageHeader
{
    int active;
    unsigned int provider;
};

struct ASISTAGE
{
    AsiStreamOpenFunc open;
    AsiStreamProcessFunc process;
    AsiStreamSeekFunc seek;
    AsiStreamCloseFunc close;
    AsiStreamPropertyFunc property;
    unsigned int inputBitRate;
    unsigned int inputSampleRate;
    unsigned int inputSampleWidth;
    unsigned int inputChannels;
    unsigned int outputBitRate;
    unsigned int outputSampleRate;
    unsigned int outputSampleWidth;
    unsigned int outputChannels;
    unsigned int outputChannelMask;
    unsigned int outputReservoir;
    unsigned int position;
    unsigned int percentDone;
    unsigned int minimumInputBlockSize;
    unsigned int rawSourceSampleRate;
    unsigned int rawSourceSampleWidth;
    unsigned int rawSourceChannels;
    unsigned int requestedSampleRate;
    unsigned int requestedSampleWidth;
    unsigned int requestedChannels;
    unsigned int streamSeekPosition;
    unsigned int dataOffset;
    unsigned int dataLength;
    unsigned int exactSeek;
    unsigned int exactGetPosition;
    unsigned int seekTableLookup;
    unsigned int setLoopingSamples;
    unsigned int clearLoopMeta;
    void* stream;
};

struct DecoderStage : StageHeader, ASISTAGE
{
};

struct MergeStage
{
    int active;
    unsigned int provider;
    MixerMergeFunc merge;
};

union SampleStage
{
    FilterStage filter;
    DecoderStage decoder;
    MergeStage merge;
};

struct SampleBuffer
{
    const unsigned char* data;
    int size;
    int position;
    int done;
    int reset;
    int resetPosition;
};

struct LOWPASS_INFO
{
    int sampleA;
    int sampleB;
    int historyA;
    int historyB;
    int coefficientA;
    int coefficientB;
    int coefficientC;
    int mode;
};

struct ChannelState
{
    LOWPASS_INFO lowPassInfo;
    int limitB;
    int pendingCoefficients;
    float appliedLowPass;
    float lowPass;
};

struct _MSS_BB
{
    unsigned char* buffer;
    int bytes;
    int chans;
    int speaker;
};

union ProcessorFunction
{
    void* raw;
    DriverFlushFunc flush;
    DriverCopyFunc copy;
    DriverMultiChannelCopyFunc multiChannelCopy;
    DriverAdpcmDecodeFunc adpcmDecode;
    MixerMergeFunc merge;
};

struct DriverProcessor
{
    int active;
    unsigned int provider;
    ProcessorFunction function;
};

struct ReverbState
{
    float* lines[DRIVER_REVERB_POINTER_COUNT];
    float state[4];
    float* starts[DRIVER_REVERB_POINTER_COUNT];
    float* ends[DRIVER_REVERB_POINTER_COUNT];
    float coefficients[DRIVER_REVERB_POINTER_COUNT];
    float lowPass[3];
};

struct DriverBus
{
    int roomType;
    float sendLevel;
    float returnGain;
    ReverbState reverb;
    unsigned char* buffer;
    unsigned int bufferBytes;
    unsigned int chunkBytes;
    int length;
    int update;
    int updateValue;
    int tail;
    float returnLevel;
    float predelay;
    float damping;
    int writeOffset;
    int readOffset;
};

struct MixTimer
{
    unsigned int high;
    unsigned int low;
};

struct SpeakerDirection
{
    float x;
    float y;
    float z;
    int members[SPEAKER_MEMBER_COUNT];
    float gains[SPEAKER_MEMBER_COUNT];
    int memberCount;
};

struct SpeakerState
{
    int active;
    float listenerPosition[3];
    float listenerFace[3];
    float listenerUp[3];
    float listenerCross[3];
    float listenerVelocity[3];
    float rolloffFactor;
    float dopplerFactor;
    float distanceFactor;
    float panExponent;
    int lowFrequencySpeakers[SPEAKER_MAXIMUM_COUNT];
    int lowFrequencyCount;
    int speakerChannels[SPEAKER_PLAIN_COUNT];
    float speakerDirections[SPEAKER_PLAIN_COUNT][3];
    int speakerCount;
    SpeakerDirection directions[SPEAKER_DIRECTION_COUNT];
    int directionCount;
    float positions[SPEAKER_MAXIMUM_COUNT][3];
};

struct _DIG_DRIVER
{
    unsigned int magic;
    int timer;
    int sharedState;
    int fillPeriod;
    float masterVolume;
    int outputRate;
    unsigned int flags;
    int activeSamples;
    unsigned int requestedSpeakerConfig;
    SpeakerState speakers;
    float reverbSendA[SPEAKER_MAXIMUM_COUNT];
    float reverbSendB[SPEAKER_MAXIMUM_COUNT];
    _RadSoundSystem* output;
    void* outputMemory;
    unsigned char reserved3[0xbdc - 0xbd8];
    int started;
    int bufferCount;
    int framesPerBuffer;
    int hardwareChannels;
    int speakerConfig;
    _SAMPLE* samples;
    int* sampleStatus;
    int sampleCount;
    unsigned char reserved1[0xc1c - 0xbfc];
    _SAMPLE* busSamples[DRIVER_BUS_COUNT];
    int busActive[DRIVER_BUS_COUNT];
    unsigned char* busBuffers[DRIVER_BUS_COUNT];
    unsigned char reserved5[0xc5c - 0xc4c];
    unsigned char phase;
    unsigned char reserved9[3];
    _MSS_BB outputs[DRIVER_OUTPUT_COUNT];
    _MSS_BB mix;
    int outputCount;
    int fillBytes;
    int limiterEnabled;
    int limiterLevel;
    int pendingStarts;
    DriverFillCallback fillCallback;
    unsigned int fillTimeHigh;
    unsigned int fillTimeLow;
    unsigned char reserved7[0xd1c - 0xd10];
    unsigned char* interleaveBuffer;
    DriverProcessor processors[DRIVER_PROCESSOR_COUNT];
    FLT_INSTANCE* matrixFilter;
    DriverBus buses[DRIVER_BUS_COUNT];
    _DIG_DRIVER* next;
    MixTimer timers[DRIVER_PROFILE_TIMER_COUNT];
    _SAMPLE* profileSamples[DRIVER_PROFILE_SLOT_COUNT];
    int profileCounts[DRIVER_PROFILE_SLOT_COUNT];
};

struct _LISTENER
{
    unsigned char reserved0[0x28];
    float x;
    float y;
    float z;
};

typedef void (*AIL_SAMPLE_CALLBACK)(_SAMPLE* sample);

struct FalloffGraphPoint
{
    float values[8];
};

struct _SAMPLE
{
    unsigned int magic;
    _DIG_DRIVER* driver;
    int index;
    SampleBuffer buffers[SAMPLE_BUFFER_COUNT];
    unsigned int resampleFraction;
    int playbackDelay;
    float volume;
    unsigned int playedHigh;
    unsigned int playedLow;
    unsigned char reserved4[4];
    unsigned int channelBufferCapacity;
    unsigned int channelBufferSize;
    unsigned char* channelBuffer;
    int* channelPositionA;
    int* channelPositionB;
    int* channelHeader;
    ChannelState* channelStates;
    float** levels;
    int** levelsB;
    int** levelsC;
    int** levelsD;
    float* speakerLevelsA;
    float* speakerScales;
    signed char* channelMap;
    int lowPassActive;
    int userLevels;
    int bufferCount;
    int lastBuffer;
    int currentBuffer;
    int starved;
    int bufferRewind;
    int loopState;
    int loopStart;
    unsigned int loopEnd;
    int loopCount;
    unsigned int loopStartRequest;
    unsigned int loopEndRequest;
    unsigned int channelFlags;
    int channelCount;
    int channelMask;
    int playbackRate;
    float playbackRateFactor;
    float volumeCurve;
    float pan;
    float panLevelA;
    float panLevelB;
    float reverbSecond;
    float reverbFirst;
    float levelD;
    float levelA;
    float levelB;
    float rate;
    float levelC;
    int cacheState;
    int reverbBus;
    unsigned int compressorSources;
    unsigned int compressorLink;
    unsigned int busPeak;
    int reverbDirty;
    unsigned int peak;
    int limiterEnabled;
    int limiterLevel;
    int endMode;
    unsigned int startTimeHigh;
    unsigned int startTimeLow;
    short fadeSamples;
    short fadePosition;
    char fadeEnds;
    unsigned char phaseMask;
    bool outputToDRC;
    unsigned char reserved7a[0x1c8 - 0x1c7];
    float obstruction;
    float occlusion;
    float exclusion;
    int loopMode;
    AIL_SAMPLE_CALLBACK startOfBuffer;
    AIL_SAMPLE_CALLBACK endOfBuffer;
    AIL_SAMPLE_CALLBACK endOfSample;
    unsigned char reserved1[0x208 - 0x1e4];
    Handle falloffHandle;
    unsigned int vocRepeatCount;
    int vocMarker;
    int vocMarkerFound;
    _STREAM* stream;
    unsigned char reserved2[0x228 - 0x220];
#if __SIZEOF_POINTER__ == 8
    const void* pcMarkerList;
#endif
    int adpcmBlockSize;
    int resetA;
    int resetB;
    unsigned char reserved15[0x270 - 0x234];
    int endCounter;
    int startCounter;
    int sampleEndCounter;
    int rampLength;
    int resampleTolerance;
    int resampleFilter;
    SampleStage stages[SAMPLE_STAGE_COUNT];
    int filterCount;
    int is3D;
    float x;
    float y;
    float z;
    float forwardX;
    float forwardY;
    float forwardZ;
    float upX;
    float upY;
    float upZ;
    float velocityX;
    float velocityY;
    float velocityZ;
    int dopplerActive;
    float dopplerScale;
    float coneInner;
    float coneOuter;
    float coneOuterVolume;
    int coneEnabled;
    float maximumDistance;
    float minimumDistance;
    int distancesChanged;
    int automaticWetAttenuation;
    float distanceVolume;
    float presetValue;
    float falloffVolume;
    float falloffLowPass;
    float falloffExclusion;
    _SAMPLE* falloffUser;
    FalloffFunc falloffFunction;
    float pathPoints[SAMPLE_PATH_POINT_COUNT][3];
    int pathPointCount;
    FalloffGraphPoint falloffPoints[SAMPLE_FALLOFF_GRAPH_COUNT][SAMPLE_FALLOFF_POINT_COUNT];
    char falloffPointCounts[4];
    float mixLevel0;
    float mixLevel1;
    float mixLevel2;
    float mixLevel3;
    float mixInput0;
    float mixInput1;
    float mixInput2;
};

struct FalloffGraph
{
    int reserved;
    float maximumDistance;
    int volumePointCount;
    int exclusionPointCount;
    int lowpassPointCount;
    FalloffGraphPoint points[1];
};

void FalloffGraph_Set(_SAMPLE* sample, int flags, FalloffGraph* graph);

int StateVar_KeyLB(StateVar* vars, int count, void* key);
int StateVar_Lookup(EventSystem* eventSystem, void* key);
void StateVar_Set(EventSystem* eventSystem, void* key, int value);
Handle Sound_Find(EventSystem* eventSystem, const char* name);

int DoLabelsMatchQuery(const char* labels, const char* query);
SoundLimitEntry* IsLabelLimited(EventSystem* eventSystem, _MSSSTRINGC label);
int Blend_Exists(EventSystem* eventSystem, const char* name);
int Blend_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step);
void Blend_Apply(SoundInstance* instance, float* volume, float* pitch);
void DestroySoundInstance(EventSystem* eventSystem, SoundInstance* instance);
int EvictSound(EventSystem* eventSystem, unsigned long long eventId, unsigned char policy, int priority,
               SoundLimitEntry* limit, SoundSource* source, const char* name);

void Queue_SetMem(int capacity, Handle storage);
unsigned long long Queue_EnumCreate();
QueuedEvent* Queue_EnumNext(unsigned long long* cursor, QueuedEvent* event);
void Queue_EnumClose(unsigned long long start, unsigned long long end);
QueuedEvent* Queue_Add(int size);
void Queue_Commit(QueuedEvent* event);
void ClearQueueToken(int token);
extern "C" unsigned long long MilesEnqueueEventByNameInternal(const char* pEventName);

extern "C" unsigned int AIL_enqueue_event_start();
extern "C" unsigned int AIL_enqueue_event_buffer(unsigned int* puiQueue, void* pData, int iDataSize, int iFlags);
extern "C" void AIL_enqueue_event_end_named(unsigned int uiQueue, const char* pEventName);
unsigned long long AIL_enqueue_event_end(int queue, void* event, int named, int ownsEvent);
unsigned long long AIL_enqueue_event_from_init_block(const char* name, void* block, int size,
                                                     unsigned long long selection);

#endif
