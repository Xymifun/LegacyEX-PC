#include "vendor/MilesEventExec.hpp"

#include "game/client/audio/FalloffGraph.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
#include "game/util/StringUtils.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesBank.hpp"
#include "vendor/MilesEventError.hpp"
#include "vendor/MilesIO.hpp"
#include "vendor/MilesMem.hpp"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

extern "C" unsigned int AIL_us_count();
extern "C" const unsigned char* AIL_next_event_step(const unsigned char* data, EVENT_STEP_INFO** step, void* buffer,
                                                    unsigned int bufferSize);


namespace
{
const unsigned int RANDOM_MULTIPLIER = 0x19660d;
const unsigned int RANDOM_INCREMENT = 0x3c6ef35f;
const int RANDOM_SHIFT = 12;
const int DEFAULT_EVENT_MEMORY = 0x10000;
const int DEFAULT_QUEUE_BYTES = 0x1400;
const int EVENT_SYSTEM_ALLOC_LINE = 0x6b1;
const int EVENT_MEMORY_ALLOC_LINE = 0x710;
const int QUEUE_ALLOC_LINE = 0x726;
const int SETTINGS_BUS_NODE_LINE = 0x8e;
const int SETTINGS_BUS_PRESET_LINE = 0x94;
const int SETTINGS_CURVE_VARIABLE_LINE = 0xcd;
const int SETTINGS_CURVE_DATA_LINE = 0xce;
const int CACHE_ALLOC_LINE = 0xcbf;
const int BUS_NODE_HEADER_SIZE = 0x21;
const int CURVE_VARIABLE_HEADER_SIZE = 0x24;
const int ENUMERATE_FIRST_INDEX_MASK = 0xffff;
const int ENUMERATE_SYSTEM_SHIFT = 16;
const int SOUND_ENTRY_PATH_SIZE = 512;
const int ASYNC_READ_WHOLE_FILE = -1;
const unsigned int BUS_FLAG_LIMITER = 2;
const unsigned int BUS_FLAG_PRESET = 1;
const int BUS_MAIN = 0;
const int BUS_FIRST_AUXILIARY = 1;
const int SAMPLE_BUS_COUNT_START = 0;
const char EVENT_EXEC_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mileseventexec.cpp";
const char ADDITIONAL_SYSTEMS_ERROR[] = "Additional event systems are addded via AIL_add_event_system.";
const char OUT_OF_MEMORY_ERROR[] = "OOM allocating event system memory";
const char COMMAND_HANDLE_ERROR[] = "Failed to set handle for command buffer.";
const char DRC_BUS_PREFIX[] = "drc";
const int DRC_BUS_PREFIX_LENGTH = 3;
const char QUERY_ALL_LABELS[] = "";
}

unsigned long long g_soundSerial = 0;
unsigned long long g_randomState = 0;
EventSystem* g_eventSystemList = NULL;
void* g_eventMemory = NULL;
unsigned int (*g_randomOverride)() = NULL;
int g_lastFrameTime = 0;
ExternalSampleReleaseFunc g_externalSampleRelease = NULL;
ExternalSampleOpenFunc g_externalSampleOpen = NULL;

int Handle_AllocStringInfo(Handle* handle, int* length, const char* text, const char* file, int line)
{
    const size_t size = strlen(text);
    const Handle allocated = Handle_AllocateInfo(size + 1, file, line);
    *handle = allocated;
    char* destination = static_cast<char*>(Handle_Get(allocated));
    if (destination == NULL)
    {
        return 0;
    }

    strcpy(destination, text);
    if (length != NULL)
    {
        *length = size;
    }

    return 1;
}

unsigned int HorribleRand()
{
    if (g_randomOverride != NULL)
    {
        return g_randomOverride();
    }

    g_randomState = static_cast<unsigned int>(g_randomState) * RANDOM_MULTIPLIER + RANDOM_INCREMENT;
    return static_cast<unsigned int>(g_randomState) >> RANDOM_SHIFT;
}

Handle* Var_GetRootFromContext(void* context)
{
    EventSystem* eventSystem = g_eventSystemList;
    if (context == NULL)
    {
        Handle* root = &eventSystem->m_variables;
        if (root != NULL)
        {
            return root;
        }
    }
    else
    {
        for (EventSystem* candidate = eventSystem; candidate != NULL; candidate = candidate->m_next)
        {
            if (context == candidate)
            {
                Handle* root = &candidate->m_variables;
                if (root != NULL)
                {
                    return root;
                }

                break;
            }
        }
    }

    unsigned int token = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context));
    unsigned int systemIndex = token >> ENUMERATE_SYSTEM_SHIFT;
    while (eventSystem != NULL && systemIndex != 0)
    {
        eventSystem = eventSystem->m_next;
        --systemIndex;
    }

    if (eventSystem == NULL)
    {
        return NULL;
    }

    const int instanceIndex = token & ENUMERATE_FIRST_INDEX_MASK;
    if (instanceIndex > eventSystem->m_instances.Count())
    {
        return NULL;
    }

    SoundInstanceData* instance = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[instanceIndex]));
    return &instance->m_context;
}

extern "C" void MilesSetVarIInternal(void* context, const char* name, int value)
{
    Handle* root = Var_GetRootFromContext(context);
    if (root != NULL)
    {
        Var_SetInstance(root, name, &value, 0);
    }
}

EventSystem* EventSystem_Allocate(_DIG_DRIVER* driver)
{
    EventSystem* eventSystem = static_cast<EventSystem*>(
        AIL_mem_alloc_lock_info(sizeof(EventSystem), EVENT_EXEC_FILE, EVENT_SYSTEM_ALLOC_LINE));
    memset(eventSystem, 0, sizeof(EventSystem));
    eventSystem->m_driver = driver;
    eventSystem->m_magic = EVENT_SYSTEM_MAGIC;
    if (g_eventSystemList != NULL)
    {
        eventSystem->m_next = g_eventSystemList->m_next;
        g_eventSystemList->m_next = eventSystem;
    }
    else
    {
        eventSystem->m_next = NULL;
        g_eventSystemList = eventSystem;
    }

    return eventSystem;
}

extern "C" unsigned int MilesStartupEventSystemInternal(_DIG_DRIVER* driver, int queueBytes, int memoryAddress,
                                                        int memoryBytes)
{
    void* memory = reinterpret_cast<void*>(memoryAddress);
    if (g_eventSystemList != NULL)
    {
        AIL_set_error(ADDITIONAL_SYSTEMS_ERROR);
        return 0;
    }

    g_randomState = AIL_us_count();
    if (memory == NULL)
    {
        if (memoryBytes == 0)
        {
            memoryBytes = DEFAULT_EVENT_MEMORY;
        }

        g_eventMemory = AIL_mem_alloc_lock_info(memoryBytes, EVENT_EXEC_FILE, EVENT_MEMORY_ALLOC_LINE);
        if (g_eventMemory == NULL)
        {
            AIL_set_error(OUT_OF_MEMORY_ERROR);
            return 0;
        }

        Handle_Init(g_eventMemory, memoryBytes);
    }
    else
    {
        Handle_Init(memory, memoryBytes);
    }

    if (queueBytes == 0)
    {
        queueBytes = DEFAULT_QUEUE_BYTES;
    }

    if (memoryBytes <= queueBytes)
    {
        queueBytes = memoryBytes / 2;
    }

    const Handle queue = Handle_AllocateInfo(queueBytes, EVENT_EXEC_FILE, QUEUE_ALLOC_LINE);
    if (Handle_Get(queue) == NULL)
    {
        Handle_Shutdown();
        if (g_eventMemory != NULL)
        {
            AIL_mem_free_lock(g_eventMemory);
            g_eventMemory = NULL;
        }

        AIL_set_error(COMMAND_HANDLE_ERROR);
        return 0;
    }

    Queue_SetMem(queueBytes, queue);
    memset(g_soundLoadRequests, 0, sizeof(g_soundLoadRequests));
    memset(g_soundLoadBusy, 0, sizeof(g_soundLoadBusy));
    memset(g_soundLoadCancelled, 0, sizeof(g_soundLoadCancelled));
    memset(g_soundLoadOwners, 0, sizeof(g_soundLoadOwners));
    EventSystem* eventSystem = EventSystem_Allocate(driver);
    if (eventSystem == NULL)
    {
        Handle_Shutdown();
        if (g_eventMemory != NULL)
        {
            AIL_mem_free_lock(g_eventMemory);
            g_eventMemory = NULL;
        }

        return 0;
    }

    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(eventSystem));
}

extern "C" int MilesEnumerateSoundInstancesInternal(EventSystem* eventSystem, unsigned int* cursor,
                                                    unsigned int typeMask, const char* labels, int eventIndex,
                                                    int eventSerial, SoundInstanceInfo* info)
{
    if (eventSystem == NULL)
    {
        eventSystem = g_eventSystemList;
    }

    int systemIndex = 0;
    for (EventSystem* candidate = g_eventSystemList; eventSystem != candidate; candidate = candidate->m_next)
    {
        ++systemIndex;
    }

    if (labels == NULL)
    {
        labels = QUERY_ALL_LABELS;
    }

    if (typeMask == 0)
    {
        typeMask = 0xffffffff;
    }

    unsigned int index = 0;
    if (cursor != NULL)
    {
        index = (*cursor + 1) & ENUMERATE_FIRST_INDEX_MASK;
    }

    SoundInstanceData* instance;
    for (;;)
    {
        for (;;)
        {
            if (static_cast<int>(index) >= eventSystem->m_instances.Count())
            {
                return 0;
            }

            instance = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
            const unsigned int serialHigh = instance->m_serial >> 32;
            const unsigned int serialLow = static_cast<unsigned int>(instance->m_serial);
            const unsigned int soundHigh = instance->m_eventId >> 32;
            const unsigned int soundLow = static_cast<unsigned int>(instance->m_eventId);
            const unsigned int selectionHigh = instance->m_selection >> 32;
            const unsigned int selectionLow = static_cast<unsigned int>(instance->m_selection);
            if ((static_cast<unsigned int>(instance->m_state) & typeMask) != 0
                && ((eventIndex == 0 && eventSerial == 0)
                    || (serialHigh == static_cast<unsigned int>(eventIndex)
                        && serialLow == static_cast<unsigned int>(eventSerial))
                    || (soundHigh == static_cast<unsigned int>(eventIndex)
                        && soundLow == static_cast<unsigned int>(eventSerial))
                    || (selectionHigh == static_cast<unsigned int>(eventIndex)
                        && selectionLow == static_cast<unsigned int>(eventSerial))))
            {
                break;
            }

            ++index;
        }

        if (DoLabelsMatchQuery(static_cast<const char*>(Handle_Get(instance->m_labels)), labels) != 0)
        {
            break;
        }

        ++index;
    }

    info->m_selection = instance->m_selection;
    info->m_serial = instance->m_serial;
    info->m_soundId = instance->m_eventId;
    info->m_sample = instance->m_sample;
    info->m_state = instance->m_state;
    info->m_stream = instance->m_stream;
    info->m_flags = instance->m_flags;
    info->m_delay = instance->m_delay;
    info->m_pitch = instance->m_pitch;
    info->m_gain = instance->m_gain;
    info->m_active = Handle_Get(instance->m_self) != NULL;
    info->m_name = static_cast<const char*>(Handle_Get(instance->m_name));
    info->m_selectionData = NULL;
    info->m_selectionSize = 0;
    if (instance->m_selectionOffset != -1)
    {
        const SelectionRecord* record = reinterpret_cast<const SelectionRecord*>(
            static_cast<const unsigned char*>(Handle_Get(instance->m_initBlock)) + instance->m_selectionOffset);
        info->m_selectionData = record->data;
        info->m_selectionSize = record->dataSize;
        if (record->isPointer != 0)
        {
            info->m_selectionData = *reinterpret_cast<void* const*>(record->data);
        }
    }

    *cursor = (systemIndex << ENUMERATE_SYSTEM_SHIFT) | index;
    return 1;
}

int ExecuteCacheSounds(const EVENT_STEP_INFO* step, EventSystem* eventSystem, unsigned long long eventId, int load)
{
    const CacheStep& cache = step->m_data.m_cache;
    char path[SOUND_ENTRY_PATH_SIZE];
    char filePath[SOUND_ENTRY_PATH_SIZE];
    _MILESBANKSOUNDINFO info;

    strcpy(path, cache.m_bank);
    path[cache.m_bankLength] = '/';
    char* nameStart = path + cache.m_bankLength + 1;
    if (load == 0)
    {
        for (int i = 0; i < cache.m_nameCount; ++i)
        {
            strcpy(nameStart, cache.m_names[i]);
            const Handle sound = Sound_Find(eventSystem, path);
            if (Handle_Get(sound) != NULL)
            {
                Sound_Free(eventSystem, sound);
            }
        }

        return 1;
    }

    for (int i = 0; i < cache.m_nameCount; ++i)
    {
        strcpy(nameStart, cache.m_names[i]);
        const Handle sound = Sound_Find(eventSystem, path);
        LoadedSound* entry = static_cast<LoadedSound*>(Handle_Get(sound));
        if (entry == NULL)
        {
            Sound_Load(eventSystem, eventId, path);
            continue;
        }

        ++entry->m_refCount;
        if (entry->m_loadFailed == 0 || entry->m_loadSlot != 0)
        {
            continue;
        }

        if (Container_GetSound(path, filePath, &info) == 0)
        {
            ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, eventId, path);
            continue;
        }

        AIL_mem_free_lock(entry->m_data);
        entry->m_data = NULL;
        entry->m_loadFailed = 0;
        entry->m_loadSlot = 0;
        entry->m_auditionId = g_nextAuditionId++;
        Audition_AsyncQueued(eventId, entry->m_auditionId, path);
        for (int slot = 0; slot < SOUND_LOAD_SLOT_COUNT; ++slot)
        {
            if (g_soundLoadBusy[slot] != 0)
            {
                continue;
            }

            MilesAsyncRead* request = &g_soundLoadRequests[slot];
            memset(request, 0, sizeof(MilesAsyncRead));
            strcpy(request->szFileName, filePath);
            Audition_AsyncLoad(entry->m_auditionId, info.m_dataSize);
            if (info.m_unbuffered == 0)
            {
                entry->m_data = AIL_mem_alloc_lock_info(info.m_dataSize, EVENT_EXEC_FILE, CACHE_ALLOC_LINE);
                if (entry->m_data == NULL)
                {
                    ErrorHandler(ERROR_TYPE_OUT_OF_MEMORY, eventId, path);
                    entry->m_loadFailed = 1;
                    break;
                }

                entry->m_size = info.m_dataSize;
                request->iLength = info.m_dataSize;
                request->iLengthHigh = info.m_dataSize >> 31;
                request->pBuffer = entry->m_data;
                request->pPriorityUserData = reinterpret_cast<void*>(entry->m_auditionId);
            }
            else
            {
                request->iLength = ASYNC_READ_WHOLE_FILE;
                request->iLengthHigh = ASYNC_READ_WHOLE_FILE;
                request->pPriorityUserData = reinterpret_cast<void*>(entry->m_auditionId);
            }

            MilesAsyncFileRead(request);
            g_soundLoadOwners[slot] = eventSystem;
            g_soundLoadBusy[slot] = 1;
            entry->m_loadSlot = slot + 1;
            break;
        }

        const int count = eventSystem->m_sounds.Count();
        for (int index = 0; index < count - 1; ++index)
        {
            LoadedSound* other = static_cast<LoadedSound*>(Handle_Get(eventSystem->m_sounds[index]));
            if (other->m_loadSlot == 0)
            {
                const Handle swapped = eventSystem->m_sounds[index];
                eventSystem->m_sounds[index] = eventSystem->m_sounds.Top();
                eventSystem->m_sounds.Top() = swapped;
                break;
            }
        }
    }

    return 1;
}

extern "C" void AIL_set_event_settings(const EventSettings* settings)
{
    if (settings->magic != EVENT_SETTINGS_MAGIC)
    {
        return;
    }

    const unsigned char* base = reinterpret_cast<const unsigned char*>(settings);
    int limitCount = settings->limitCount;
    int blendCount = settings->blendCount;
    int rampCount = settings->rampCount;
    int lfoCount = settings->lfoCount;
    const int stepCount = limitCount + blendCount + rampCount + lfoCount;
    const int* cursor = settings->stepOffsets;

    for (int i = 0; i < stepCount; ++i)
    {
        EVENT_STEP_INFO* step = NULL;
        char buffer[EVENT_QUEUE_SCRATCH_SIZE];
        AIL_next_event_step(base + *cursor, &step, buffer, EVENT_QUEUE_SCRATCH_SIZE);
        ++cursor;
        if (step == NULL)
        {
            return;
        }

        if (limitCount != 0)
        {
            if (step->m_kind != EVENT_STEP_LIMIT)
            {
                return;
            }

            Limit_Add(g_eventSystemList, step, 0);
            --limitCount;
        }
        else if (blendCount != 0)
        {
            if (step->m_kind != EVENT_STEP_BLEND)
            {
                return;
            }

            Blend_Add(g_eventSystemList, step);
            --blendCount;
        }
        else if (rampCount != 0)
        {
            if (step->m_kind != EVENT_STEP_RAMP)
            {
                return;
            }

            Ramp_Add(&g_eventSystemList->m_ramps, g_eventSystemList, step);
            --rampCount;
        }
        else if (lfoCount != 0)
        {
            if (step->m_kind != EVENT_STEP_LFO)
            {
                return;
            }

            LFO_Add(g_eventSystemList, step);
            --lfoCount;
        }
    }

    Handle previousBus;
    unsigned int firstBusFlags = 0;
    const EventSettingsBus* bus = reinterpret_cast<const EventSettingsBus*>(cursor);
    for (int busIndex = 0; busIndex < settings->busCount; ++busIndex, ++bus)
    {
        const char* name = reinterpret_cast<const char*>(base + bus->nameOffset);
        const char* presetName = reinterpret_cast<const char*>(base + bus->presetOffset);
        const unsigned int flags = bus->flags;

        const Handle nodeHandle = Handle_AllocateInfo(strlen(name) + BUS_NODE_HEADER_SIZE, EVENT_EXEC_FILE,
                                                      SETTINGS_BUS_NODE_LINE);
        BusNode* node = static_cast<BusNode*>(Handle_Get(nodeHandle));
        if (node == NULL)
        {
            continue;
        }

        const Handle presetHandle = Handle_AllocateInfo(strlen(presetName) + 1, EVENT_EXEC_FILE,
                                                        SETTINGS_BUS_PRESET_LINE);
        char* presetText = static_cast<char*>(Handle_Get(presetHandle));
        if (presetText == NULL)
        {
            Handle_Free(nodeHandle);
            continue;
        }

        if (busIndex == 0)
        {
            strcpy(node->m_name, name);
            strcpy(presetText, presetName);
            node->m_preset = presetHandle;
            node->m_index = 0;
            node->m_self = nodeHandle;
            node->m_presetEnabled = flags & BUS_FLAG_PRESET;
            Handle* previous = static_cast<Handle*>(Handle_Get(previousBus));
            if (previous != NULL)
            {
                *previous = nodeHandle;
            }

            firstBusFlags = flags;
            g_eventSystemList->m_buses = node->m_self;
        }
        else
        {
            node->m_sample = AIL_allocate_bus(g_eventSystemList->m_driver);
            node->m_sample->limiterEnabled = flags & BUS_FLAG_LIMITER;
            strcpy(node->m_name, name);
            strcpy(presetText, presetName);
            node->m_preset = presetHandle;
            node->m_index = busIndex;
            node->m_self = nodeHandle;
            node->m_presetEnabled = flags & BUS_FLAG_PRESET;
            BusNode* previous = static_cast<BusNode*>(Handle_Get(previousBus));
            if (previous != NULL)
            {
                previous->m_next = nodeHandle;
            }

            if (busIndex == 1)
            {
                previous->m_sample = AIL_bus_sample_handle(g_eventSystemList->m_driver, 0);
                previous->m_sample->limiterEnabled = firstBusFlags & BUS_FLAG_LIMITER;
            }
        }

        if (AIL_strnicmp(node->m_name, DRC_BUS_PREFIX, DRC_BUS_PREFIX_LENGTH) == 0
            && (node->m_name[3] == '\0' || node->m_name[3] == ','))
        {
            node->m_sample->outputToDRC = true;
        }

        previousBus = nodeHandle;
    }

    cursor = reinterpret_cast<const int*>(bus);
    for (int curveIndex = 0; curveIndex < settings->curveVariableCount; ++curveIndex)
    {
        const char* name = reinterpret_cast<const char*>(base + cursor[0]);
        const int size = cursor[1];
        cursor += 2;
        const Handle variableHandle = Handle_AllocateInfo(strlen(name) + CURVE_VARIABLE_HEADER_SIZE, EVENT_EXEC_FILE,
                                                          SETTINGS_CURVE_VARIABLE_LINE);
        const Handle curveHandle = Handle_AllocateInfo(size, EVENT_EXEC_FILE, SETTINGS_CURVE_DATA_LINE);
        EventSystemVariable* variable = static_cast<EventSystemVariable*>(Handle_Get(variableHandle));
        int* curve = static_cast<int*>(Handle_Get(curveHandle));
        if (variable != NULL && curve != NULL)
        {
            variable->m_next = g_eventSystemList->m_variables;
            g_eventSystemList->m_variables = variableHandle;
            variable->m_isFloat = 1;
            variable->m_self = variableHandle;
            variable->m_curve = curveHandle;
            strcpy(variable->m_name, name);
            memcpy(curve, cursor, size);
            variable->m_value.m_intValue = *curve;
        }
        else
        {
            Handle_Free(variableHandle);
            Handle_Free(curveHandle);
        }

        cursor += size / 4;
    }

    const EventSettingsVariable* variable = reinterpret_cast<const EventSettingsVariable*>(cursor);
    for (int variableIndex = 0; variableIndex < settings->variableCount; ++variableIndex, ++variable)
    {
        Var_SetInstance(&g_eventSystemList->m_variables, reinterpret_cast<const char*>(base + variable->nameOffset),
                        const_cast<float*>(&variable->value), 1);
    }
}

namespace
{
const int INSTANCE_ALLOC_LINE = 0x91f;
const int INSTANCE_INIT_BLOCK_LINE = 0x943;
const int INSTANCE_NAME_LINE = 0x948;
const int INSTANCE_LABELS_LINE = 0x94b;
const int INSTANCE_COMPLETION_LINE = 0x950;
const int INSTANCE_PRESET_LINE = 0x957;
const int INSTANCE_BLEND_LINE = 0x96b;
const int BLEND_NODE_HEADER_SIZE = 0x104;
const int SOUND_NAME_SIZE = 512;
const int SOUND_PATH_SIZE = 511;
const int VARIABLE_NAME_SIZE = 128;
const int SILENCE_SIZE = 8;
const unsigned int RANDOM_UNIT_MASK = 0x3ff;
const float RANDOM_UNIT_DIVISOR = 1023.0f;
const float RANGE_EPSILON = 0.001f;
const float FADE_THRESHOLD = 0.1f;
const float MILLISECONDS_PER_SECOND = 1000.0f;
const int SAMPLE_INIT_SILENCE = 3;
const unsigned int SAMPLE_INIT_MONO = 1;
const unsigned int SAMPLE_INIT_STEREO = 3;
const unsigned int SAMPLE_INIT_ADPCM = 4;
const unsigned int SAMPLE_INIT_MULTICHANNEL = 0x11;
const int SAMPLE_CHANNEL_SHIFT = 16;
const int SAMPLE_CHANNEL_STEREO = 2;
const int SAMPLE_CHANNEL_MAXIMUM_SIMPLE = 3;
const int SELECTION_NONE = -1;
const int MARKER_TYPE_SAMPLES = 1;
const int MARKER_TYPE_MILLISECONDS = 2;
const int MARKER_TYPE_CONVERTED = 4;
const int MARKER_FLAG_SHIFT = 3;
const int MODE_MASK = 7;
const int MODE_BLEND = 4;
const char SILENCE_NAME[] = "Silence";
const char FAILED_EVICT_REASON[] = "FailedEvict";
const int SUFFIX_FORMAT_DIGIT = 2;
const unsigned char SILENCE_DATA[SILENCE_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};

void ReleaseSound(EventSystem* eventSystem, _SAMPLE* sample, _STREAM* stream, bool external, Handle sound)
{
    if (external)
    {
        g_externalSampleRelease(sample);
    }
    else if (stream == NULL)
    {
        AIL_release_sample_handle(sample);
        Sound_Free(eventSystem, sound);
    }
    else
    {
        AIL_close_stream(stream);
        Sound_Free(eventSystem, sound);
    }
}
}

int StartSingle(const EVENT_STEP_INFO* step, EventSystem* eventSystem, const char* name, int nameLength,
                void* initBlock, unsigned int initBlockSize, unsigned long long serial, unsigned long long selection,
                signed char blendIndex)
{
    const PlayStep& play = step->m_data.m_play;
    int delay = play.m_delayMinimum;
    if (play.m_delayMaximum != play.m_delayMinimum)
    {
        delay += HorribleRand() % static_cast<unsigned int>(play.m_delayMaximum - play.m_delayMinimum);
    }

    float volume = play.m_volumeMinimum;
    const float volumeRange = play.m_volumeMaximum - volume;
    if (volumeRange > RANGE_EPSILON)
    {
        volume = volumeRange * (static_cast<float>(HorribleRand() & RANDOM_UNIT_MASK) / RANDOM_UNIT_DIVISOR) + volume;
    }

    float pitch = play.m_pitchMinimum;
    const float pitchRange = play.m_pitchMaximum - pitch;
    if (pitchRange > RANGE_EPSILON)
    {
        pitch = pitchRange * (static_cast<float>(HorribleRand() & RANDOM_UNIT_MASK) / RANDOM_UNIT_DIVISOR) + pitch;
    }

    char soundName[SOUND_NAME_SIZE];
    soundName[0] = '\0';
    const InitRecord* const initEnd = reinterpret_cast<const InitRecord*>(static_cast<const char*>(initBlock)
                                                                          + initBlockSize);
    if (nameLength != 0)
    {
        strncpy(soundName, name, nameLength);
        soundName[nameLength] = '\0';
        char* nameEnd = soundName + nameLength;
        if ((play.m_modeFlags >> MARKER_FLAG_SHIFT) != 0)
        {
            const InitRecord* record = static_cast<const InitRecord*>(initBlock);
            for (;;)
            {
                if (record->tag == INIT_RECORD_END || record >= initEnd)
                {
                    ErrorHandler(ERROR_TYPE_SELECTION_MISSING, serial, soundName);
                    break;
                }

                if (record->tag == INIT_RECORD_SUFFIX)
                {
                    if (record->integers[0] < 0)
                    {
                        ErrorHandler(ERROR_TYPE_SELECTION_MISSING, serial, soundName);
                        break;
                    }

                    char* digits = nameEnd;
                    while (digits[-1] >= '0' && digits[-1] <= '9')
                    {
                        --digits;
                    }

                    if (digits != nameEnd)
                    {
                        char format[8] = "%00d";
                        format[SUFFIX_FORMAT_DIGIT] = static_cast<char>(nameEnd - digits + '0');
                        sprintf(digits, format, record->integers[0]);
                    }

                    break;
                }

                record = reinterpret_cast<const InitRecord*>(reinterpret_cast<const char*>(record) + record->size);
            }
        }
    }

    const unsigned long long soundId = g_soundSerial++;
    Audition_Sound(volume, pitch, serial, soundId, soundName, play.m_labels, delay);

    _MSSSTRINGC labelQuery;
    labelQuery.text = play.m_labels;
    labelQuery.length = play.m_labelsLength;
    SoundLimitEntry* labelLimit = IsLabelLimited(eventSystem, labelQuery);
    while (labelLimit != NULL)
    {
        if (EvictSound(eventSystem, soundId, play.m_limitPolicy, play.m_priority, labelLimit, NULL, NULL) == 0)
        {
            Audition_SoundLimited(soundId, reinterpret_cast<LimitEntry*>(labelLimit)->m_query);
            return 1;
        }

        labelLimit = IsLabelLimited(eventSystem, labelQuery);
    }

    unsigned char flags = 0;
    _MILESBANKSOUNDINFO info;
    char path[SOUND_PATH_SIZE];
    memset(&info, 0, sizeof(info));
    if (nameLength != 0)
    {
        if (Container_GetSound(soundName, path, &info) == 0)
        {
            ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, soundId, soundName);
            nameLength = 0;
            flags = SOUND_INSTANCE_FLAG_UNNAMED;
            soundName[0] = '\0';
        }
    }

    if (info.m_maxInstances != 0)
    {
        int matches = 0;
        const int count = eventSystem->m_instances.Count();
        for (int index = 0; index < count; ++index)
        {
            SoundInstanceData* other = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
            if (other->m_nameLength == nameLength
                && AIL_stricmp(static_cast<const char*>(Handle_Get(other->m_name)), soundName) == 0)
            {
                ++matches;
            }

            if (info.m_maxInstances <= matches)
            {
                break;
            }
        }

        if (info.m_maxInstances <= matches)
        {
            if (EvictSound(eventSystem, soundId, play.m_limitPolicy, play.m_priority, NULL, NULL, soundName) == 0)
            {
                Audition_SoundLimited(soundId, FAILED_EVICT_REASON);
                return 1;
            }
        }
    }

    bool external = false;
    _SAMPLE* sample = NULL;
    _STREAM* stream = NULL;
    Handle cachedSound;
    LoadedSound* cachedEntry = NULL;
    bool silence = false;

    if (nameLength == 0)
    {
        silence = true;
    }
    else
    {
        if (g_externalSampleOpen != NULL)
        {
            const InitRecord* selectionRecord = NULL;
            for (const InitRecord* record = static_cast<const InitRecord*>(initBlock);
                 record->tag != INIT_RECORD_END && record < initEnd;
                 record = reinterpret_cast<const InitRecord*>(reinterpret_cast<const char*>(record) + record->size))
            {
                if (record->tag == INIT_RECORD_SELECTION)
                {
                    selectionRecord = record;
                }
            }

            const void* selectionData = NULL;
            int selectionSize = 0;
            if (selectionRecord != NULL)
            {
                selectionData = &selectionRecord->integers[0];
                selectionSize = selectionRecord->size;
                if (selectionRecord->tag != 0)
                {
                    selectionData = reinterpret_cast<const void*>(selectionRecord->integers[0]);
                }
            }

            sample = reinterpret_cast<_SAMPLE*>(
                g_externalSampleOpen(soundName, path, eventSystem->m_driver, selectionData, selectionSize));
            external = sample != NULL;
        }

        if (!external)
        {
            if (play.m_stream == 0)
            {
                cachedSound = Sound_Find(eventSystem, soundName);
                cachedEntry = static_cast<LoadedSound*>(Handle_Get(cachedSound));
                if (cachedEntry == NULL)
                {
                    if (play.m_loadOnDemand == 0)
                    {
                        flags |= SOUND_INSTANCE_FLAG_SILENT;
                        soundName[0] = '\0';
                        silence = true;
                    }
                    else
                    {
                        cachedSound = Sound_Load(eventSystem, soundId, soundName);
                        cachedEntry = static_cast<LoadedSound*>(Handle_Get(cachedSound));
                        if (cachedEntry == NULL)
                        {
                            return 0;
                        }
                    }
                }
                else
                {
                    ++cachedEntry->m_refCount;
                }

                if (!silence)
                {
                    sample = AIL_allocate_sample_handle(eventSystem->m_driver);
                    if (sample == NULL)
                    {
                        ErrorHandler(ERROR_TYPE_NO_PROVIDER, soundId, soundName);
                        Sound_Free(eventSystem, cachedSound);
                        return 0;
                    }

                    if (cachedEntry->m_data == NULL || cachedEntry->m_loadSlot != 0 || cachedEntry->m_loadFailed != 0)
                    {
                        unsigned int initFlags = SAMPLE_INIT_MONO;
                        if (info.m_channels < SAMPLE_CHANNEL_MAXIMUM_SIMPLE && info.m_channelMask == -1)
                        {
                            if (info.m_channels == SAMPLE_CHANNEL_STEREO)
                            {
                                initFlags = SAMPLE_INIT_STEREO;
                            }
                        }
                        else
                        {
                            initFlags = (info.m_channels << SAMPLE_CHANNEL_SHIFT) | SAMPLE_INIT_MULTICHANNEL;
                        }

                        if (info.m_adpcm != 0)
                        {
                            initFlags |= SAMPLE_INIT_ADPCM;
                        }

                        if (AIL_init_sample(sample, initFlags) != 0)
                        {
                            AIL_set_sample_playback_rate(sample, info.m_sampleRate);
                            if (info.m_adpcm != 0)
                            {
                                AIL_set_sample_adpcm_block_size(sample, info.m_adpcmBlockSize);
                            }
                        }

                        if (cachedEntry->m_loadSlot != 0)
                        {
                            flags |= SOUND_INSTANCE_FLAG_WAITING_FOR_DATA;
                        }
                        else
                        {
                            flags |= SOUND_INSTANCE_FLAG_WAITING_FOR_STREAM;
                        }
                    }
                    else if (AIL_set_named_sample_file(sample, path, cachedEntry->m_data, cachedEntry->m_size, 0) == 0)
                    {
                        ErrorHandler(ERROR_TYPE_NO_PROVIDER, soundId, soundName);
                        AIL_release_sample_handle(sample);
                        Sound_Free(eventSystem, cachedSound);
                        return 0;
                    }
                }
            }
            else
            {
                stream = AIL_open_stream(eventSystem->m_driver, path, info.m_streamBufferSize);
                if (stream == NULL)
                {
                    ErrorHandler(ERROR_TYPE_NO_PROVIDER, soundId, soundName);
                    return 0;
                }

                sample = AIL_stream_sample_handle(stream);
            }
        }
    }

    if (silence)
    {
        sample = AIL_allocate_sample_handle(eventSystem->m_driver);
        if (sample == NULL)
        {
            ErrorHandler(ERROR_TYPE_NO_PROVIDER, soundId, SILENCE_NAME);
            return 0;
        }

        if (AIL_init_sample(sample, SAMPLE_INIT_SILENCE) == 0)
        {
            ErrorHandler(ERROR_TYPE_NO_PROVIDER, soundId, SILENCE_NAME);
            AIL_release_sample_handle(sample);
            return 0;
        }

        AIL_set_sample_address(sample, SILENCE_DATA, SILENCE_SIZE);
    }

    int busIndex = BUS_MAIN;
    for (BusNode* bus = static_cast<BusNode*>(Handle_Get(eventSystem->m_buses)); bus != NULL;
         bus = static_cast<BusNode*>(Handle_Get(bus->m_next)))
    {
        if (bus->m_index != 0 && DoLabelsMatchQuery(play.m_labels, bus->m_name) != 0)
        {
            busIndex = bus->m_index;
            AIL_set_sample_bus(sample, bus->m_index);
            break;
        }
    }

    if (eventSystem->m_instances.PushEmpty() == 0)
    {
        ReleaseSound(eventSystem, sample, stream, external, cachedSound);
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, soundId, soundName[0] != '\0' ? soundName : SILENCE_NAME);
        return 0;
    }

    const Handle instanceHandle = Handle_AllocateInfo(sizeof(SoundInstanceData), EVENT_EXEC_FILE, INSTANCE_ALLOC_LINE);
    SoundInstanceData* instance = static_cast<SoundInstanceData*>(Handle_Get(instanceHandle));
    bool built = false;
    if (instance != NULL)
    {
        memset(instance, 0, sizeof(SoundInstanceData));
        instance->m_selectionOffset = SELECTION_NONE;
        for (const InitRecord* record = static_cast<const InitRecord*>(initBlock);
             record->tag != INIT_RECORD_END && record < initEnd;
             record = reinterpret_cast<const InitRecord*>(reinterpret_cast<const char*>(record) + record->size))
        {
            if (record->tag == INIT_RECORD_POSITION)
            {
                sample->x = record->values[0];
                sample->y = record->values[1];
                sample->z = record->values[2];
            }
            else if (record->tag == INIT_RECORD_SELECTION)
            {
                instance->m_selectionOffset = reinterpret_cast<const char*>(record)
                                              - static_cast<const char*>(initBlock);
            }
            else if (record->tag == INIT_RECORD_VARIABLE)
            {
                Var_SetInstance(&instance->m_context, reinterpret_cast<const char*>(&record->size),
                                const_cast<InitRecord*>(record), 1);
            }
            else if (record->tag == INIT_RECORD_VELOCITY)
            {
                sample->velocityX = record->values[0] * record->values[3];
                sample->velocityY = record->values[1] * record->values[3];
                sample->velocityZ = record->values[2] * record->values[3];
            }
        }

        instance->m_initBlock = Handle_AllocateInfo(initBlockSize, EVENT_EXEC_FILE, INSTANCE_INIT_BLOCK_LINE);
        void* initCopy = Handle_Get(instance->m_initBlock);
        if (initCopy != NULL)
        {
            memcpy(initCopy, initBlock, initBlockSize);
            instance->m_initBlockSize = initBlockSize;
            if (Handle_AllocStringInfo(&instance->m_name, &instance->m_nameLength, soundName, EVENT_EXEC_FILE,
                                       INSTANCE_NAME_LINE)
                    != 0
                && Handle_AllocStringInfo(&instance->m_labels, NULL, play.m_labels, EVENT_EXEC_FILE,
                                          INSTANCE_LABELS_LINE)
                       != 0
                && (play.m_completionEventLength == 0
                    || Handle_AllocStringInfo(&instance->m_self, NULL, play.m_completionEvent, EVENT_EXEC_FILE,
                                              INSTANCE_COMPLETION_LINE)
                           != 0)
                && (play.m_presetLength == 0 || play.m_presetPersistent == 0
                    || Handle_AllocStringInfo(&instance->m_preset, NULL, play.m_preset, EVENT_EXEC_FILE,
                                              INSTANCE_PRESET_LINE)
                           != 0))
            {
                built = true;
                if ((play.m_modeFlags & MODE_MASK) == MODE_BLEND)
                {
                    BlendListNode* blend = static_cast<BlendListNode*>(Handle_Get(eventSystem->m_blends));
                    while (blend != NULL && AIL_stricmp(blend->m_name, play.m_selector) != 0)
                    {
                        blend = static_cast<BlendListNode*>(Handle_Get(blend->m_next));
                    }

                    if (blend == NULL)
                    {
                        const Handle blendHandle = Handle_AllocateInfo(play.m_selectorLength + BLEND_NODE_HEADER_SIZE,
                                                                       EVENT_EXEC_FILE, INSTANCE_BLEND_LINE);
                        blend = static_cast<BlendListNode*>(Handle_Get(blendHandle));
                        if (blend == NULL)
                        {
                            built = false;
                        }
                        else
                        {
                            memset(blend, 0, BLEND_NODE_HEADER_SIZE);
                            strcpy(blend->m_name, play.m_selector);
                            blend->m_self = blendHandle;
                            blend->m_next = eventSystem->m_blends;
                            eventSystem->m_blends = blendHandle;
                        }
                    }

                    if (built)
                    {
                        instance->m_blend = blend->m_self;
                    }
                }
            }
        }
    }

    if (!built)
    {
        if (instance != NULL)
        {
            Handle_Free(instance->m_labels);
            Handle_Free(instance->m_name);
            Handle_Free(instance->m_self);
            Handle_Free(instance->m_initBlock);
            Handle_Free(instance->m_preset);
        }

        Handle_Free(instanceHandle);
        eventSystem->m_instances.Pop();
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, soundId, soundName[0] != '\0' ? soundName : SILENCE_NAME);
        ReleaseSound(eventSystem, sample, stream, external, cachedSound);
        return 0;
    }

    instance->m_sample = sample;
    instance->m_stream = stream;
    instance->m_eventSystem = eventSystem;
    instance->m_flags = flags;
    instance->m_paused = 0;
    instance->m_state = SOUND_STATE_PENDING;
    instance->m_source = cachedSound;
    instance->m_serial = serial;
    instance->m_selection = selection;
    instance->m_eventId = soundId;
    instance->m_priority = play.m_priority;
    instance->m_startTime = AIL_ms_count();
    instance->m_externalSample = external;
    instance->m_volume = 1.0f;
    if (play.m_fadeTime > FADE_THRESHOLD)
    {
        instance->m_volume = 0.0f;
        instance->m_fadeRate = 1.0f / play.m_fadeTime;
    }

    instance->m_pitch = pitch;
    instance->m_gain = volume;
    instance->m_blendIndex = blendIndex;
    instance->m_delay = delay;
    instance->m_delayRemaining = static_cast<float>(delay) / MILLISECONDS_PER_SECOND;
    eventSystem->m_instances.Top() = instanceHandle;
    sample->falloffHandle = instanceHandle;

    if (play.m_variablesLength != 0 && *play.m_variables != '\0')
    {
        const char* cursor = play.m_variables;
        for (;;)
        {
            const char* firstSeparator = strchr(cursor, ':');
            if (firstSeparator == NULL)
            {
                break;
            }

            char variableName[VARIABLE_NAME_SIZE];
            memcpy(variableName, cursor, firstSeparator - cursor);
            variableName[firstSeparator - cursor] = '\0';
            const float minimum = atof_with_len(firstSeparator + 1, 0);
            const char* secondSeparator = strchr(firstSeparator + 1, ':');
            if (secondSeparator == NULL)
            {
                break;
            }

            const float maximum = atof_with_len(secondSeparator + 1, 0);
            float value = minimum;
            if (maximum - minimum > RANGE_EPSILON)
            {
                value = (maximum - minimum)
                            * (static_cast<float>(HorribleRand() & RANDOM_UNIT_MASK) / RANDOM_UNIT_DIVISOR)
                        + minimum;
            }

            Var_SetInstance(&instance->m_context, variableName, &value, 1);
            const char* thirdSeparator = strchr(secondSeparator + 2, ':');
            if (thirdSeparator == NULL || thirdSeparator[1] == '\0')
            {
                break;
            }

            cursor = thirdSeparator + 1;
        }
    }

    Persist_Apply(eventSystem, reinterpret_cast<SoundInstance*>(instance), sample, play.m_labels, play.m_labelsLength,
                  0);
    if (play.m_presetLength != 0)
    {
        void* preset = Container_GetPreset(play.m_preset);
        if (preset != NULL)
        {
            AIL_resolve_raw_sound_preset(preset, instance, EvalFunc);
            AIL_apply_raw_sound_preset(sample, preset);
        }
        else
        {
            ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, soundId, play.m_preset);
        }
    }

    SampleEventState* sampleState = reinterpret_cast<SampleEventState*>(sample);
    sampleState->m_markerList = Container_GetMarkerList(soundName);
    instance->m_loopEnd = -1;
    instance->m_loopStart = 0;
    if (play.m_loopEndMarkerLength != 0)
    {
        int markerType = 0;
        if (FindMarker(eventSystem, reinterpret_cast<SoundInstance*>(instance), sampleState->m_markerList,
                       play.m_loopEndMarker, &instance->m_loopEnd, &markerType)
            != 0)
        {
            if (markerType == MARKER_TYPE_MILLISECONDS)
            {
                if (instance->m_loopEnd != 0)
                {
                    instance->m_loopEnd = static_cast<unsigned long long>(
                                              static_cast<unsigned int>(instance->m_loopEnd))
                                          * static_cast<unsigned int>(info.m_sampleRate) / 1000;
                }

                markerType = MARKER_TYPE_CONVERTED;
            }

            instance->m_markerFlags |= markerType << MARKER_FLAG_SHIFT;
        }
    }

    if (play.m_loopStartMarkerLength != 0)
    {
        int markerIndex = 0;
        int markerType = 0;
        if (FindMarker(eventSystem, reinterpret_cast<SoundInstance*>(instance), sampleState->m_markerList,
                       play.m_loopStartMarker, &markerIndex, &markerType)
            != 0)
        {
            if (markerIndex == -1)
            {
                markerIndex = 0;
            }

            if (markerType == MARKER_TYPE_MILLISECONDS)
            {
                instance->m_loopStart = static_cast<unsigned long long>(
                                            static_cast<unsigned int>(instance->m_loopStart))
                                        * static_cast<unsigned int>(info.m_sampleRate) / 1000;
                markerType = MARKER_TYPE_CONVERTED;
            }

            instance->m_loopStart = markerIndex;
            if (markerType != 0)
            {
                instance->m_markerFlags |= markerType;
            }
        }
    }

    if (play.m_positionMarkerLength != 0)
    {
        if (FindMarker(eventSystem, reinterpret_cast<SoundInstance*>(instance), sampleState->m_markerList,
                       play.m_positionMarker, &instance->m_startPosition, &instance->m_startPositionType)
            != 0)
        {
            if (instance->m_startPositionType == MARKER_TYPE_SAMPLES)
            {
                instance->m_startPositionType = 0;
            }
            else if (instance->m_startPositionType == MARKER_TYPE_MILLISECONDS)
            {
                instance->m_startPositionType = 1;
            }
            else if (instance->m_startPositionType == MARKER_TYPE_CONVERTED)
            {
                instance->m_startPositionType = 1;
                instance->m_startPosition = static_cast<unsigned long long>(
                                                static_cast<unsigned int>(instance->m_startPosition))
                                            * 1000 / static_cast<unsigned int>(info.m_sampleRate);
            }
        }
    }

    if (stream == NULL)
    {
        AIL_set_sample_loop_count(sample, play.m_loopCount);
    }
    else
    {
        AIL_set_stream_loop_count(stream, play.m_loopCount);
    }

    AIL_set_sample_playback_delay(sample, delay);
    float leftLevel;
    float rightLevel;
    AIL_sample_volume_levels(sample, &leftLevel, &rightLevel);
    AIL_set_sample_volume_levels(sample, leftLevel * volume, rightLevel * volume);
    const float playbackRate = AIL_sample_playback_rate_factor(sample);
    AIL_set_sample_playback_rate_factor(sample, playbackRate * pitch);
    float blendVolume;
    float blendPitch;
    Blend_Apply(reinterpret_cast<SoundInstance*>(instance), &blendVolume, &blendPitch);
    Ramp_ApplyAll(eventSystem, reinterpret_cast<SoundInstance*>(instance), 0.0f);

    LimitList* limits = static_cast<LimitList*>(Handle_Get(eventSystem->m_limits));
    if (limits != NULL)
    {
        for (LimitEntry* entry = static_cast<LimitEntry*>(Handle_Get(limits->m_entries)); entry != NULL;
             entry = static_cast<LimitEntry*>(Handle_Get(entry->m_next)))
        {
            if (DoLabelsMatchQuery(play.m_labels, entry->m_query) != 0)
            {
                ++entry->m_count;
            }
        }
    }

    LoadedSound* loaded = static_cast<LoadedSound*>(Handle_Get(instance->m_source));
    if (loaded != NULL)
    {
        ++loaded->m_playCount;
    }

    Audition_SoundFlags(soundId, instance->m_flags);
    Audition_SoundBus(soundId, busIndex);
    return 1;
}

extern int strtoi(const char* value, const char** end);

const int BEGIN_ASYNC_LOAD_LINE = 0x2f0;
const int BEGIN_ASYNC_OWNER_LINE = 0x2ec;
const int BUS_PRESET_LINE = 0xb57;
const int CONTROL_PRESET_LINE = 0xc18;
const int ENVIRONMENT_NAME_LINE = 0xd29;
const int TWEEN_ALLOC_LINE = 0xd4e;
const int BLEND_SOUND_LIMIT = 10;
const int LOOP_POINT_UNCHANGED = -2;
const int LOOP_COUNT_UNCHANGED = -1;
const int BUS_NAME_PREFIX_LENGTH = 4;
const int BUS_CONTROL_NAME_LENGTH = 5;
const char BUS_NAME_PREFIX[] = "_Bus";
const char NO_EVENT_SYSTEMS_ERROR[] = "No Event Systems Active";
const char AUDITION_EVENT_PREFIX = 'Z';
const float CONTROL_FADE_THRESHOLD = 0.1f;
const float CONTROL_FADE_TARGET = -1.0f;
const int SAMPLE_DONE = 2;
const int STREAM_DONE = -1;
const int ACTION_PAUSE = 1;
const int ACTION_RESUME = 2;
const int ACTION_STOP = 3;
const int ACTION_STOP_AND_FREE = 4;
const int ACTION_FADE_OUT = 5;
const int CONTROL_MARKER_MILLISECONDS = 2;
const int CONTROL_MARKER_CONVERTED = 4;
const int CONTROL_MARKER_SAMPLES = 1;

static inline bool InstanceMatchesTarget(const SoundInstanceData* instance, unsigned long long target)
{
    return instance->m_state != SOUND_STATE_FINISHED
           && ((target == 0) || instance->m_serial == target || instance->m_eventId == target
               || instance->m_selection == target);
}

static bool SameHandle(const Handle& first, const Handle& second)
{
    return first.index == second.index && first.generation == second.generation;
}

static void FreeNodeList(Handle* head)
{
    EventListNode* node = static_cast<EventListNode*>(Handle_Get(*head));
    while (node != NULL)
    {
        const Handle next = node->m_next;
        Handle_Free(node->m_self);
        node = static_cast<EventListNode*>(Handle_Get(next));
    }
}

static void PlayStepSelect(const EVENT_STEP_INFO* step, EventSystem* eventSystem, QueuedEvent* event,
                           const unsigned char* stepKey)
{
    const PlayStep& play = step->m_data.m_play;
    unsigned int pickMask = 0;
    int forcedIndex = -1;
    int state = 0;
    const int mode = play.m_modeFlags & MODE_MASK;

    if (mode != 0)
    {
        if (play.m_selectorLength == 0)
        {
            state = StateVar_Lookup(eventSystem, const_cast<unsigned char*>(stepKey));
        }
        else
        {
            Var_LookupWithContext(eventSystem, eventSystem->m_variables, play.m_selector, 0, &state);
        }

        const int selectMode = play.m_modeFlags & MODE_MASK;
        if (selectMode == 1 || selectMode == 3)
        {
            pickMask = state;
        }
        else if (selectMode == 2)
        {
            forcedIndex = state;
        }
    }

    int totalWeight = 0;
    unsigned int playedCount = 0;
    int entryCount = 0;
    unsigned int availableWeight = 0;
    const char* cursor = play.m_sounds;
    while (*cursor != '\0')
    {
        if (*cursor != ':')
        {
            cursor = strchr(cursor, ':') + 1;
        }
        else
        {
            ++cursor;
        }

        const int weight = strtoi(cursor, NULL);
        totalWeight += weight;
        if ((pickMask & (1 << (entryCount & 0x3f))) != 0)
        {
            ++playedCount;
        }
        else
        {
            availableWeight += weight;
        }

        cursor = strchr(cursor, ':') + 1;
        ++entryCount;
    }

    if (entryCount <= forcedIndex)
    {
        forcedIndex = 0;
    }

    int weightRange;
    if (static_cast<int>(playedCount) == entryCount)
    {
        pickMask = 0;
        weightRange = totalWeight;
    }
    else
    {
        weightRange = availableWeight;
    }

    const char* chosen = NULL;
    int chosenLength = 0;
    int chosenIndex = 0;
    if (weightRange == 0)
    {
        chosen = play.m_sounds;
        if (*chosen != '\0')
        {
            chosenLength = strchr(chosen, ':') - chosen;
        }
    }
    else
    {
        int remaining = HorribleRand() % static_cast<unsigned int>(weightRange);
        int index = 0;
        cursor = play.m_sounds;
        while (*cursor != '\0')
        {
            const char* separator = cursor;
            if (*cursor != ':')
            {
                separator = strchr(cursor, ':');
            }

            const char* weightText = separator + 1;
            bool pick = false;
            if ((pickMask & (1 << (index & 0x3f))) == 0)
            {
                const int weight = strtoi(weightText, NULL);
                pick = remaining < weight;
                remaining -= weight;
            }

            if (forcedIndex != -1)
            {
                pick = forcedIndex == index;
            }

            if (pick)
            {
                chosen = cursor;
                chosenLength = weightText - 1 - cursor;
                chosenIndex = index;
                break;
            }

            cursor = strchr(weightText, ':');
            ++cursor;
            ++index;
        }
    }

    unsigned int newState = chosenIndex;
    if (mode != 0)
    {
        if (mode == 3)
        {
            newState = pickMask | (1 << (chosenIndex & 0x3f));
        }
        else if (mode == 1)
        {
            newState = 1 << (chosenIndex & 0x3f);
        }
        else if (mode == 2)
        {
            newState = forcedIndex + 1;
        }

        if (play.m_selectorLength != 0)
        {
            MilesSetVarIInternal(eventSystem, play.m_selector, newState);
            StartSingle(step, eventSystem, chosen, chosenLength, event->data, event->dataSize, event->serial,
                        event->selection, -1);
            return;
        }

        StateVar_Set(eventSystem, const_cast<unsigned char*>(stepKey), newState);
    }

    StartSingle(step, eventSystem, chosen, chosenLength, event->data, event->dataSize, event->serial, event->selection,
                -1);
}

static void PlayStepBlend(const EVENT_STEP_INFO* step, EventSystem* eventSystem, QueuedEvent* event)
{
    const PlayStep& play = step->m_data.m_play;
    if (Blend_Exists(eventSystem, play.m_selector) == 0)
    {
        ErrorHandler(ERROR_TYPE_BLEND_NOT_SET, event->serial, play.m_selector);
        return;
    }

    const char* cursor = play.m_sounds;
    signed char index = 0;
    while (*cursor != '\0')
    {
        const char* next;
        if (*cursor == ':')
        {
            next = cursor + 1;
        }
        else
        {
            next = strchr(cursor, ':') + 1;
        }

        const int length = next - cursor;
        if (index >= BLEND_SOUND_LIMIT)
        {
            ErrorHandler(ERROR_TYPE_TOO_MANY_BLEND_SOUNDS, event->serial, NULL);
            break;
        }

        if (StartSingle(step, eventSystem, cursor, length - 1, event->data, event->dataSize, event->serial,
                        event->selection, index)
            == 0)
        {
            break;
        }

        index = static_cast<signed char>(index + 1);
        next = strchr(next, ':');
        cursor = next + 1;
    }
}

static void ControlBus(const ControlStep& control, EventSystem* eventSystem, const char* busName,
                       unsigned long long serial)
{
    if (control.m_presetLength == 0)
    {
        return;
    }

    const char busDigit = busName[BUS_NAME_PREFIX_LENGTH];
    void* preset = Container_GetPreset(control.m_preset);
    if (preset == NULL)
    {
        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, serial, control.m_preset);
        return;
    }

    BusNode* bus = static_cast<BusNode*>(Handle_Get(eventSystem->m_buses));
    while (bus != NULL)
    {
        if (static_cast<int>(bus->m_index) == busDigit - '0')
        {
            g_eventSystem = eventSystem;
            AIL_resolve_raw_sound_preset(preset, NULL, EvalFunc);
            AIL_apply_raw_sound_preset(bus->m_sample, preset);
            Handle_Free(bus->m_preset);
            bus->m_preset = Handle_AllocateInfo(control.m_presetLength + 1, EVENT_EXEC_FILE, BUS_PRESET_LINE);
            char* text = static_cast<char*>(Handle_Get(bus->m_preset));
            if (text == NULL)
            {
                ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, serial, NULL);
            }
            else
            {
                strcpy(text, control.m_preset);
            }

            bus->m_presetEnabled = control.m_presetPersistent != 0;
        }
        else
        {
            ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, serial, control.m_preset);
        }

        bus = static_cast<BusNode*>(Handle_Get(bus->m_next));
    }
}

static void ControlInstance(const ControlStep& control, EventSystem* eventSystem, SoundInstanceData* instance,
                            unsigned long long serial)
{
    _SAMPLE* sample = instance->m_sample;
    if (control.m_labelsLength != 0
        && DoLabelsMatchQuery(static_cast<const char*>(Handle_Get(instance->m_labels)), control.m_labels) == 0)
    {
        return;
    }

    const int action = control.m_action;
    if (action == ACTION_STOP)
    {
        DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
        return;
    }

    if (action == ACTION_STOP_AND_FREE)
    {
        DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
        Handle_Free(instance->m_self);
        return;
    }

    if (action == ACTION_FADE_OUT)
    {
        if (control.m_fadeTime < CONTROL_FADE_THRESHOLD)
        {
            DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
        }
        else
        {
            instance->m_fadeRate = CONTROL_FADE_TARGET / control.m_fadeTime;
        }

        return;
    }

    if (action == ACTION_PAUSE)
    {
        if (instance->m_stream != NULL)
        {
            AIL_pause_stream(instance->m_stream, 1);
        }
        else
        {
            AIL_stop_sample(sample);
        }

        instance->m_paused = 1;
    }

    int loopStart = LOOP_POINT_UNCHANGED;
    int loopEnd = LOOP_POINT_UNCHANGED;
    int position = LOOP_POINT_UNCHANGED;
    int loopStartType = 0;
    int loopEndType = 0;
    int positionType = 0;
    SampleEventState* sampleState = reinterpret_cast<SampleEventState*>(sample);
    if (control.m_loopEndMarkerLength != 0)
    {
        if (FindMarker(eventSystem, reinterpret_cast<SoundInstance*>(instance), sampleState->m_markerList,
                       control.m_loopEndMarker, &loopEnd, &loopEndType)
            == 0)
        {
            loopEnd = LOOP_POINT_UNCHANGED;
        }
    }

    if (control.m_loopStartMarkerLength != 0)
    {
        if (FindMarker(eventSystem, reinterpret_cast<SoundInstance*>(instance), sampleState->m_markerList,
                       control.m_loopStartMarker, &loopStart, &loopStartType)
            == 0)
        {
            loopStart = LOOP_POINT_UNCHANGED;
        }
    }

    if (control.m_positionMarkerLength != 0)
    {
        if (FindMarker(eventSystem, reinterpret_cast<SoundInstance*>(instance), sampleState->m_markerList,
                       control.m_positionMarker, &position, &positionType)
            == 0)
        {
            position = LOOP_POINT_UNCHANGED;
        }
    }

    _MILESBANKSOUNDINFO info;
    info.m_sampleRate = 0;
    if (loopEndType == CONTROL_MARKER_MILLISECONDS || loopStartType == CONTROL_MARKER_MILLISECONDS
        || positionType == CONTROL_MARKER_CONVERTED)
    {
        Container_GetSound(static_cast<const char*>(Handle_Get(instance->m_name)), NULL, &info);
        if (loopEndType == CONTROL_MARKER_MILLISECONDS)
        {
            loopEndType = CONTROL_MARKER_CONVERTED;
            loopEnd = static_cast<unsigned long long>(static_cast<unsigned int>(loopEnd))
                      * static_cast<unsigned int>(info.m_sampleRate) / 1000;
        }

        if (loopStartType == CONTROL_MARKER_MILLISECONDS)
        {
            loopStartType = CONTROL_MARKER_CONVERTED;
            loopStart = static_cast<unsigned long long>(static_cast<unsigned int>(loopStart))
                        * static_cast<unsigned int>(info.m_sampleRate) / 1000;
        }

        if (positionType == CONTROL_MARKER_CONVERTED)
        {
            positionType = CONTROL_MARKER_MILLISECONDS;
            position = static_cast<unsigned long long>(static_cast<unsigned int>(position)) * 1000
                       / static_cast<unsigned int>(info.m_sampleRate);
        }
    }

    _STREAM* stream = instance->m_stream;
    if (stream != NULL)
    {
        if ((loopStart != LOOP_POINT_UNCHANGED || loopEnd != LOOP_POINT_UNCHANGED)
            && loopStartType == CONTROL_MARKER_SAMPLES && loopEndType == CONTROL_MARKER_SAMPLES)
        {
            AIL_set_stream_loop_block(stream, loopStart, loopEnd);
        }

        if (position != LOOP_POINT_UNCHANGED)
        {
            if (positionType == CONTROL_MARKER_MILLISECONDS)
            {
                AIL_set_stream_ms_position(stream, position);
            }
            else
            {
                AIL_set_stream_position(stream, position);
            }
        }

        if (control.m_loopCount != LOOP_COUNT_UNCHANGED)
        {
            AIL_set_stream_loop_count(stream, control.m_loopCount);
        }
    }
    else
    {
        if (loopStart != LOOP_POINT_UNCHANGED || loopEnd != LOOP_POINT_UNCHANGED)
        {
            if (loopStartType == CONTROL_MARKER_SAMPLES)
            {
                if (loopEndType == CONTROL_MARKER_SAMPLES)
                {
                    AIL_set_sample_loop_block(sample, loopStart, loopEnd);
                }
                else if (loopEndType == CONTROL_MARKER_CONVERTED)
                {
                    AIL_set_sample_loop_samples(sample, LOOP_POINT_UNCHANGED, loopEnd);
                    AIL_set_sample_loop_block(sample, loopStart, LOOP_POINT_UNCHANGED);
                }
                else
                {
                    AIL_set_sample_loop_samples(sample, loopStart, loopEnd);
                }
            }
            else if (loopStartType == CONTROL_MARKER_CONVERTED && loopEndType == CONTROL_MARKER_SAMPLES)
            {
                AIL_set_sample_loop_samples(sample, loopStart, LOOP_POINT_UNCHANGED);
                AIL_set_sample_loop_block(sample, LOOP_POINT_UNCHANGED, loopEnd);
            }
            else
            {
                AIL_set_sample_loop_samples(sample, loopStart, loopEnd);
            }
        }

        if (position != LOOP_POINT_UNCHANGED)
        {
            if (positionType == CONTROL_MARKER_MILLISECONDS)
            {
                AIL_set_sample_ms_position(sample, position);
            }
            else
            {
                AIL_set_sample_position(sample, position);
            }
        }

        if (control.m_loopCount != LOOP_COUNT_UNCHANGED)
        {
            AIL_set_sample_loop_count(sample, control.m_loopCount);
        }
    }

    if (control.m_presetLength != 0)
    {
        void* preset = Container_GetPreset(control.m_preset);
        if (preset != NULL)
        {
            AIL_resolve_raw_sound_preset(preset, instance, EvalFunc);
            AIL_apply_raw_sound_preset(sample, preset);
            if (instance->m_preset.index != 0)
            {
                Handle_Free(instance->m_preset);
                instance->m_preset = Handle();
            }

            if (control.m_presetPersistent != 0)
            {
                instance->m_preset = Handle_AllocateInfo(control.m_presetLength + 1, EVENT_EXEC_FILE,
                                                         CONTROL_PRESET_LINE);
                char* text = static_cast<char*>(Handle_Get(instance->m_preset));
                if (text == NULL)
                {
                    ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, serial, NULL);
                }
                else
                {
                    strcpy(text, control.m_preset);
                }
            }
        }
        else
        {
            ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, serial, control.m_preset);
        }
    }

    if (action == ACTION_RESUME)
    {
        if (instance->m_stream != NULL)
        {
            AIL_pause_stream(instance->m_stream, 0);
        }
        else
        {
            AIL_resume_sample(sample);
        }

        instance->m_paused = 0;
    }
}

static void StepControl(const EVENT_STEP_INFO* step, EventSystem* eventSystem, QueuedEvent* event)
{
    const ControlStep& control = step->m_data.m_control;
    if ((event->flags & QUEUED_EVENT_AUDITIONED) != 0)
    {
        Audition_Control(event->serial, control.m_labels, control.m_action, event->context);
    }

    if (control.m_labelsLength == BUS_CONTROL_NAME_LENGTH
        && strncmp(control.m_labels, BUS_NAME_PREFIX, BUS_NAME_PREFIX_LENGTH) == 0)
    {
        ControlBus(control, eventSystem, control.m_labels, event->serial);
        return;
    }

    const int count = eventSystem->m_instances.Count();
    for (int index = 0; index < count; ++index)
    {
        SoundInstanceData* instance = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
        if (InstanceMatchesTarget(instance, event->context))
        {
            ControlInstance(control, eventSystem, instance, event->serial);
        }
    }
}

static void StepEnvironment(const EVENT_STEP_INFO* step, EventSystem* eventSystem, QueuedEvent* event)
{
    const EnvironmentStep& environment = step->m_data.m_environment;
    if (environment.m_nameLength == 0)
    {
        if (eventSystem->m_environment.index != 0)
        {
            const char* name = static_cast<const char*>(Handle_Get(eventSystem->m_environment));
            void* preset = Container_GetEnvironment(name);
            if (preset != NULL)
            {
                AIL_unapply_raw_environment_preset(eventSystem->m_driver, preset);
            }

            Handle_Free(eventSystem->m_environment);
            eventSystem->m_environment = Handle();
        }

        eventSystem->m_environmentActive = 0;
        return;
    }

    void* preset = Container_GetEnvironment(environment.m_name);
    if (preset == NULL)
    {
        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, event->serial, environment.m_name);
        return;
    }

    if (eventSystem->m_environment.index != 0)
    {
        Handle_Free(eventSystem->m_environment);
        eventSystem->m_environment = Handle();
    }

    eventSystem->m_environment = Handle_AllocateInfo(environment.m_nameLength + 1, EVENT_EXEC_FILE,
                                                     ENVIRONMENT_NAME_LINE);
    char* text = static_cast<char*>(Handle_Get(eventSystem->m_environment));
    if (text == NULL)
    {
        ErrorHandler(ERROR_TYPE_OUT_OF_EVENT_SYSTEM_MEMORY, event->serial, NULL);
        return;
    }

    strcpy(text, environment.m_name);
    eventSystem->m_environmentActive = environment.m_enabled != 0;
    g_eventSystem = eventSystem;
    AIL_resolve_raw_environment_preset(preset, EvalFunc);
    AIL_apply_raw_environment_preset(eventSystem->m_driver, preset);
}

static void StepRamp(const EVENT_STEP_INFO* step, EventSystem* eventSystem, QueuedEvent* event)
{
    const RampStep& ramp = step->m_data.m_ramp;
    if (event->context == 0 && ramp.m_global == 1)
    {
        Audition_AddRamp(0, ramp.m_target, ramp.m_name, ramp.m_labels, 1, event->serial);
        Ramp_Add(&eventSystem->m_ramps, eventSystem, step);
        return;
    }

    const int count = eventSystem->m_instances.Count();
    for (int index = 0; index < count; ++index)
    {
        SoundInstanceData* instance = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
        if (InstanceMatchesTarget(instance, event->context)
            && DoLabelsMatchQuery(static_cast<const char*>(Handle_Get(instance->m_labels)), ramp.m_labels) != 0)
        {
            Audition_AddRamp(instance->m_eventId, ramp.m_target, ramp.m_name, ramp.m_labels, 0, event->serial);
            if (Ramp_Add(&instance->m_ramps, eventSystem, step) == 0)
            {
                return;
            }
        }
    }
}

static void StepReset(EventSystem* eventSystem, QueuedEvent* event)
{
    PersistNode* persist = static_cast<PersistNode*>(Handle_Get(eventSystem->m_persists));
    while (persist != NULL)
    {
        Persist_Free(eventSystem, event->serial, persist->m_name);
        persist = static_cast<PersistNode*>(Handle_Get(eventSystem->m_persists));
    }

    EventListNode* blend = static_cast<EventListNode*>(Handle_Get(eventSystem->m_blends));
    while (blend != NULL)
    {
        eventSystem->m_blends = blend->m_next;
        Handle_Free(blend->m_self);
        blend = static_cast<EventListNode*>(Handle_Get(eventSystem->m_blends));
    }

    EventListNode* ramp = static_cast<EventListNode*>(Handle_Get(eventSystem->m_ramps));
    while (ramp != NULL)
    {
        eventSystem->m_ramps = ramp->m_next;
        Handle_Free(ramp->m_self);
        ramp = static_cast<EventListNode*>(Handle_Get(eventSystem->m_ramps));
    }

    if (eventSystem->m_environment.index != 0)
    {
        const char* name = static_cast<const char*>(Handle_Get(eventSystem->m_environment));
        void* preset = Container_GetEnvironment(name);
        if (preset != NULL)
        {
            AIL_unapply_raw_environment_preset(eventSystem->m_driver, preset);
        }

        Handle_Free(eventSystem->m_environment);
        eventSystem->m_environment = Handle();
    }

    Audition_ClearState();
}

static void StepLimitSelect(const EVENT_STEP_INFO* step, EventSystem* eventSystem, QueuedEvent* event)
{
    const NameStep& name = step->m_data.m_name;
    Handle current = eventSystem->m_limits;
    Handle previous;
    for (;;)
    {
        LimitList* list = static_cast<LimitList*>(Handle_Get(current));
        if (list == NULL)
        {
            ErrorHandler(ERROR_TYPE_BAD_STRING_PASSED_TO_SET_LIMITS, event->serial, name.m_name);
            return;
        }

        if (AIL_stricmp(list->m_name, name.m_name) == 0)
        {
            if (SameHandle(current, eventSystem->m_limits))
            {
                return;
            }

            LimitList* previousList = static_cast<LimitList*>(Handle_Get(previous));
            previousList->m_next = list->m_next;
            eventSystem->m_limits = list->m_self;
            list->m_next = previousList->m_self;
            LimitEntry* entry = static_cast<LimitEntry*>(Handle_Get(list->m_entries));
            while (entry != NULL)
            {
                entry->m_count = 0;
                const int count = eventSystem->m_instances.Count();
                for (int index = 0; index < count; ++index)
                {
                    SoundInstanceData* instance = static_cast<SoundInstanceData*>(
                        Handle_Get(eventSystem->m_instances[index]));
                    if (instance->m_state != SOUND_STATE_FINISHED
                        && DoLabelsMatchQuery(static_cast<const char*>(Handle_Get(instance->m_labels)), entry->m_query)
                               != 0)
                    {
                        ++entry->m_count;
                    }
                }

                entry = static_cast<LimitEntry*>(Handle_Get(entry->m_next));
            }

            return;
        }

        previous = current;
        current = list->m_next;
    }
}

static inline void SetTweenSegments(TweenNode* node, const TweenStep& tween)
{
    node->m_curves[0] = tween.m_firstCurve;
    node->m_curves[1] = tween.m_secondCurve;
    node->m_values[0] = tween.m_start;
    node->m_values[1] = tween.m_middle;
    node->m_values[2] = tween.m_end;
    node->m_times[0] = tween.m_firstTime;
    node->m_times[1] = tween.m_secondTime;
}

static void StepTween(const EVENT_STEP_INFO* step, EventSystem* eventSystem)
{
    const TweenStep& tween = step->m_data.m_tween;
    TweenNode* node = static_cast<TweenNode*>(Handle_Get(eventSystem->m_tweens));
    while (node != NULL)
    {
        if (AIL_stricmp(node->m_name, tween.m_name) == 0)
        {
            node->m_elapsed = 0.0f;
            SetTweenSegments(node, tween);
            return;
        }

        node = static_cast<TweenNode*>(Handle_Get(node->m_next));
    }

    const Handle handle = Handle_AllocateInfo(tween.m_nameLength + sizeof(TweenNode) + 1, EVENT_EXEC_FILE,
                                              TWEEN_ALLOC_LINE);
    node = static_cast<TweenNode*>(Handle_Get(handle));
    if (node != NULL)
    {
        memcpy(node->m_name, tween.m_name, tween.m_nameLength + 1);
        node->m_self = handle;
        node->m_next = eventSystem->m_tweens;
        eventSystem->m_tweens = handle;
        SetTweenSegments(node, tween);
        node->m_elapsed = 0.0f;
    }
}

static void UpdateSoundLoads(EventSystem* eventSystem)
{
    int count = eventSystem->m_sounds.Count();
    for (int index = 0; index < count; ++index)
    {
        LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(eventSystem->m_sounds[index]));
        if (sound->m_loadSlot == 0)
        {
            break;
        }

        MilesAsyncRead* request = &g_soundLoadRequests[sound->m_loadSlot - 1];
        if (MilesAsyncFileStatus(request, 0) == 0)
        {
            continue;
        }

        const int status = request->iStatus;
        if (status == 1 && sound->m_data == NULL)
        {
            sound->m_size = request->iLength;
            sound->m_data = request->pBuffer;
        }

        eventSystem->m_soundBytes += sound->m_size;
        if (status == 1)
        {
            Audition_AsyncComplete(static_cast<int>(reinterpret_cast<intptr_t>(request->pPriorityUserData)), sound->m_size);
            g_soundLoadBusy[sound->m_loadSlot - 1] = 0;
            sound->m_loadSlot = 0;
        }
        else
        {
            Audition_AsyncError(static_cast<int>(reinterpret_cast<intptr_t>(request->pPriorityUserData)));
            g_soundLoadBusy[sound->m_loadSlot - 1] = 0;
            sound->m_loadSlot = 0;
        }

        for (int next = index + 1; next < count; ++next)
        {
            Handle& current = eventSystem->m_sounds[next];
            const Handle following = current;
            if (static_cast<LoadedSound*>(Handle_Get(following))->m_loadSlot == 0)
            {
                break;
            }

            current = eventSystem->m_sounds[next - 1];
            eventSystem->m_sounds[next - 1] = following;
        }

        if (status != 1)
        {
            sound->m_loadFailed = 1;
            ErrorHandler(ERROR_TYPE_ASYNC_LOAD_FAILED, 0, static_cast<const char*>(Handle_Get(sound->m_name)));
        }
    }

    int freeSlots = 0;
    for (int slot = 0; slot < SOUND_LOAD_SLOT_COUNT; ++slot)
    {
        if (g_soundLoadBusy[slot] == 0)
        {
            ++freeSlots;
        }
    }

    if (freeSlots == 0)
    {
        return;
    }

    for (int index = 0; index < count;)
    {
        LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(eventSystem->m_sounds[index]));
        char path[SOUND_ENTRY_PATH_SIZE];
        _MILESBANKSOUNDINFO info;
        if (sound->m_loadSlot == 0 && sound->m_data == NULL && sound->m_loadFailed == 0
            && Container_GetSound(static_cast<const char*>(Handle_Get(sound->m_name)), path, &info) != 0)
        {
            int slot = 0;
            while (slot < SOUND_LOAD_SLOT_COUNT && g_soundLoadBusy[slot] != 0)
            {
                ++slot;
            }

            MilesAsyncRead* request = &g_soundLoadRequests[slot];
            memset(request, 0, sizeof(MilesAsyncRead));
            strcpy(request->szFileName, path);
            request->iOwnerLine = BEGIN_ASYNC_OWNER_LINE;
            g_soundLoadOwners[slot] = eventSystem;
            request->pOwnerName = EVENT_EXEC_FILE;
            if (info.m_unbuffered == 0)
            {
                sound->m_data = AIL_mem_alloc_lock_info(info.m_dataSize, EVENT_EXEC_FILE, BEGIN_ASYNC_LOAD_LINE);
                if (sound->m_data == NULL)
                {
                    ErrorHandler(ERROR_TYPE_OUT_OF_MEMORY, 0, static_cast<const char*>(Handle_Get(sound->m_name)));
                    Sound_Free(eventSystem, eventSystem->m_sounds[index]);
                    --count;
                    continue;
                }

                sound->m_size = info.m_dataSize;
                request->pBuffer = sound->m_data;
                request->iLength = info.m_dataSize;
                request->iLengthHigh = info.m_dataSize >> 31;
            }
            else
            {
                request->iLengthHigh = -1;
                request->iLength = -1;
            }

            Audition_AsyncLoad(sound->m_auditionId, info.m_dataSize);
            g_soundLoadBusy[slot] = 1;
            request->pPriorityUserData = reinterpret_cast<void*>(sound->m_auditionId);
            sound->m_loadSlot = slot + 1;
            MilesAsyncFileRead(request);
            for (int earlier = 0; earlier < index; ++earlier)
            {
                Handle& candidate = eventSystem->m_sounds[earlier];
                if (static_cast<LoadedSound*>(Handle_Get(candidate))->m_loadSlot == 0)
                {
                    const Handle swapped = candidate;
                    candidate = eventSystem->m_sounds[index];
                    eventSystem->m_sounds[index] = swapped;
                    break;
                }
            }

            --freeSlots;
            if (freeSlots == 0)
            {
                return;
            }
        }

        ++index;
    }
}

static void ReloadAllSounds(EventSystem* eventSystem)
{
    const int instanceCount = eventSystem->m_instances.Count();
    for (int index = 0; index < instanceCount; ++index)
    {
        DestroySoundInstance(eventSystem,
                             reinterpret_cast<SoundInstance*>(Handle_Get(eventSystem->m_instances[index])));
    }

    PODHandleArray<Handle> sounds;
    sounds.Construct();
    eventSystem->m_sounds.Reacquire();
    sounds.PushCount(eventSystem->m_sounds.Data(), eventSystem->m_sounds.Count());
    const int soundCount = eventSystem->m_sounds.Count();
    for (int index = 0; index < soundCount; ++index)
    {
        LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(sounds[index]));
        const int references = sound->m_refCount;
        char name[SOUND_ENTRY_PATH_SIZE];
        strcpy(name, static_cast<const char*>(Handle_Get(sound->m_name)));
        for (int release = 0; release < references; ++release)
        {
            Sound_Free(eventSystem, sounds[index]);
        }

        for (int load = 0; load < references; ++load)
        {
            Sound_Load(eventSystem, 0, name);
        }
    }

    sounds.FreeMem();
}

static void FinishCompletedSounds(EventSystem* eventSystem)
{
    const int count = eventSystem->m_instances.Count();
    for (int index = 0; index < count; ++index)
    {
        SoundInstanceData* instance = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
        if (instance->m_state != SOUND_STATE_PLAYING || instance->m_paused != 0)
        {
            continue;
        }

        bool finished;
        if (instance->m_stream != NULL)
        {
            const int status = AIL_stream_status(instance->m_stream);
            finished = status == STREAM_DONE || status == SAMPLE_DONE;
        }
        else
        {
            finished = AIL_sample_status(instance->m_sample) == SAMPLE_DONE;
        }

        if (!finished)
        {
            continue;
        }

        const char* completionEvent = static_cast<const char*>(Handle_Get(instance->m_self));
        if (completionEvent != NULL)
        {
            const unsigned long long completion = AIL_enqueue_event_from_init_block(
                completionEvent, Handle_Get(instance->m_initBlock), instance->m_initBlockSize, instance->m_serial);
            Audition_CompletionEvent(completion, instance->m_eventId);
        }

        DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
    }
}

extern "C" int MilesBeginEventQueueProcessingInternal()
{
    if (g_eventSystemList == NULL)
    {
        AIL_set_error(NO_EVENT_SYSTEMS_ERROR);
        return 0;
    }

    Audition_FrameStart();
    for (int slot = 0; slot < SOUND_LOAD_SLOT_COUNT; ++slot)
    {
        if (g_soundLoadCancelled[slot] != 0 && MilesAsyncFileStatus(&g_soundLoadRequests[slot], 0) != 0)
        {
            Audition_AsyncCancel(static_cast<int>(reinterpret_cast<intptr_t>(g_soundLoadRequests[slot].pPriorityUserData)));
            AIL_mem_free_lock(g_soundLoadRequests[slot].pBuffer);
            g_soundLoadBusy[slot] = 0;
            g_soundLoadCancelled[slot] = 0;
        }
    }

    for (EventSystem* eventSystem = g_eventSystemList; eventSystem != NULL; eventSystem = eventSystem->m_next)
    {
        UpdateSoundLoads(eventSystem);
    }

    const unsigned long long queueStart = Queue_EnumCreate();
    unsigned long long queueCursor = queueStart;
    for (QueuedEvent* event = reinterpret_cast<QueuedEvent*>(Queue_EnumNext(&queueCursor, NULL)); event != NULL;
         event = reinterpret_cast<QueuedEvent*>(Queue_EnumNext(&queueCursor, reinterpret_cast< ::QueuedEvent*>(event))))
    {
        EventSystem* eventSystem = event->eventSystem;
        if (eventSystem == NULL)
        {
            eventSystem = g_eventSystemList;
        }

        const unsigned char* steps = static_cast<const unsigned char*>(event->event);
        const char* auditionName = NULL;
        bool fromBank = false;
        if ((event->flags & QUEUED_EVENT_NAMED) != 0)
        {
            auditionName = static_cast<const char*>(event->event);
            steps = Container_GetEvent(auditionName);
            fromBank = steps != NULL;
            if (!fromBank && Audition_Status() == 0)
            {
                ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, event->serial, auditionName);
            }
        }
        else if (steps != NULL && *steps == AUDITION_EVENT_PREFIX)
        {
            auditionName = reinterpret_cast<const char*>(steps + 1);
            steps = NULL;
        }

        if (auditionName != NULL)
        {
            Audition_Event(auditionName, event->serial, event->context, fromBank, event->data, event->dataSize);
        }

        const unsigned char* cursor = steps;
        while (cursor != NULL)
        {
            AIL_set_error(NULL);
            EVENT_STEP_INFO* step = NULL;
            char buffer[EVENT_QUEUE_SCRATCH_SIZE];
            const unsigned char* stepKey = cursor;
            cursor = AIL_next_event_step(cursor, &step, buffer, EVENT_QUEUE_SCRATCH_SIZE);
            if (step == NULL)
            {
                break;
            }

            switch (step->m_kind)
            {
            case EVENT_STEP_RESET:
                StepReset(eventSystem, event);
                break;
            case EVENT_STEP_ENVIRONMENT:
                StepEnvironment(step, eventSystem, event);
                break;
            case EVENT_STEP_CACHE:
                ExecuteCacheSounds(step, eventSystem, event->serial, 1);
                break;
            case EVENT_STEP_UNCACHE:
                ExecuteCacheSounds(step, eventSystem, event->serial, 0);
                break;
            case EVENT_STEP_PERSIST:
                if (step->m_data.m_persist.m_nameLength != 0)
                {
                    if (step->m_data.m_persist.m_presetLength == 0)
                    {
                        Persist_Free(eventSystem, event->serial, step->m_data.m_persist.m_name);
                    }
                    else
                    {
                        Persist_Create(eventSystem, event->serial, step);
                    }
                }

                break;
            case EVENT_STEP_LIMIT_SELECT:
                StepLimitSelect(step, eventSystem, event);
                break;
            case EVENT_STEP_CONTROL:
                StepControl(step, eventSystem, event);
                break;
            case EVENT_STEP_RAMP:
                StepRamp(step, eventSystem, event);
                break;
            case EVENT_STEP_PLAY:
                if ((step->m_data.m_play.m_modeFlags & MODE_MASK) == MODE_BLEND)
                {
                    PlayStepBlend(step, eventSystem, event);
                }
                else
                {
                    PlayStepSelect(step, eventSystem, event, stepKey);
                }

                break;
            case EVENT_STEP_ENQUEUE:
                if (step->m_data.m_name.m_nameLength != 0)
                {
                    AIL_enqueue_event_from_init_block(step->m_data.m_name.m_name, event->data, event->dataSize,
                                                      event->serial);
                }

                break;
            case EVENT_STEP_TWEEN:
                StepTween(step, eventSystem);
                break;
            default:
                break;
            }
        }

        if ((event->flags & QUEUED_EVENT_OWNS_EVENT) != 0)
        {
            AIL_mem_free_lock(const_cast<void*>(event->event));
            event->flags ^= ~QUEUED_EVENT_OWNS_EVENT;
        }
    }

    Queue_EnumClose(queueStart, queueCursor);
    if (Audition_Pump() != 0)
    {
        ReloadAllSounds(g_eventSystemList);
    }

    for (EventSystem* eventSystem = g_eventSystemList; eventSystem != NULL; eventSystem = eventSystem->m_next)
    {
        FinishCompletedSounds(eventSystem);
    }

    return 1;
}


const float BUS_GAIN_SCALE = 32767.0f;
const float BUS_GAIN_FLOOR = 1.0e-5f;
const float BUS_DECIBEL_FLOOR = -96.0f;
const double BUS_DECIBEL_FACTOR = 20.0;
const double BUS_RATE_TOLERANCE = 1.0e-5f;
const float BUS_DEFAULT_RATE = 96000.0f;
const float LFO_HALF_TURN = 3.1415f;
const float LFO_FULL_TURN = 6.28318f;
const float LFO_PERCENT = 100.0f;
const float TWEEN_CUBIC_SCALE = 4.0f;
const unsigned int MILLISECOND_DIVISOR = 1000;
const unsigned int MARKER_KIND_MASK = 7;
const int FRAME_TIME_LIMIT = 1000;
const int MAIN_BUS_INDEX = 0;
const int DAC_NAME_DIGIT = 10;
const int DECIBEL_NAME_DIGIT = 9;
const char MAIN_BUS_DAC_NAME[] = "_MainBusDAC";
const char MAIN_BUS_DECIBEL_NAME[] = "_MainBusDb";
const int BUS_NAME_BUFFER_SIZE = 12;
const int LFO_SINE = 0;
const int LFO_TRIANGLE = 1;
const int LFO_RAMP = 2;
const int LFO_SQUARE = 3;
const int TWEEN_CURVE_DECELERATE = 0;
const int TWEEN_CURVE_ACCELERATE = 1;
const int TWEEN_CURVE_SMOOTHSTEP = 2;
const int POSITION_BYTES_PER_UNIT = 2;
const int POSITION_BYTES_SHIFT = 3;
const int POSITION_PREFETCH_BYTES = 0x10000;
const unsigned char SOUND_FLAG_PENDING_DATA_MASK = 4;

static void SetBusVariables(EventSystem* eventSystem, const BusNode* bus, float gain)
{
    float value = gain;
    char dacName[BUS_NAME_BUFFER_SIZE] = "_AuxBusDAC0";
    dacName[DAC_NAME_DIGIT] = static_cast<char>(bus->m_index + '0');
    const char* variableName = dacName;
    if (bus->m_index == MAIN_BUS_INDEX)
    {
        variableName = MAIN_BUS_DAC_NAME;
    }

    Var_SetInstance(&eventSystem->m_variables, variableName, &value, 1);
    char decibelName[BUS_NAME_BUFFER_SIZE] = "_AuxBusDb0";
    decibelName[DECIBEL_NAME_DIGIT] = static_cast<char>(bus->m_index + '0');
    variableName = decibelName;
    if (bus->m_index == MAIN_BUS_INDEX)
    {
        variableName = MAIN_BUS_DECIBEL_NAME;
    }

    float decibels;
    if (gain < BUS_GAIN_FLOOR)
    {
        decibels = BUS_DECIBEL_FLOOR;
    }
    else
    {
        decibels = static_cast<float>(BUS_DECIBEL_FACTOR * log10(static_cast<double>(gain)));
    }

    Var_SetInstance(&eventSystem->m_variables, variableName, &decibels, 1);
}

static void ApplyBusPreset(EventSystem* eventSystem, BusNode* bus)
{
    bus->m_presetApplied = 1;
    const char* presetName = static_cast<const char*>(Handle_Get(bus->m_preset));
    if (*presetName == '\0')
    {
        return;
    }

    void* preset = Container_GetPreset(presetName);
    if (preset == NULL)
    {
        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, 0, presetName);
        bus->m_presetEnabled = 0;
        return;
    }

    g_eventSystem = eventSystem;
    AIL_resolve_raw_sound_preset(preset, NULL, EvalFunc);
    AIL_apply_raw_sound_preset(bus->m_sample, preset);
}

static void UpdateBuses(EventSystem* eventSystem)
{
    BusNode* bus = static_cast<BusNode*>(Handle_Get(eventSystem->m_buses));
    while (bus != NULL)
    {
        const float gain = static_cast<float>(bus->m_sample->peak) / BUS_GAIN_SCALE;
        SetBusVariables(eventSystem, bus, gain);
        if (bus->m_presetApplied == 0 || bus->m_presetEnabled != 0)
        {
            ApplyBusPreset(eventSystem, bus);
        }

        float levelA = 1.0f;
        float levelB = 1.0f;
        float rate = BUS_DEFAULT_RATE;
        Ramp* ramp = static_cast<Ramp*>(Handle_Get(eventSystem->m_ramps));
        while (ramp != NULL)
        {
            if (strncmp(ramp->m_labels, BUS_NAME_PREFIX, BUS_NAME_PREFIX_LENGTH) == 0
                && ramp->m_labels[BUS_NAME_PREFIX_LENGTH] - '0' == bus->m_index)
            {
                const float value = Ramp_GetCurrentValue(eventSystem, ramp, 1);
                switch (ramp->m_target)
                {
                case 0:
                    levelA = levelA * value;
                    break;
                case 1:
                    levelB = levelB * value;
                    break;
                case 2:
                    rate = value;
                    break;
                }
            }

            ramp = static_cast<Ramp*>(Handle_Get(ramp->m_next));
        }

        _SAMPLE* sample = bus->m_sample;
        if (BUS_RATE_TOLERANCE < fabs(static_cast<double>(sample->rate - rate)))
        {
            sample->cacheState = -1;
        }

        sample->levelA = levelA;
        sample->levelB = levelB;
        sample->rate = rate;
        bus = static_cast<BusNode*>(Handle_Get(bus->m_next));
    }
}

static void ReapplyEnvironment(EventSystem* eventSystem)
{
    if (eventSystem->m_environment.index == 0 || eventSystem->m_environmentActive == 0)
    {
        return;
    }

    void* preset = Container_GetEnvironment(static_cast<const char*>(Handle_Get(eventSystem->m_environment)));
    if (preset == NULL)
    {
        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, 0, static_cast<const char*>(Handle_Get(eventSystem->m_environment)));
        return;
    }

    g_eventSystem = eventSystem;
    AIL_resolve_raw_environment_preset(preset, EvalFunc);
    AIL_apply_raw_environment_preset(eventSystem->m_driver, preset);
}

static void UpdateLfos(EventSystem* eventSystem, float elapsed)
{
    LfoNode* lfo = static_cast<LfoNode*>(Handle_Get(eventSystem->m_lfos));
    while (lfo != NULL)
    {
        const char* frequencyText = static_cast<const char*>(Handle_Get(lfo->m_frequencyText));
        const char* centerText = static_cast<const char*>(Handle_Get(lfo->m_centerText));
        const char* amplitudeText = static_cast<const char*>(Handle_Get(lfo->m_amplitudeText));
        if (frequencyText != NULL)
        {
            Var_LookupWithContext(eventSystem, eventSystem->m_variables, frequencyText, 1, &lfo->m_frequency);
        }

        if (centerText != NULL)
        {
            Var_LookupWithContext(eventSystem, eventSystem->m_variables, centerText, 1, &lfo->m_center);
        }

        if (amplitudeText != NULL)
        {
            Var_LookupWithContext(eventSystem, eventSystem->m_variables, amplitudeText, 1, &lfo->m_amplitude);
        }

        const float period = 1.0f / lfo->m_frequency;
        float value = 0.0f;
        switch (lfo->m_mode)
        {
        case LFO_SINE:
            value = lfo->m_flip != 0 ? 1.0f
                                           + sinf(static_cast<float>(lfo->m_inverted) * LFO_HALF_TURN
                                                  + LFO_FULL_TURN * lfo->m_time / period)
                                     : 0.0f
                                           + sinf(static_cast<float>(lfo->m_inverted) * LFO_HALF_TURN
                                                  + LFO_FULL_TURN * lfo->m_time / period);
            break;
        case LFO_TRIANGLE:
            value = (lfo->m_time + lfo->m_time) / period;
            if (period * 0.5f < lfo->m_time)
            {
                value = 2.0f - value;
            }

            if (lfo->m_inverted != 0)
            {
                value = 1.0f - value;
            }

            if (lfo->m_flip == 0)
            {
                value = value - 0.5f;
            }

            break;
        case LFO_RAMP:
            value = lfo->m_time / period;
            if (lfo->m_inverted != 0)
            {
                value = 1.0f - value;
            }

            if (lfo->m_flip == 0)
            {
                value = value - 0.5f;
            }

            break;
        case LFO_SQUARE:
            value = LFO_PERCENT * lfo->m_time / period - static_cast<float>(lfo->m_offset) < 0.0f ? 1.0f : 0.0f;
            if (lfo->m_inverted != 0)
            {
                value = value != 0.0f ? 0.0f : 1.0f;
            }

            if (lfo->m_flip == 0)
            {
                value = value - 0.5f;
            }

            break;
        }

        value = value * lfo->m_amplitude + lfo->m_center;
        float time = lfo->m_time + elapsed;
        if (period <= time)
        {
            time = time - period;
        }

        lfo->m_time = time;
        Var_SetInstance(&eventSystem->m_variables, lfo->m_name, &value, 1);
        lfo = static_cast<LfoNode*>(Handle_Get(lfo->m_next));
    }
}

static float EvaluateTween(const TweenNode* tween, int segment)
{
    float time = tween->m_elapsed;
    float duration = tween->m_times[0];
    if (segment != 0)
    {
        time = time - duration;
        duration = tween->m_times[1] - duration;
    }

    const float fraction = time / duration;
    switch (tween->m_curves[segment])
    {
    default:
        return fraction * (tween->m_values[segment + 1] - tween->m_values[segment]) + tween->m_values[segment];
    case TWEEN_CURVE_DECELERATE:
        return (1.0f - (1.0f - fraction) * (1.0f - fraction))
                   * (tween->m_values[segment + 1] - tween->m_values[segment])
               + tween->m_values[segment];
    case TWEEN_CURVE_ACCELERATE:
        return fraction * fraction * (tween->m_values[segment + 1] - tween->m_values[segment])
               + tween->m_values[segment];
    case TWEEN_CURVE_SMOOTHSTEP:
    {
        const float offset = fraction - 0.5f;
        return (offset * offset * offset * TWEEN_CUBIC_SCALE + 0.5f)
                   * (tween->m_values[segment + 1] - tween->m_values[segment])
               + tween->m_values[segment];
    }
    }
}

static void UpdateTweens(EventSystem* eventSystem, float elapsed)
{
    TweenNode* previous = NULL;
    TweenNode* tween = static_cast<TweenNode*>(Handle_Get(eventSystem->m_tweens));
    while (tween != NULL)
    {
        const int segment = tween->m_elapsed < tween->m_times[0] ? 0 : 1;
        float value = EvaluateTween(tween, segment);
        Var_SetInstance(&eventSystem->m_variables, tween->m_name, &value, 1);
        TweenNode* next;
        if (tween->m_elapsed >= tween->m_times[1])
        {
            next = static_cast<TweenNode*>(Handle_Get(tween->m_next));
            if (previous != NULL)
            {
                previous->m_next = tween->m_next;
            }
            else
            {
                eventSystem->m_tweens = tween->m_next;
            }

            Handle_Free(tween->m_self);
        }
        else
        {
            tween->m_elapsed = static_cast<float>(tween->m_elapsed + elapsed);
            next = static_cast<TweenNode*>(Handle_Get(tween->m_next));
            previous = tween;
        }

        tween = next;
    }
}

static void AuditVariables(EventSystem* eventSystem)
{
    EventSystemVariable* variable = static_cast<EventSystemVariable*>(Handle_Get(eventSystem->m_variables));
    while (variable != NULL)
    {
        Audition_VarState(variable->m_name, 0, variable->m_isFloat == 0, &variable->m_value);
        variable = static_cast<EventSystemVariable*>(Handle_Get(variable->m_next));
    }
}

static void UpdateRamps(EventSystem* eventSystem, float elapsed, int auditing)
{
    Ramp* ramp = static_cast<Ramp*>(Handle_Get(eventSystem->m_ramps));
    while (ramp != NULL)
    {
        const float before = ramp->m_elapsed;
        ramp->m_elapsed = before + elapsed;
        if (ramp->m_duration < before + elapsed)
        {
            ramp->m_elapsed = ramp->m_duration;
        }

        if (auditing != 0)
        {
            Ramp_GetCurrentValue(eventSystem, ramp, 1);
            Audition_RampState(ramp->m_name, 0, ramp->m_target);
        }

        ramp = static_cast<Ramp*>(Handle_Get(ramp->m_next));
    }
}

static void AuditSoundVariables(SoundInstanceData* instance)
{
    EventSystemVariable* variable = static_cast<EventSystemVariable*>(Handle_Get(instance->m_context));
    while (variable != NULL)
    {
        Audition_VarState(variable->m_name, instance->m_eventId, variable->m_isFloat == 0, &variable->m_value);
        variable = static_cast<EventSystemVariable*>(Handle_Get(variable->m_next));
    }
}

static inline void FreeNodes(Handle head)
{
    EventListNode* node = static_cast<EventListNode*>(Handle_Get(head));
    while (node != NULL)
    {
        const Handle next = node->m_next;
        Handle_Free(node->m_self);
        node = static_cast<EventListNode*>(Handle_Get(next));
    }
}

static inline void ApplyInstancePreset(SoundInstanceData* instance)
{
    if (instance->m_preset.index == 0)
    {
        return;
    }

    const char* presetName = static_cast<const char*>(Handle_Get(instance->m_preset));
    void* preset = Container_GetPreset(presetName);
    if (preset == NULL)
    {
        ErrorHandler(ERROR_TYPE_SOUND_NOT_FOUND, instance->m_eventId, presetName);
        return;
    }

    AIL_resolve_raw_sound_preset(preset, instance, EvalFunc);
    AIL_apply_raw_sound_preset(instance->m_sample, preset);
}

static void ReleaseFinishedInstance(EventSystem* eventSystem, SoundInstanceData* instance, int index)
{
    Audition_SoundComplete(instance->m_eventId);
    Handle_Free(instance->m_initBlock);
    Handle_Free(instance->m_self);
    if (instance->m_externalSample != 0)
    {
        g_externalSampleRelease(instance->m_sample);
    }
    else if (instance->m_stream != NULL)
    {
        AIL_close_stream(instance->m_stream);
    }
    else
    {
        AIL_release_sample_handle(instance->m_sample);
    }

    FreeNodes(instance->m_ramps);
    FreeNodes(instance->m_context);
    LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(instance->m_source));
    if (sound != NULL)
    {
        --sound->m_playCount;
    }

    Handle_Free(instance->m_name);
    Handle_Free(instance->m_labels);
    Handle_Free(instance->m_preset);
    Sound_Free(eventSystem, instance->m_source);
    Handle_Free(eventSystem->m_instances[index]);
    eventSystem->m_instances.RemoveUnsorted(index);
}

static inline void FailInstance(EventSystem* eventSystem, SoundInstanceData* instance, unsigned char waitingFlag)
{
    DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
    instance->m_flags = (instance->m_flags ^ waitingFlag) | SOUND_INSTANCE_FLAG_FAILED;
    Audition_SoundFlags(instance->m_eventId, instance->m_flags);
}

static void UpdatePlayingInstance(EventSystem* eventSystem, SoundInstanceData* instance, float elapsed, int auditing)
{
    LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(instance->m_source));
    if (sound != NULL)
    {
        if (sound->m_loadFailed != 0)
        {
            FailInstance(eventSystem, instance, SOUND_FLAG_PENDING_DATA_MASK);
        }
        else if ((instance->m_flags & SOUND_FLAG_PENDING_DATA_MASK) != 0 && sound->m_loadSlot == 0)
        {
            instance->m_flags = instance->m_flags ^ SOUND_FLAG_PENDING_DATA_MASK;
            Audition_SoundFlags(instance->m_eventId, instance->m_flags);
        }
    }

    float blendVolume;
    float blendPitch;
    Blend_Apply(reinterpret_cast<SoundInstance*>(instance), &blendVolume, &blendPitch);
    ApplyInstancePreset(instance);
    const char* labels = static_cast<const char*>(Handle_Get(instance->m_labels));
    Persist_Apply(eventSystem, reinterpret_cast<SoundInstance*>(instance), instance->m_sample, labels, strlen(labels),
                  1);
    Ramp_ApplyAll(eventSystem, reinterpret_cast<SoundInstance*>(instance), elapsed);
    if (instance->m_fadeRate != 0.0f)
    {
        float fadeTime;
        if (instance->m_delayRemaining > 0.0f)
        {
            const float remaining = instance->m_delayRemaining - elapsed;
            if (remaining > 0.0f)
            {
                fadeTime = 0.0f;
                instance->m_delayRemaining = remaining;
            }
            else
            {
                fadeTime = -remaining;
                instance->m_delayRemaining = 0.0f;
            }
        }
        else
        {
            fadeTime = elapsed;
        }

        float volume = instance->m_fadeRate * fadeTime + instance->m_volume;
        if (volume > 1.0f)
        {
            instance->m_fadeRate = 0.0f;
            volume = 1.0f;
        }

        instance->m_volume = volume;
        if (volume <= 0.0f && instance->m_fadeRate < 0.0f)
        {
            instance->m_fadeRate = 0.0f;
            instance->m_volume = 0.0f;
            DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
        }
    }

    if (auditing != 0)
    {
        _SAMPLE* sample = instance->m_sample;
        Audition_SoundState(sample->volume, sample->distanceVolume, blendVolume, blendPitch, sample->levelA,
                            sample->levelB, sample->rate, sample->levelC, instance->m_eventId);
        AuditSoundVariables(instance);
    }
}

static void ApplyLoopPoints(SoundInstanceData* instance)
{
    const unsigned int startKind = instance->m_markerFlags & MARKER_KIND_MASK;
    const unsigned int endKind = (instance->m_markerFlags >> MARKER_FLAG_SHIFT) & MARKER_KIND_MASK;
    if (instance->m_markerFlags == 0)
    {
        return;
    }

    if (startKind == MARKER_TYPE_CONVERTED)
    {
        if (instance->m_stream == NULL)
        {
            if (endKind == MARKER_TYPE_CONVERTED)
            {
                AIL_set_sample_loop_samples(instance->m_sample, instance->m_loopStart, instance->m_loopEnd);
            }
            else
            {
                AIL_set_sample_loop_samples(instance->m_sample, instance->m_loopStart, LOOP_POINT_UNCHANGED);
                AIL_set_sample_loop_block(instance->m_sample, LOOP_POINT_UNCHANGED, instance->m_loopEnd);
            }
        }
    }
    else if (endKind == MARKER_TYPE_CONVERTED)
    {
        if (instance->m_stream == NULL)
        {
            AIL_set_sample_loop_samples(instance->m_sample, LOOP_POINT_UNCHANGED, instance->m_loopEnd);
            AIL_set_sample_loop_block(instance->m_sample, instance->m_loopStart, LOOP_POINT_UNCHANGED);
        }
    }
    else if (instance->m_stream != NULL)
    {
        AIL_set_stream_loop_block(instance->m_stream, instance->m_loopStart, instance->m_loopEnd);
    }
    else
    {
        AIL_set_sample_loop_block(instance->m_sample, instance->m_loopStart, instance->m_loopEnd);
    }
}

static void StartInstancePlayback(EventSystem* eventSystem, SoundInstanceData* instance)
{
    ApplyLoopPoints(instance);
    if (instance->m_startPosition != 0)
    {
        if (instance->m_startPositionType == 0)
        {
            if (instance->m_stream != NULL)
            {
                AIL_set_stream_position(instance->m_stream, instance->m_startPosition);
            }
            else
            {
                AIL_set_sample_position(instance->m_sample, instance->m_startPosition);
            }
        }
        else if (instance->m_stream != NULL)
        {
            AIL_set_stream_ms_position(instance->m_stream, instance->m_startPosition);
        }
        else
        {
            AIL_set_sample_ms_position(instance->m_sample, instance->m_startPosition);
        }
    }

    ApplyInstancePreset(instance);
    if (instance->m_paused == 0)
    {
        if (instance->m_stream != NULL)
        {
            AIL_pause_stream(instance->m_stream, 0);
        }
        else
        {
            AIL_resume_sample(instance->m_sample);
        }
    }

    Audition_SoundPlaying(instance->m_eventId);
    instance->m_state = SOUND_STATE_PLAYING;
}

static bool AttachSoundData(EventSystem* eventSystem, SoundInstanceData* instance)
{
    int position = instance->m_startPosition;
    if (instance->m_startPositionType != 0)
    {
        _MILESBANKSOUNDINFO info;
        Container_GetSound(static_cast<const char*>(Handle_Get(instance->m_name)), NULL, &info);
        const unsigned long long frames = static_cast<unsigned long long>(static_cast<unsigned int>(position))
                                          * static_cast<unsigned int>(info.m_sampleRate) / MILLISECOND_DIVISOR;
        position = (info.m_channels * static_cast<int>(frames) * POSITION_BYTES_PER_UNIT) >> POSITION_BYTES_SHIFT;
    }

    const int needed = position + POSITION_PREFETCH_BYTES;
    LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(instance->m_source));
    if (sound->m_loadFailed != 0)
    {
        FailInstance(eventSystem, instance, SOUND_FLAG_PENDING_DATA_MASK);
        return false;
    }

    const int slot = sound->m_loadSlot;
    if (slot == 0)
    {
        instance->m_flags = instance->m_flags ^ SOUND_FLAG_PENDING_DATA_MASK;
        Audition_SoundFlags(instance->m_eventId, instance->m_flags);
    }
    else if (g_soundLoadRequests[slot - 1].iBytesRead < needed)
    {
        return false;
    }

    const void* data = sound->m_data;
    unsigned int size = sound->m_size;
    if (slot != 0)
    {
        data = g_soundLoadRequests[slot - 1].pBuffer;
        size = g_soundLoadRequests[slot - 1].iLength;
    }

    if (sound->m_provider != 0)
    {
        AIL_set_sample_address(instance->m_sample, data, size);
        if (AIL_set_sample_processor(instance->m_sample, 0, sound->m_provider) == -1)
        {
            ErrorHandler(ERROR_TYPE_NO_PROVIDER, instance->m_eventId,
                         static_cast<const char*>(Handle_Get(sound->m_name)));
            DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
            return false;
        }
    }
    else
    {
        _AILSOUNDINFO wave;
        if (AIL_WAV_info(data, &wave) == 0)
        {
            ErrorHandler(ERROR_TYPE_NO_PROVIDER, instance->m_eventId,
                         static_cast<const char*>(Handle_Get(sound->m_name)));
            DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(instance));
            return false;
        }

        AIL_set_sample_address(instance->m_sample, wave.data, wave.dataLength);
    }

    return true;
}

static bool UpdatePendingInstance(EventSystem* eventSystem, SoundInstanceData* instance)
{
    const unsigned char flags = instance->m_flags;
    if ((flags & SOUND_INSTANCE_FLAG_UNNAMED) == 0 && instance->m_nameLength != 0)
    {
        if ((flags & SOUND_INSTANCE_FLAG_WAITING_FOR_STREAM) != 0)
        {
            LoadedSound* sound = static_cast<LoadedSound*>(Handle_Get(instance->m_source));
            if (sound->m_loadFailed != 0)
            {
                FailInstance(eventSystem, instance, SOUND_INSTANCE_FLAG_WAITING_FOR_STREAM);
                return false;
            }

            if (sound->m_loadSlot == 0)
            {
                return false;
            }

            instance->m_flags = (flags ^ SOUND_INSTANCE_FLAG_WAITING_FOR_STREAM) | SOUND_INSTANCE_FLAG_WAITING_FOR_DATA;
        }

        if ((instance->m_flags & SOUND_INSTANCE_FLAG_WAITING_FOR_DATA) != 0)
        {
            if (!AttachSoundData(eventSystem, instance))
            {
                return false;
            }
        }
    }

    StartInstancePlayback(eventSystem, instance);
    return true;
}

static void UpdateInstances(EventSystem* eventSystem, float elapsed, int auditing)
{
    int count = eventSystem->m_instances.Count();
    int index = 0;
    while (index < count)
    {
        SoundInstanceData* instance = static_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
        if (auditing != 0)
        {
            if (instance->m_sample->is3D != 0)
            {
                float x;
                float y;
                float z;
                AIL_sample_3D_position(instance->m_sample, &x, &y, &z);
                Audition_SoundPosition(x, y, z, instance->m_eventId);
            }

            Audition_SoundFlags(instance->m_eventId, instance->m_flags);
        }

        const int state = instance->m_state;
        if (state == SOUND_STATE_PENDING)
        {
            UpdatePendingInstance(eventSystem, instance);
        }
        else if (state == SOUND_STATE_FINISHED)
        {
            ReleaseFinishedInstance(eventSystem, instance, index);
            --count;
            continue;
        }
        else if (state == SOUND_STATE_PLAYING)
        {
            UpdatePlayingInstance(eventSystem, instance, elapsed, auditing);
        }

        ++index;
    }
}

extern "C" int MilesCompleteEventQueueProcessingInternal()
{
    if (g_eventSystemList == NULL)
    {
        return 1;
    }

    const int now = AIL_ms_count();
    float elapsed = 0.0f;
    if (now - g_lastFrameTime < FRAME_TIME_LIMIT)
    {
        elapsed = static_cast<float>(now - g_lastFrameTime) / MILLISECONDS_PER_SECOND;
    }

    g_lastFrameTime = now;
    Audition_Suppress(0);
    for (EventSystem* eventSystem = g_eventSystemList; eventSystem != NULL; eventSystem = eventSystem->m_next)
    {
        const int auditing = Audition_Status();
        if (auditing != 0)
        {
            float x;
            float y;
            float z;
            AIL_listener_3D_position(eventSystem->m_listener, &x, &y, &z);
            Audition_ListenerPosition(x, y, z);
            Audition_SendCPU(eventSystem->m_driver);
            Audition_UpdateDataCount(eventSystem->m_soundBytes);
            Audition_SendCount(eventSystem->m_instances.Count());
            int firstStat = 0;
            int secondStat = 0;
            Handle_MemStats(&firstStat, &secondStat);
            Audition_HandleSystemLoad(secondStat, firstStat);
        }

        UpdateBuses(eventSystem);
        ReapplyEnvironment(eventSystem);
        UpdateLfos(eventSystem, elapsed);
        UpdateTweens(eventSystem, elapsed);
        if (auditing != 0)
        {
            AuditVariables(eventSystem);
        }

        UpdateRamps(eventSystem, elapsed, auditing);
        UpdateInstances(eventSystem, elapsed, auditing);
        Audition_Suppress(1);
    }

    Audition_Suppress(0);
    Audition_DefragStart();
    Handle_Defrag();
    for (EventSystem* eventSystem = g_eventSystemList; eventSystem != NULL; eventSystem = eventSystem->m_next)
    {
        eventSystem->m_instances.Reacquire();
        eventSystem->m_sounds.Reacquire();
        eventSystem->m_stateVars.Reacquire();
    }

    Audition_FrameEnd();
    return 1;
}
