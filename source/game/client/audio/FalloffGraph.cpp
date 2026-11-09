#include "game/client/audio/FalloffGraph.hpp"
#include "game/client/audio/EventSystem.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/core/PODHandleArray.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesEventError.hpp"
#include "vendor/MilesMem.hpp"
#include "vendor/MilesRR.hpp"

#include <math.h>
#include <stddef.h>
#include <string.h>

extern "C" void AIL_mem_free_lock(void* pBlock);
extern "C" int AIL_ms_count();
extern "C" void AIL_end_sample(_SAMPLE* sample);
extern "C" void AIL_set_sample_volume_levels(_SAMPLE* sample, float left, float right);
extern "C" void AIL_set_sample_playback_rate_factor(_SAMPLE* sample, float factor);

namespace
{
const int QUEUE_RECORD_ALIGNMENT_MASK = 7;
const int QUEUE_HANDLE_SHIFT = 16;
const int QUEUE_TOKEN_MASK = 0xffff;
const int QUEUE_TOKEN_LIMIT = 8;
const int QUEUE_BUFFER_RECORD_HEADER_SIZE = 0x10;
#ifndef MINECRAFT_PC
const int QUEUED_EVENT_HEADER_SIZE = 0x30;
#endif
const int QUEUED_EVENT_PADDING = 0xf;
const unsigned int QUEUE_USED_MASK = 0x7fffffff;
const unsigned long long QUEUE_HEAD_MASK = 0xffffffff00000000ull;
const int BLEND_NO_INDEX = -1;
const float BLEND_VOLUME_CURVE_EXPONENT = 1.6666666f;
const float SOUND_VALUE_EPSILON = 0.01f;
const float SEMITONES_PER_OCTAVE = 12.0f;
const int SOUND_STATE_DESTROYED = 4;
const unsigned char SOUND_FLAG_EVICTED = 2;
const unsigned int QUEUE_FLAG_EVENT_OWNED = 2;
const unsigned int QUEUE_FLAG_NAMED = 4;
const unsigned int QUEUE_FLAG_MARKER = 8;
const float QUIETEST_START_VOLUME = 100.0f;
const ErrorType ERROR_TYPE_ENQUEUE_BUFFER_FULL = static_cast<ErrorType>(10);

enum QueueRecordTag
{
    QUEUE_RECORD_END = 0,
    QUEUE_RECORD_BUFFER = 3,
    QUEUE_RECORD_CONTEXT = 5,
    QUEUE_RECORD_NAME = 6,
    QUEUE_RECORD_EVENT = 7,
    QUEUE_RECORD_FILTER = 8,
    QUEUE_RECORD_SELECTION = 9,
    QUEUE_RECORD_MARKER = 10
};

enum EvictPolicy
{
    EVICT_LOWER_PRIORITY = 0,
    EVICT_FARTHEST = 1,
    EVICT_QUIETEST = 2,
    EVICT_OLDEST = 3
};
}

struct QueueRecord
{
    unsigned int tag;
    unsigned int size;
};

struct QueueBufferRecord
{
    QueueRecord header;
    int isValue;
    int dataSize;
    unsigned char data[1];
};

struct QueueNameRecord
{
    QueueRecord header;
    char name[1];
};

struct QueueEventRecord
{
    QueueRecord header;
    unsigned int ownsEvent;
    const void* event;
};

struct QueueFilterRecord
{
    QueueRecord header;
    unsigned int filter;
};

struct QueueIdRecord
{
    QueueRecord header;
    unsigned long long value;
};

struct QueuedEvent
{
    unsigned int committedSize;
    unsigned int size;
    const void* event;
#if __SIZEOF_POINTER__ == 8
    uintptr_t filter;
#else
    unsigned int filter;
#endif
    unsigned long long serial;
    unsigned long long context;
    unsigned long long selection;
    unsigned int flags;
    unsigned int dataSize;
    unsigned char data[1];
};

#ifdef MINECRAFT_PC
const int QUEUED_EVENT_HEADER_SIZE = offsetof(QueuedEvent, data);
#endif

struct BlendNode
{
    Handle next;
    Handle self;
    float rangeStart[MAX_BLEND_POINTS];
    float fadeInEnd[MAX_BLEND_POINTS];
    float fadeOutStart[MAX_BLEND_POINTS];
    float rangeEnd[MAX_BLEND_POINTS];
    float pitchAtStart[MAX_BLEND_POINTS];
    float pitchAtEnd[MAX_BLEND_POINTS];
    char name[1];
};

struct SoundLimitList
{
    unsigned char reserved0[0x10];
    Handle entries;
};

struct SoundLimitEntry
{
    Handle next;
    unsigned char reserved0[8];
    short limit;
    short count;
    char query[1];
};

struct SoundEntry
{
    Handle name;
    unsigned int provider;
    void* data;
    unsigned int nameLength;
};

static Handle sQueueStorage;
static unsigned int sQueueCapacity;
static volatile unsigned int sQueueTokens;
static QueueBuffer sQueueBuffers[QUEUE_BUFFER_COUNT];
static volatile unsigned long long sQueueRange = 0;
static volatile long long sEventSerial = 1;

static const char MILES_EVENT_SUPPORT_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mileseventsupport.cpp";

void Queue_SetMem(int capacity, Handle storage)
{
    sQueueCapacity = capacity;
    sQueueStorage = storage;
}

QueuedEvent* Queue_EnumNext(unsigned long long* cursor, QueuedEvent* event)
{
    if (event != NULL)
    {
        memset(event, 0, event->committedSize);
    }

    unsigned int position = static_cast<unsigned int>(*cursor >> 32);
    unsigned int remaining = static_cast<unsigned int>(*cursor);
    if (position >= sQueueCapacity)
    {
        position -= sQueueCapacity;
    }

    unsigned char* storage = static_cast<unsigned char*>(Handle_Get(sQueueStorage));
    while (remaining != 0)
    {
        QueuedEvent* queued = reinterpret_cast<QueuedEvent*>(storage + position);
        const unsigned int size = queued->committedSize;
        if (size == 0)
        {
            return NULL;
        }

        unsigned int end = position + size;
        if (position >= sQueueCapacity || end < sQueueCapacity)
        {
            remaining -= size;
            if (end >= sQueueCapacity)
            {
                end -= sQueueCapacity;
            }
            *cursor = (static_cast<unsigned long long>(end) << 32) | remaining;
            return queued;
        }

        remaining -= size;
        position = end - sQueueCapacity;
    }
    return NULL;
}

void Queue_EnumClose(unsigned long long start, unsigned long long end)
{
    const unsigned int consumed = static_cast<unsigned int>(start) - static_cast<unsigned int>(end);
    unsigned long long previous = rrAtomicLoadAcquire64(&sQueueRange);
    unsigned long long snapshot;
    do
    {
        snapshot = previous;
        previous = rrAtomicCmpXchg64(&sQueueRange,
                                     ((end >> 32) << 32) | (static_cast<unsigned int>(snapshot) - consumed), snapshot);
    } while (previous != snapshot);
}

unsigned long long Queue_EnumCreate()
{
    return rrAtomicLoadAcquire64(&sQueueRange);
}

QueuedEvent* Queue_Add(int size)
{
    const int allocation = (size + QUEUED_EVENT_HEADER_SIZE + QUEUED_EVENT_PADDING) & ~QUEUE_RECORD_ALIGNMENT_MASK;
    for (;;)
    {
        unsigned long long previous = rrAtomicLoadAcquire64(&sQueueRange);
        unsigned long long snapshot;
        unsigned long long updated;
        do
        {
            snapshot = previous;
            updated = (((snapshot >> 32) << 32) | (static_cast<unsigned int>(snapshot) & QUEUE_USED_MASK)) + allocation;
            if (static_cast<unsigned int>(updated) >= sQueueCapacity)
            {
                ErrorHandler(ERROR_TYPE_COMMAND_BUFFER_FULL, 0, 0);
                return NULL;
            }
            previous = rrAtomicCmpXchg64(&sQueueRange, updated, snapshot);
        } while (previous != snapshot);

        const int end = static_cast<int>(updated >> 32) + static_cast<int>(updated);
        const int capacity = sQueueCapacity;
        int start = end - allocation;
        if (start < capacity)
        {
            if (end >= capacity)
            {
                unsigned char* filler = static_cast<unsigned char*>(Handle_Get(sQueueStorage));
                *reinterpret_cast<int*>(filler + start) = allocation;
                continue;
            }
        }
        else
        {
            start -= capacity;
        }

        unsigned char* storage = static_cast<unsigned char*>(Handle_Get(sQueueStorage));
        QueuedEvent* event = reinterpret_cast<QueuedEvent*>(storage + start);
        event->size = size;
        return event;
    }
}

void Queue_Commit(QueuedEvent* event)
{
    rrAtomicStoreRelease32(&event->committedSize, (event->size + QUEUED_EVENT_HEADER_SIZE + QUEUED_EVENT_PADDING)
                                                      & ~QUEUE_RECORD_ALIGNMENT_MASK);
}

extern "C" unsigned int AIL_enqueue_event_start()
{
    for (;;)
    {
        unsigned int tokens = rrAtomicLoadAcquire32(&sQueueTokens);
        for (;;)
        {
            unsigned int previous = 0;
            unsigned int index;
            for (index = 0; index < QUEUE_BUFFER_COUNT; ++index)
            {
                if ((tokens & (1u << index)) == 0)
                {
                    previous = rrAtomicCmpXchg32(&sQueueTokens, tokens | (1u << index), tokens);
                    break;
                }
            }
            if (index == QUEUE_BUFFER_COUNT)
            {
                break;
            }
            if (previous == tokens)
            {
                return index + 1;
            }
            tokens = previous;
        }
        rrThreadSleep(0);
    }
}

void ClearQueueToken(int token)
{
    const unsigned int mask = 1u << token;
    for (;;)
    {
        const unsigned int current = rrAtomicLoadAcquire32(&sQueueTokens);
        if (rrAtomicCmpXchg32(&sQueueTokens, current & ~mask, current) == current)
        {
            break;
        }
        rrThreadSleep(0);
    }
}

extern "C" unsigned int AIL_enqueue_event_buffer(unsigned int* puiQueue, void* pData, int iDataSize, int iFlags)
{
    int size = iDataSize;
    if (iFlags != 0)
    {
        size = 4;
    }

    unsigned int result = 0;
    if (puiQueue != NULL)
    {
        const int offset = static_cast<int>(*puiQueue) >> QUEUE_HANDLE_SHIFT;
        int token = *puiQueue & QUEUE_TOKEN_MASK;
        if (token == 0 || token >= QUEUE_TOKEN_LIMIT)
        {
            *puiQueue = 0;
            return 0;
        }
        --token;

        QueueBuffer& buffer = sQueueBuffers[token];
        QueueBufferRecord* record = reinterpret_cast<QueueBufferRecord*>(buffer.bytes + offset);
        const unsigned int recordSize = (size + QUEUE_BUFFER_RECORD_HEADER_SIZE + QUEUE_RECORD_ALIGNMENT_MASK)
                                        & ~QUEUE_RECORD_ALIGNMENT_MASK;
        if (reinterpret_cast<unsigned char*>(record) + recordSize + 4 >= buffer.bytes + QUEUE_BUFFER_SIZE)
        {
            ErrorHandler(ERROR_TYPE_ENQUEUE_BUFFER_FULL, 0, 0);
            ClearQueueToken(token);
            *puiQueue = 0;
            return 0;
        }

        record->header.size = recordSize;
        record->header.tag = QUEUE_RECORD_BUFFER;
        memset(&record->isValue, 0, recordSize - sizeof(QueueRecord));
        record->isValue = iFlags;
        record->dataSize = size;
        if (iFlags != 0)
        {
            memcpy(record->data, &pData, size);
        }
        else
        {
            memcpy(record->data, pData, size);
        }
        *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(record) + recordSize) = QUEUE_RECORD_END;
        result = 1;
        *puiQueue = (token + 1) | (offset + recordSize) << QUEUE_HANDLE_SHIFT;
    }
    return result;
}

unsigned long long AIL_enqueue_event_from_init_block(const char* name, void* block, int size,
                                                     unsigned long long selection)
{
    unsigned int filter = 0;
    unsigned long long context = 0;
    bool marker = false;
    QueueRecord* nameRecord = NULL;

    const unsigned char* const end = static_cast<unsigned char*>(block) + size;
    QueueRecord* scan = static_cast<QueueRecord*>(block);
    while (scan->tag != QUEUE_RECORD_END)
    {
        const unsigned int recordSize = scan->size;
        switch (scan->tag)
        {
        case QUEUE_RECORD_NAME:
            nameRecord = scan;
            break;
        case QUEUE_RECORD_CONTEXT:
            context = reinterpret_cast<QueueIdRecord*>(scan)->value;
            break;
        case QUEUE_RECORD_FILTER:
            filter = reinterpret_cast<QueueFilterRecord*>(scan)->filter;
            break;
        case QUEUE_RECORD_MARKER:
            marker = true;
            break;
        }

        scan = reinterpret_cast<QueueRecord*>(reinterpret_cast<unsigned char*>(scan) + recordSize);
        if (reinterpret_cast<unsigned char*>(scan) >= end)
        {
            break;
        }
    }

    if (nameRecord == NULL)
    {
        return 0;
    }

    const int prefixSize = reinterpret_cast<unsigned char*>(nameRecord) - static_cast<unsigned char*>(block);
    const unsigned int nameRecordSize = (sizeof(QueueRecord) + strlen(name) + 1 + QUEUE_RECORD_ALIGNMENT_MASK)
                                        & ~QUEUE_RECORD_ALIGNMENT_MASK;
    QueuedEvent* queued = Queue_Add(nameRecordSize + prefixSize);
    if (queued == NULL)
    {
        return 0;
    }

    memcpy(queued->data, block, prefixSize);
    QueueNameRecord* copied = reinterpret_cast<QueueNameRecord*>(queued->data + prefixSize);
    copied->header.tag = QUEUE_RECORD_NAME;
    copied->header.size = nameRecordSize;
    strcpy(copied->name, name);
    const unsigned long long serial = rrAtomicAddExchange64(&sEventSerial, 1);
    queued->dataSize = size;
    queued->selection = selection;
    queued->flags = (marker ? QUEUE_FLAG_MARKER : 0) | QUEUE_FLAG_NAMED;
    queued->filter = filter;
    queued->serial = serial;
    queued->event = copied->name;
    queued->context = context;
    Queue_Commit(queued);
    return serial;
}

unsigned long long AIL_enqueue_event_end(int queue, void* event, int named, int ownsEvent)
{
    const int offset = queue >> QUEUE_HANDLE_SHIFT;
    if (named != 0)
    {
        const size_t length = strlen(static_cast<const char*>(event));
        int token = queue & QUEUE_TOKEN_MASK;
        if (token == 0 || token >= QUEUE_TOKEN_LIMIT)
        {
            return 0;
        }
        {
            --token;
            QueueBuffer& buffer = sQueueBuffers[token];
            QueueNameRecord* record = reinterpret_cast<QueueNameRecord*>(buffer.bytes + offset);
            const unsigned int recordSize = (sizeof(QueueRecord) + length + 1 + QUEUE_RECORD_ALIGNMENT_MASK)
                                            & ~QUEUE_RECORD_ALIGNMENT_MASK;
            if (reinterpret_cast<unsigned char*>(record) + recordSize + sizeof(unsigned int)
                >= buffer.bytes + QUEUE_BUFFER_SIZE)
            {
                ErrorHandler(ERROR_TYPE_ENQUEUE_BUFFER_FULL, 0, 0);
                ClearQueueToken(token);
                queue = 0;
            }
            else
            {
                record->header.size = recordSize;
                record->header.tag = QUEUE_RECORD_NAME;
                memset(record->name, 0, recordSize - sizeof(QueueRecord));
                strcpy(record->name, static_cast<const char*>(event));
                queue = (token + 1) | (offset + recordSize) << QUEUE_HANDLE_SHIFT;
                *reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(record)
                                                 + recordSize) = QUEUE_RECORD_END;
            }
        }
    }
    else
    {
        int token = queue & QUEUE_TOKEN_MASK;
        if (token == 0 || token >= QUEUE_TOKEN_LIMIT)
        {
            if (ownsEvent != 0)
            {
                AIL_mem_free_lock(event);
            }
            return 0;
        }
        {
            --token;
            QueueBuffer& buffer = sQueueBuffers[token];
            QueueEventRecord* record = reinterpret_cast<QueueEventRecord*>(buffer.bytes + offset);
            if (reinterpret_cast<unsigned char*>(record) + sizeof(QueueEventRecord) + sizeof(unsigned int)
                >= buffer.bytes + QUEUE_BUFFER_SIZE)
            {
                ErrorHandler(ERROR_TYPE_ENQUEUE_BUFFER_FULL, 0, 0);
                ClearQueueToken(token);
                queue = 0;
            }
            else
            {
                record->header.tag = QUEUE_RECORD_EVENT;
                record->header.size = sizeof(QueueEventRecord);
                memset(&record->ownsEvent, 0, sizeof(QueueEventRecord) - sizeof(QueueRecord));
                record->ownsEvent = ownsEvent != 0;
                queue = (token + 1) | (offset + sizeof(QueueEventRecord)) << QUEUE_HANDLE_SHIFT;
                record->event = event;
                *reinterpret_cast<unsigned int*>(record + 1) = QUEUE_RECORD_END;
            }
        }
    }

    const int usedSize = queue >> QUEUE_HANDLE_SHIFT;
    int token = queue & QUEUE_TOKEN_MASK;
    if (token != 0 && token < QUEUE_TOKEN_LIMIT)
    {
        --token;
        QueueBuffer& buffer = sQueueBuffers[token];
        unsigned long long context = 0;
        unsigned long long selection = 0;
        unsigned int filter = 0;
        bool marker = false;

        const unsigned char* const end = buffer.bytes + QUEUE_BUFFER_SIZE;
        const QueueRecord* scan = reinterpret_cast<const QueueRecord*>(buffer.bytes);
        while (scan->tag != QUEUE_RECORD_END)
        {
            const unsigned int recordSize = scan->size;
            switch (scan->tag)
            {
            case QUEUE_RECORD_CONTEXT:
                context = reinterpret_cast<const QueueIdRecord*>(scan)->value;
                break;
            case QUEUE_RECORD_FILTER:
                filter = reinterpret_cast<const QueueFilterRecord*>(scan)->filter;
                break;
            case QUEUE_RECORD_SELECTION:
                selection = reinterpret_cast<const QueueIdRecord*>(scan)->value;
                break;
            case QUEUE_RECORD_MARKER:
                marker = true;
                break;
            }

            scan = reinterpret_cast<const QueueRecord*>(reinterpret_cast<const unsigned char*>(scan) + recordSize);
            if (reinterpret_cast<const unsigned char*>(scan) >= end)
            {
                break;
            }
        }

        QueuedEvent* queued = Queue_Add(usedSize);
        if (queued != NULL)
        {
            memcpy(queued->data, buffer.bytes, usedSize);
            const unsigned long long serial = rrAtomicAddExchange64(&sEventSerial, 1);
            queued->serial = serial;
            queued->context = context;
            queued->flags = (named ? QUEUE_FLAG_NAMED : 0) | (marker ? QUEUE_FLAG_MARKER : 0);
            queued->filter = filter;
            if (selection != 0)
            {
                queued->selection = selection;
            }
            else
            {
                queued->selection = serial;
            }
            queued->dataSize = usedSize;

            if (named != 0)
            {
                queued->event = queued->data + offset + sizeof(QueueRecord);
                Queue_Commit(queued);
                ClearQueueToken(token);
            }
            else
            {
                const QueueEventRecord* record = reinterpret_cast<const QueueEventRecord*>(buffer.bytes + offset);
                queued->event = record->event;
                queued->flags |= record->ownsEvent ? QUEUE_FLAG_EVENT_OWNED : 0;
                Queue_Commit(queued);
                ClearQueueToken(token);
            }
            return serial;
        }
        ClearQueueToken(token);
    }

    if (named == 0 && ownsEvent != 0)
    {
        AIL_mem_free_lock(event);
    }
    return 0;
}

extern "C" void AIL_enqueue_event_end_named(unsigned int uiQueue, const char* pEventName)
{
    AIL_enqueue_event_end(uiQueue, const_cast<char*>(pEventName), 1, 0);
}

extern "C" unsigned long long MilesEnqueueEventByNameInternal(const char* pEventName)
{
    unsigned long long eventId = 0;
    if (pEventName != NULL)
    {
        const unsigned int queue = AIL_enqueue_event_start();
        eventId = AIL_enqueue_event_end(queue, const_cast<char*>(pEventName), 1, 0);
    }
    return eventId;
}

int StateVar_KeyLB(StateVar* vars, int count, void* key)
{
    StateVar* const first = vars;
    while (count != 0)
    {
        const int half = count / 2;
        void* const candidate = vars[half].key;
        if (candidate == key)
        {
            return (vars - first) + half;
        }
        if (candidate < key)
        {
            vars += half + 1;
            count -= half + 1;
        }
        else
        {
            count = half;
        }
    }
    return vars - first;
}

int StateVar_Lookup(EventSystem* eventSystem, void* key)
{
    StateVar* vars = eventSystem->m_stateVars.Data();
    const int count = eventSystem->m_stateVars.Count();
    const int index = StateVar_KeyLB(vars, count, key);
    if (index < count && vars[index].key == key)
    {
        return vars[index].value;
    }
    return 0;
}

void StateVar_Set(EventSystem* eventSystem, void* key, int value)
{
    StateVar* const vars = eventSystem->m_stateVars.Data();
    const int count = eventSystem->m_stateVars.Count();
    const int index = StateVar_KeyLB(vars, count, key);
    if (index >= count || vars[index].key != key)
    {
        StateVar entry;
        entry.key = key;
        entry.value = value;
        eventSystem->m_stateVars.Insert(index, entry);
    }
    else
    {
        vars[index].value = value;
    }
}

int Blend_Exists(EventSystem* eventSystem, const char* name)
{
    BlendNode* blend = static_cast<BlendNode*>(Handle_Get(eventSystem->m_blends));
    while (blend != NULL)
    {
        if (AIL_stricmp(blend->name, name) == 0)
        {
            return 1;
        }
        blend = static_cast<BlendNode*>(Handle_Get(blend->next));
    }
    return 0;
}

void Blend_Apply(SoundInstance* instance, float* volume, float* pitch)
{
    SoundInstanceData* sound = reinterpret_cast<SoundInstanceData*>(instance);

    *volume = 1.0f;
    *pitch = 1.0f;
    if (sound->m_blendIndex == BLEND_NO_INDEX)
    {
        return;
    }

    BlendNode* blend = static_cast<BlendNode*>(Handle_Get(sound->m_blend));
    float value = 0.0f;
    Var_LookupWithContext(sound->m_eventSystem, sound->m_context, blend->name, 1, &value);

    if (fabs(static_cast<double>(blend->rangeEnd[sound->m_blendIndex] - blend->rangeStart[sound->m_blendIndex]))
        <= SOUND_VALUE_EPSILON)
    {
        return;
    }

    float level;
    if (value < blend->fadeInEnd[sound->m_blendIndex])
    {
        level = (value - blend->rangeStart[sound->m_blendIndex])
                / (blend->fadeInEnd[sound->m_blendIndex] - blend->rangeStart[sound->m_blendIndex]);
        if (level >= 0.0f)
        {
            if (1.0f < level)
            {
                level = 1.0f;
            }
        }
        else
        {
            level = 0.0f;
        }
    }
    else if (value > blend->fadeOutStart[sound->m_blendIndex])
    {
        level = 1.0f
                - (value - blend->fadeOutStart[sound->m_blendIndex])
                      / (blend->rangeEnd[sound->m_blendIndex] - blend->fadeOutStart[sound->m_blendIndex]);
        if (level >= 0.0f)
        {
            if (1.0f < level)
            {
                level = 1.0f;
            }
        }
        else
        {
            level = 0.0f;
        }
    }
    else
    {
        level = 1.0f;
    }

    const float attenuation = static_cast<float>(pow(level, BLEND_VOLUME_CURVE_EXPONENT));
    *volume = attenuation;
    AIL_set_sample_volume_levels(sound->m_sample, attenuation, attenuation);

    if (attenuation > 0.0f)
    {
        if (fabs(static_cast<double>(blend->pitchAtStart[sound->m_blendIndex] - blend->pitchAtEnd[sound->m_blendIndex]))
            > SOUND_VALUE_EPSILON)
        {
            const float lower = static_cast<float>(
                pow(2.0, blend->pitchAtStart[sound->m_blendIndex] / SEMITONES_PER_OCTAVE));
            const float upper = static_cast<float>(
                pow(2.0, blend->pitchAtEnd[sound->m_blendIndex] / SEMITONES_PER_OCTAVE));
            *pitch = ((value - blend->rangeStart[sound->m_blendIndex])
                      / (blend->rangeEnd[sound->m_blendIndex] - blend->rangeStart[sound->m_blendIndex]))
                         * (upper - lower)
                     + lower;
            AIL_set_sample_playback_rate_factor(sound->m_sample, *pitch);
        }
    }
}

int Blend_Add(EventSystem* eventSystem, const EVENT_STEP_INFO* step)
{
    BlendNode* blend = static_cast<BlendNode*>(Handle_Get(eventSystem->m_blends));
    while (blend != NULL)
    {
        if (AIL_stricmp(blend->name, step->m_data.m_blend.m_name) == 0)
        {
            break;
        }
        blend = static_cast<BlendNode*>(Handle_Get(blend->next));
    }

    if (blend == NULL)
    {
        const Handle allocated = Handle_AllocateInfo(sizeof(BlendNode) + step->m_data.m_blend.m_nameLength,
                                                     MILES_EVENT_SUPPORT_FILE, 174);
        blend = static_cast<BlendNode*>(Handle_Get(allocated));
        if (blend == NULL)
        {
            return 0;
        }
        memcpy(blend->name, step->m_data.m_blend.m_name, step->m_data.m_blend.m_nameLength + 1);
        blend->self = allocated;
        blend->next = eventSystem->m_blends;
        eventSystem->m_blends = allocated;
    }

    const int byteCount = step->m_data.m_blend.m_pointCount * sizeof(float);
    memcpy(blend->fadeInEnd, step->m_data.m_blend.m_fadeInEnd, byteCount);
    memcpy(blend->rangeStart, step->m_data.m_blend.m_rangeStart, byteCount);
    memcpy(blend->rangeEnd, step->m_data.m_blend.m_rangeEnd, byteCount);
    memcpy(blend->fadeOutStart, step->m_data.m_blend.m_fadeOutStart, byteCount);
    memcpy(blend->pitchAtStart, step->m_data.m_blend.m_pitchAtStart, byteCount);
    memcpy(blend->pitchAtEnd, step->m_data.m_blend.m_pitchAtEnd, byteCount);
    return 1;
}

int DoLabelsMatchQuery(const char* labels, const char* query)
{
    if (*query == '\0')
    {
        return 1;
    }

    int requiredFound = 0;
    int optionalFound = 0;
    for (;;)
    {
        int termLength = 0;
        while (query[termLength] != ',' && query[termLength] != '\0')
        {
            ++termLength;
        }

        int sign = 0;
        if (*query == '-')
        {
            sign = -1;
            ++query;
            --termLength;
        }
        else if (*query == '+')
        {
            --termLength;
            ++query;
            sign = 1;
        }

        const char* label = labels;
        for (;;)
        {
            int labelLength = 0;
            while (label[labelLength] != '\0' && label[labelLength] != ',')
            {
                ++labelLength;
            }
            if (labelLength == 0)
            {
                break;
            }
            if (labelLength != termLength || AIL_strnicmp(label, query, termLength) != 0)
            {
                label += labelLength;
                if (*label == '\0')
                {
                    break;
                }
                ++label;
            }
            else
            {
                if (sign == -1)
                {
                    return 0;
                }
                label = NULL;
                break;
            }
        }

        if (sign == 1)
        {
            if (label != NULL)
            {
                return 0;
            }
            ++requiredFound;
        }
        else if (sign == 0 && label == NULL)
        {
            ++optionalFound;
        }

        query += termLength;
        if (*query == '\0')
        {
            break;
        }
        ++query;
    }

    if (requiredFound != 0 || optionalFound != 0)
    {
        return 1;
    }
    return 0;
}

Handle Sound_Find(EventSystem* eventSystem, const char* name)
{
    const size_t length = strlen(name);
    int count = eventSystem->m_sounds.Count();
    Handle* handles = eventSystem->m_sounds.Data();
    while (count != 0)
    {
        SoundEntry* sound = static_cast<SoundEntry*>(Handle_Get(*handles));
        if (sound->nameLength == length)
        {
            if (AIL_stricmp(static_cast<const char*>(Handle_Get(sound->name)), name) == 0)
            {
                return *handles;
            }
        }
        --count;
        ++handles;
    }
    return Handle();
}

void DestroySoundInstance(EventSystem* eventSystem, SoundInstance* instance)
{
    SoundInstanceData* sound = reinterpret_cast<SoundInstanceData*>(instance);
    if (sound->m_state == SOUND_STATE_DESTROYED)
    {
        return;
    }

    AIL_end_sample(sound->m_sample);
    sound->m_state = SOUND_STATE_DESTROYED;
    const char* labels = static_cast<const char*>(Handle_Get(sound->m_labels));

    SoundLimitList* limits = static_cast<SoundLimitList*>(Handle_Get(eventSystem->m_limits));
    if (limits != NULL)
    {
        SoundLimitEntry* entry = static_cast<SoundLimitEntry*>(Handle_Get(limits->entries));
        while (entry != NULL)
        {
            if (DoLabelsMatchQuery(labels, entry->query) != 0)
            {
                --entry->count;
            }
            entry = static_cast<SoundLimitEntry*>(Handle_Get(entry->next));
        }
    }
}

int EvictSound(EventSystem* eventSystem, unsigned long long eventId, unsigned char policy, int priority,
               SoundLimitEntry* limit, SoundSource* source, const char* name)
{
    int bestIndex = -1;

    float listenerX;
    float listenerY;
    float listenerZ;
    AIL_listener_3D_position(eventSystem->m_listener, &listenerX, &listenerY, &listenerZ);

    float farthestDistance = 0.0f;
    float quietestVolume = QUIETEST_START_VOLUME;
    int oldestTime = AIL_ms_count();
    int lowestPriority = 0xff;

    const int count = eventSystem->m_instances.Count();
    for (int index = 0; index < count; ++index)
    {
        SoundInstanceData* sound = reinterpret_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[index]));
        if (sound->m_state == SOUND_STATE_DESTROYED)
        {
            continue;
        }
        if (policy == EVICT_LOWER_PRIORITY && sound->m_priority >= priority)
        {
            continue;
        }
        if (source != NULL && source != Handle_Get(sound->m_source))
        {
            continue;
        }
        if (name != NULL && AIL_stricmp(name, static_cast<const char*>(Handle_Get(sound->m_name))) != 0)
        {
            continue;
        }
        if (limit != NULL
            && DoLabelsMatchQuery(static_cast<const char*>(Handle_Get(sound->m_labels)), limit->query) == 0)
        {
            continue;
        }

        if (policy == EVICT_LOWER_PRIORITY)
        {
            Handle_Free(sound->m_self);
            sound->m_flags |= SOUND_FLAG_EVICTED;
            DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(sound));
            Audition_SoundEvicted(sound->m_eventId, eventId, 0);
            return 1;
        }

        switch (policy)
        {
        case EVICT_OLDEST:
            if (sound->m_startTime < oldestTime)
            {
                bestIndex = index;
                oldestTime = sound->m_startTime;
            }
            break;
        case EVICT_QUIETEST:
        {
            const float volume = sound->m_sample->volume;
            if (fabs(static_cast<double>(volume - quietestVolume)) < SOUND_VALUE_EPSILON)
            {
                if (sound->m_priority < lowestPriority)
                {
                    quietestVolume = volume;
                    bestIndex = index;
                    lowestPriority = sound->m_priority;
                }
            }
            else if (volume < quietestVolume)
            {
                quietestVolume = volume;
                bestIndex = index;
                lowestPriority = sound->m_priority;
            }
            break;
        }
        case EVICT_FARTHEST:
        {
            float x;
            float y;
            float z;
            AIL_sample_3D_position(sound->m_sample, &x, &y, &z);
            const float distance = (x - listenerX) * (x - listenerX) + (y - listenerY) * (y - listenerY)
                                   + (z - listenerZ) * (z - listenerZ);
            if (distance > farthestDistance)
            {
                farthestDistance = distance;
                bestIndex = index;
            }
            break;
        }
        }
    }

    if (policy == EVICT_LOWER_PRIORITY || bestIndex == -1)
    {
        return 0;
    }

    SoundInstanceData* sound = reinterpret_cast<SoundInstanceData*>(Handle_Get(eventSystem->m_instances[bestIndex]));
    Handle_Free(sound->m_self);
    sound->m_flags |= SOUND_FLAG_EVICTED;
    DestroySoundInstance(eventSystem, reinterpret_cast<SoundInstance*>(sound));
    Audition_SoundEvicted(sound->m_eventId, eventId, policy);
    return 1;
}

SoundLimitEntry* IsLabelLimited(EventSystem* eventSystem, _MSSSTRINGC label)
{
    SoundLimitList* limits = static_cast<SoundLimitList*>(Handle_Get(eventSystem->m_limits));
    if (limits != NULL && label.length != 0)
    {
        SoundLimitEntry* entry = static_cast<SoundLimitEntry*>(Handle_Get(limits->entries));
        while (entry != NULL)
        {
            if (DoLabelsMatchQuery(label.text, entry->query) != 0 && entry->count + 1 > entry->limit)
            {
                return entry;
            }
            entry = static_cast<SoundLimitEntry*>(Handle_Get(entry->next));
        }
    }
    return NULL;
}

extern "C" void AIL_set_sample_3D_volume_falloff(_SAMPLE* sample, const FalloffGraphPoint* points, int count);
extern "C" void AIL_set_sample_3D_lowpass_falloff(_SAMPLE* sample, const FalloffGraphPoint* points, int count);
extern "C" void AIL_set_sample_3D_exclusion_falloff(_SAMPLE* sample, const FalloffGraphPoint* points, int count);

void FalloffGraph_Set(_SAMPLE* sample, int flags, FalloffGraph* graph)
{
    if (Handle_Get(sample->falloffHandle) == NULL)
    {
        return;
    }

    if (graph == NULL)
    {
        if ((flags & 1) != 0)
        {
            AIL_set_sample_3D_volume_falloff(sample, NULL, 0);
        }
        if ((flags & 2) != 0)
        {
            AIL_set_sample_3D_lowpass_falloff(sample, NULL, 0);
        }
        if ((flags & 4) != 0)
        {
            AIL_set_sample_3D_exclusion_falloff(sample, NULL, 0);
        }
    }

    sample->maximumDistance = graph->maximumDistance;

    if ((flags & 1) != 0)
    {
        AIL_set_sample_3D_volume_falloff(sample, graph->points, graph->volumePointCount);
    }
    if ((flags & 2) != 0)
    {
        AIL_set_sample_3D_lowpass_falloff(sample, graph->points + graph->volumePointCount, graph->lowpassPointCount);
    }
    if ((flags & 4) != 0)
    {
        AIL_set_sample_3D_exclusion_falloff(sample, graph->points + graph->volumePointCount + graph->lowpassPointCount,
                                            graph->exclusionPointCount);
    }

    sample->is3D = 1;
}
