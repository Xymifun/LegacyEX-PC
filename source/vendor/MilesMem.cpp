#include "vendor/MilesMem.hpp"

#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <coreinit/time.h>
#ifdef __ghs__
#include <ppc_ghs.h>
#endif
#include <string.h>

#include "game/client/audio/FalloffGraph.hpp"
#include "game/core/PODHandleArray.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesEventError.hpp"
#include "vendor/MilesFilters.hpp"
#include "vendor/MilesIO.hpp"
#include "vendor/MilesHandle.hpp"
#include "vendor/MilesRadSS.hpp"
#include "vendor/MilesRib.hpp"
#include "vendor/MilesTime.hpp"

typedef void (*AIL_LOG_CALLBACK)(const char* pText, int iDepth);

static const char MILES_DEBUG_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mssdbg.c";

void __debugbreak();

static const char AIL_indent[] = "                                ";

const unsigned int WORD_MASK_16 = 0xffff;
const unsigned int BYTE_MASK_8 = 0xff;
const unsigned int ALL_BITS_SET = 0xffffffff;
const unsigned int ALIGN_16_MASK = 0xfffffff0;
const int FIXED_POINT_SHIFT = 16;
const int LOW_PASS_COEFFICIENT_SHIFT = 13;
const int LOW_PASS_MODE_ACTIVE = 0x2000f;
const int LOW_PASS_MODE_PENDING = 0x1000f;
const int BUFFER_SIZE_ROUNDING = 0xff;
const int MIX_ALIGNMENT_BYTES = 0x10;
const unsigned int BUS_TAIL_SCALE = 0x100;
const unsigned int BUS_TAIL_MARGIN = 0x6400;
const int PROFILE_SITE_BASE = 0x311e;

static const int AIL_LOG_PREFIX_LENGTH = 15;
static const unsigned int AIL_LOG_MAXIMUM_HOURS = 99;

int AIL_debug_enabled = 0;
int AIL_debug_nested = 0;
int AIL_debug_depth = 0;

int AIL_log_start_ms = 0;
AIL_LOG_CALLBACK AIL_log_callback = NULL;
FILE* AIL_log_file = NULL;

MilesProfileSite AIL_debug_log_site;
MilesProfileSite MSS_alloc_site;
MilesProfileSite MSS_free_site;

static int ShouldLog()
{
    if (AIL_debug_enabled == 0)
    {
        return 0;
    }

    if (AIL_debug_depth != 1 && AIL_debug_nested == 0)
    {
        return 0;
    }

    return AIL_API_background() == 0;
}

extern "C" void AIL_debug_log(const char* pFormat, ...)
{
    va_list arguments;
    char szTimestamp[16];
    char szBuffer[512];
    char* pText;
    char* pLine;
    const char* pMessageFormat;
    const char* pDescription;
    unsigned int uElapsed;
    unsigned int uHours;

    if (AIL_debug_enabled == 0)
    {
        return;
    }

    va_start(arguments, pFormat);

    pText = szBuffer + AIL_LOG_PREFIX_LENGTH;
    pMessageFormat = pFormat;

    if (*pFormat == '!' || *pFormat == '@')
    {
        if (AIL_debug_depth > 1)
        {
            strcpy(pText, &AIL_indent[sizeof(AIL_indent) - AIL_debug_depth * 2]);
            pText = pText + AIL_debug_depth * 2 - 1;
        }

        pMessageFormat = pFormat + 1;
    }

    vsnprintf(pText, (sizeof(szBuffer) - 1) - (size_t)(pText - szBuffer), pMessageFormat, arguments);
    va_end(arguments);

    szBuffer[sizeof(szBuffer) - 1] = '\0';

    if (MSS_profile != NULL)
    {
        pDescription = MSS_profile->pDescribe(MSS_profile, szBuffer + AIL_LOG_PREFIX_LENGTH);
        MSS_profile->pMessage(MSS_profile, 0, 1, &AIL_debug_log_site, "%s", pDescription);
    }

    if (AIL_log_file == NULL && AIL_log_callback == NULL && MSS_profile != NULL)
    {
        return;
    }

    uElapsed = (unsigned int)(AIL_ms_count() - AIL_log_start_ms);

    uHours = uElapsed / 3600000;
    if (uHours > AIL_LOG_MAXIMUM_HOURS)
    {
        uHours = AIL_LOG_MAXIMUM_HOURS;
    }

    pLine = szBuffer + AIL_LOG_PREFIX_LENGTH;

    if (*pFormat == '@' || *pFormat == '!')
    {
        if (*pFormat == '@' && AIL_debug_depth == 1)
        {
            sprintf(szTimestamp, "[%.02d:%.02d:%.02d.%.03d] ", (int)uHours, (int)((uElapsed / 60000) % 60),
                    (int)((uElapsed / 1000) % 60), (int)(uElapsed % 1000));
            memcpy(szBuffer, szTimestamp, AIL_LOG_PREFIX_LENGTH);
        }
        else
        {
            memset(szBuffer, ' ', AIL_LOG_PREFIX_LENGTH);
        }

        pLine = szBuffer;
    }

    if (AIL_log_file != NULL)
    {
        fprintf(AIL_log_file, "%s\n", pLine);
        return;
    }

    if (AIL_log_callback != NULL)
    {
        AIL_log_callback(pLine, AIL_debug_depth);
    }
}

extern "C" void AIL_set_error(const char* pError)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        if (pError == NULL)
        {
            AIL_debug_log("@AIL_set_error(NULL)");
        }
        else
        {
            AIL_debug_log("@AIL_set_error(%s)", pError);
        }
    }

    AIL_API_set_error(pError);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine)
{
    void* pBlock;
    unsigned int uCallStack;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        AIL_debug_log("@AIL_mem_alloc_lock(%ld) from: %s, line: %d ", iSize, pOwnerName, iOwnerLine);
    }

    pBlock = AIL_API_mem_alloc_lock((unsigned int)iSize);

    if (MSS_profile != NULL)
    {
        uCallStack = MSS_profile->pCallStack(MSS_profile, 0, 0);
        MSS_profile->pAlloc(MSS_profile, pBlock, 0, iSize, pOwnerName, iOwnerLine, &MSS_alloc_site, "Miles alloc: %t",
                            uCallStack);
    }

    if (ShouldLog() != 0)
    {
        outresuhex((unsigned int)(uintptr_t)pBlock);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return pBlock;
}

extern "C" void AIL_mem_free_lock(void* pBlock)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (ShouldLog() != 0)
    {
        AIL_debug_log("@AIL_mem_free_lock(0x%lX)", pBlock);
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pFree(MSS_profile, pBlock, MILES_DEBUG_FILE, 440, &MSS_free_site);
    }

    AIL_API_mem_free_lock(pBlock);

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" void outresuhex(unsigned int uResult)
{
    AIL_debug_log("!-Result = 0x%lX", uResult);
}

extern "C" void outresint(int iResult)
{
    AIL_debug_log("!-Result = %d", iResult);
}

static const unsigned int HANDLE_GUARD = 0xDEADBEEF;
static const int HANDLE_BLOCK_OVERHEAD = 0x17;
static const unsigned int HANDLE_BLOCK_ALIGNMENT_MASK = 0xFFFFFFF0;
static const int HANDLE_TAG_FREE = 1;

Handle Handle_AllocateInfo(int size, const char* file, int line)
{
    Handle handle;
    HandleEntry* entry;
    unsigned char* data;
    int index;
    unsigned int generation;
    int tableSize;
    int blockSize;

    blockSize = (size + HANDLE_BLOCK_OVERHEAD) & HANDLE_BLOCK_ALIGNMENT_MASK;
    tableSize = HandleCount * sizeof(HandleEntry);

    if (HandleFreeHead == 0)
    {
        tableSize = tableSize + sizeof(HandleEntry);
    }

    if (blockSize > (HandleTableEnd - HandleHeapCursor) - tableSize)
    {
        return Handle();
    }

    data = HandleHeapCursor;

    if (HandleFreeHead == 0)
    {
        index = HandleCount + 1;
        HandleCount = index;
    }
    else
    {
        index = -HandleFreeHead;
        entry = reinterpret_cast<HandleEntry*>(HandleTableEnd - index * sizeof(HandleEntry));
        HandleFreeHead = HandleFreeHead + entry->nextFree;
    }

    memset(data, 0, size);
    *reinterpret_cast<unsigned int*>(data + size) = HANDLE_GUARD;

    HandleHeapCursor = HandleHeapCursor + blockSize;
    reinterpret_cast<int*>(HandleHeapCursor)[-1] = 0;

    generation = HandleGeneration;
    HandleGeneration = HandleGeneration + 1;

    if (HandleGeneration == 0)
    {
        HandleGeneration = 1;
    }

    entry = reinterpret_cast<HandleEntry*>(HandleTableEnd - index * sizeof(HandleEntry));
    entry->size = size;
    entry->generation = generation;
    entry->data = data;
    reinterpret_cast<int*>(data)[-1] = -index;

    handle.index = index;
    handle.generation = generation;
    return handle;
}

void Handle_Free(Handle handle)
{
    HandleEntry* entry;
    unsigned int index;
    int size;

    index = handle.index;

    if (index == 0)
    {
        return;
    }

    if (index > HandleCount)
    {
        if (HandleBreakOnError != 0)
        {
            __debugbreak();
        }

        return;
    }

    entry = reinterpret_cast<HandleEntry*>(HandleTableEnd - index * sizeof(HandleEntry));
    size = entry->size;

    if (entry->generation != handle.generation)
    {
        if (HandleBreakOnError != 0)
        {
            __debugbreak();
        }

        return;
    }

    if (*reinterpret_cast<unsigned int*>(static_cast<unsigned char*>(entry->data) + size) != HANDLE_GUARD
        && HandleBreakOnError != 0)
    {
        __debugbreak();
    }

    entry->generation = 0;
    reinterpret_cast<int*>(entry->data)[-1] = HANDLE_TAG_FREE;
    *static_cast<unsigned int*>(entry->data) = (size + HANDLE_BLOCK_OVERHEAD) & HANDLE_BLOCK_ALIGNMENT_MASK;

    if (index == HandleCount)
    {
        HandleCount = HandleCount - 1;
        HandleNeedsDefrag = 1;
        return;
    }

    if (HandleFreeHead == 0)
    {
        entry->nextFree = index;
    }
    else
    {
        entry->nextFree = HandleFreeHead + index;
    }

    HandleNeedsDefrag = 1;
    HandleFreeHead = -static_cast<int>(index);
}

static const int SAMPLE_STATUS_FREE = 1;
static const int SAMPLE_STATUS_DONE = 2;

extern "C" void AIL_API_end_sample(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return;
    }

    if (pSample->driver->sampleStatus[pSample->index] == SAMPLE_STATUS_FREE
        || pSample->driver->sampleStatus[pSample->index] == SAMPLE_STATUS_DONE)
    {
        return;
    }

    pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_DONE;
    pSample->starved = 1;

    if (pSample->endOfBuffer != NULL)
    {
        pSample->endOfBuffer(pSample);
    }

    if (pSample->endOfSample != NULL)
    {
        pSample->endOfSample(pSample);
    }
}

extern "C" int AIL_API_sample_3D_position(_SAMPLE* pSample, float* pX, float* pY, float* pZ)
{
    if (pSample != NULL)
    {
        if (pX != NULL)
        {
            *pX = pSample->x;
        }

        if (pY != NULL)
        {
            *pY = pSample->y;
        }

        if (pZ != NULL)
        {
            *pZ = pSample->z;
        }

        return pSample->is3D;
    }

    return 0;
}

extern "C" void AIL_API_listener_3D_position(_LISTENER* pListener, float* pX, float* pY, float* pZ)
{
    if (pListener == NULL)
    {
        return;
    }

    if (pX != NULL)
    {
        *pX = pListener->x;
    }

    if (pY != NULL)
    {
        *pY = pListener->y;
    }

    if (pZ != NULL)
    {
        *pZ = pListener->z;
    }
}

extern "C" void AIL_end_sample(_SAMPLE* pSample)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_end_sample(0x%lX)", pSample);
    }

    InMilesMutex();
    AIL_API_end_sample(pSample);
    OutMilesMutex();

    AIL_debug_depth = AIL_debug_depth - 1;
}

extern "C" int AIL_sample_3D_position(_SAMPLE* pSample, float* pX, float* pY, float* pZ)
{
    int iResult;

    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_sample_3D_position(0x%lX,0x%lX,0x%lX,0x%lX)", pSample, pX, pY, pZ);
    }

    iResult = AIL_API_sample_3D_position(pSample, pX, pY, pZ);

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        outresint(iResult);
    }

    AIL_debug_depth = AIL_debug_depth - 1;
    return iResult;
}

extern "C" void AIL_listener_3D_position(_LISTENER* pListener, float* pX, float* pY, float* pZ)
{
    AIL_debug_depth = AIL_debug_depth + 1;

    if (AIL_debug_enabled != 0 && (AIL_debug_depth == 1 || AIL_debug_nested != 0) && AIL_API_background() == 0)
    {
        AIL_debug_log("@AIL_listener_3D_position(0x%lX,0x%lX,0x%lX,0x%lX)", pListener, pX, pY, pZ);
    }

    AIL_API_listener_3D_position(pListener, pX, pY, pZ);

    AIL_debug_depth = AIL_debug_depth - 1;
}

namespace
{
const int PRESET_MAGIC = 0x50535420;
const int AUDIO_BANK_NAME_OFFSET = 8;
const int SOUND_ASSET_PATH_MARKER = '*';
const int SOUND_ASSET_INFO_OFFSET = 0xc;
const int SOUND_ASSET_INFO_SIZE = 0x2c;
const int SOUND_ASSET_NAME_OFFSET = 4;
const int SPEAKER_COUNT = 8;
const int CHANNEL_MATRIX_SIZE = 64;
const unsigned int PRESET_FLAG_SPEAKER_SCALES = 0x20000;
const unsigned int PRESET_FLAG_CHANNEL_LEVELS = 0x10000;
const int PRESET_CHANNEL_LIMIT = 8;
const unsigned int PRESET_CHANNEL_MASK = 0xffff;
const int PRESET_CHANNEL_SHIFT = 16;
const int PRESET_NO_INDEX = -1;
const int PRESET_PAIR_WORDS = 2;
const int PRESET_FALLOFF_HEADER_WORDS = 6;
const int PRESET_FALLOFF_POINT_WORDS = 8;
const int PRESET_PROCESSOR_ENTRY_WORDS = 3;
const int PRESET_PROCESSOR_HEADER_WORDS = 4;
const int PRESET_CHANNEL_LEVEL_WORDS = 5;
const unsigned int PRESET_COMPRESSOR_BUS_LIMIT = 3;
const int SAMPLE_STAGE_BASE = 1;
const int SAMPLE_REVERB_BUS_COMPRESSOR = 4;
const int GRAPH_MODE_LINEAR = 0;
const int GRAPH_MODE_STEP = 2;
const int ALL_CHANNELS = -1;
const double PRESET_DECIBEL_BASE = 10.0;
const float PRESET_DECIBELS_PER_DECADE = 20.0f;
const double PRESET_VOLUME_EXPONENT = 0.6;
const double PRESET_SEMITONE_BASE = 2.0;
const float PRESET_SEMITONES_PER_OCTAVE = 12.0f;
const float PRESET_UNIT = 1.0f;
const float PRESET_ZERO = 0.0f;
const float PRESET_HALF = 0.5f;
const float PRESET_DEFAULT_CONE_ANGLE = 360.0f;
const float PRESET_DEFAULT_MAXIMUM_DISTANCE = 200.0f;
const float GRAPH_TANGENT_DIVISOR = 3.0f;
const float GRAPH_CUBIC_COEFFICIENT = 3.0f;
const char PRESET_COMPRESSOR_FILTER[] = "Compressor Filter";
const char PRESET_COMPRESSOR_INPUT[] = "Input";
const char PRESET_FILTER_ERROR[] = "Sound Preset Error - Filter Not Found.";
const char PRESET_NOT_FOUND_ERROR[] = "Preset Not Found.";

enum PresetRecordType
{
    PRESET_CHANNEL_ROUTING = 0,
    PRESET_POSITION = 1,
    PRESET_CONE = 2,
    PRESET_DISTANCES = 3,
    PRESET_ORIENTATION = 4,
    PRESET_VELOCITY = 5,
    PRESET_VALUE = 6,
    PRESET_REVERB = 7,
    PRESET_LOW_PASS = 8,
    PRESET_OCCLUSION = 9,
    PRESET_EXCLUSION = 10,
    PRESET_OBSTRUCTION = 11,
    PRESET_PITCH = 12,
    PRESET_VOLUME_PAN = 13,
    PRESET_FALLOFF = 14,
    PRESET_PROCESSOR = 15,
    PRESET_CHANNEL_LEVEL = 16
};

union PresetValue
{
    int integer;
    float number;
};

struct PresetPair
{
    PresetValue value;
    int expression;
};

struct PresetHeader
{
    int magic;
    int reserved0;
    unsigned int flags;
    int size;
};

struct PresetProcessorEntry
{
    int nameOffset;
    PresetValue value;
    int expression;
};

PresetPair* PresetPairs(int* record)
{
    return reinterpret_cast<PresetPair*>(record + 1);
}

void ResolvePair(PresetHeader* header, void* instance, PresetEvalFunc evaluate, PresetPair* pair, int flags)
{
    if (pair->expression != 0)
    {
        evaluate(instance, reinterpret_cast<char*>(header) + pair->expression, PRESET_NO_INDEX, &pair->value, flags);
    }
}

int* PresetEnd(PresetHeader* header)
{
    return reinterpret_cast<int*>(reinterpret_cast<char*>(header) + header->size);
}

int* PresetBegin(PresetHeader* header)
{
    return reinterpret_cast<int*>(header + 1);
}

void FillChannelMatrix(int* sources, int* destinations)
{
    for (int row = 0; row < PRESET_CHANNEL_LIMIT; ++row)
    {
        for (int column = 0; column < PRESET_CHANNEL_LIMIT; ++column)
        {
            sources[row * PRESET_CHANNEL_LIMIT + column] = column;
            destinations[row * PRESET_CHANNEL_LIMIT + column] = row;
        }
    }
}
}

extern "C" const char* AIL_get_soundbank_name(SoundBank* bank)
{
    SoundBankHeader* header = reinterpret_cast<SoundBankHeader*>(bank);
    if (header->magic == SOUND_BANK_MAGIC)
    {
        return header->name;
    }

    if (header->magic == AUDIO_BANK_MAGIC)
    {
        return reinterpret_cast<const char*>(header) + AUDIO_BANK_NAME_OFFSET;
    }

    return NULL;
}

extern "C" void* AIL_find_sound_preset(SoundBank* bank, const char* name)
{
    SoundBankHeader* header = reinterpret_cast<SoundBankHeader*>(bank);
    if (header == NULL || name == NULL || header->magic != SOUND_BANK_MAGIC)
    {
        return NULL;
    }

    SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
#if SOUND_BANK_OFFSETS
        static_cast<uintptr_t>(FindAsset(bank, name, SoundBankAt<SoundBank::AssetEntry>(header, header->presets),
                                         header->presetCount)));
#else
        static_cast<uintptr_t>(FindAsset(bank, name, header->presets, header->presetCount)));
#endif
    if (entry == NULL)
    {
        AIL_set_error(PRESET_NOT_FOUND_ERROR);
        return NULL;
    }

    return reinterpret_cast<char*>(header) + entry->value;
}

extern "C" int AIL_sound_asset_info(SoundBank* bank, const char* name, char* path, _MILESBANKSOUNDINFO* info)
{
    SoundBankHeader* header = reinterpret_cast<SoundBankHeader*>(bank);
    SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
#if SOUND_BANK_OFFSETS
        static_cast<uintptr_t>(FindAsset(bank, name, SoundBankAt<SoundBank::AssetEntry>(header, header->sounds),
                                         header->soundCount)));
#else
        static_cast<uintptr_t>(FindAsset(bank, name, header->sounds, header->soundCount)));
#endif
    if (entry == NULL)
    {
        if (path != NULL)
        {
            *path = '\0';
        }
        return 0;
    }

    char* asset = reinterpret_cast<char*>(header) + entry->value;
    if (info != NULL)
    {
        memcpy(info, asset + SOUND_ASSET_INFO_OFFSET, SOUND_ASSET_INFO_SIZE);
    }

    const char* assetName = asset + *reinterpret_cast<int*>(asset + SOUND_ASSET_NAME_OFFSET);
    size_t length = strlen(assetName);
    int prefixLength = header->pathLength;
    if (path != NULL)
    {
        *path = SOUND_ASSET_PATH_MARKER;
#if SOUND_BANK_OFFSETS
        memcpy(path + 1, SoundBankAt<const char>(header, header->path), header->pathLength);
#else
        memcpy(path + 1, header->path, header->pathLength);
#endif
        strcpy(path + header->pathLength + 1, assetName);
    }

    return prefixLength + length + 2;
}

extern "C" float evaluate_graph(float position, const GraphPoint* points, unsigned int count)
{
    if (count == 0)
    {
        return PRESET_ZERO;
    }

    if (count != 1)
    {
        const int last = count - 1;
        if (last >= 0)
        {
            int index = last;
            for (unsigned int remaining = count; remaining != 0; --remaining, --index)
            {
                if (points[index].x <= position)
                {
                    break;
                }
            }

            if (index >= 0 && points[index].x <= position)
            {
                const GraphPoint* current = points + index;
                if (index == last)
                {
                    return current->y;
                }

                const GraphPoint* next = current + 1;
                if (current->outMode != GRAPH_MODE_STEP && next->inMode != GRAPH_MODE_STEP)
                {
                    const float width = next->x - current->x;
                    const float height = next->y - current->y;
                    float outX = current->outTangentX;
                    float outY = current->outTangentY;
                    if (current->outMode == GRAPH_MODE_LINEAR)
                    {
                        outX = width;
                        outY = height;
                    }

                    float inX = next->inTangentX;
                    float inY = next->inTangentY;
                    if (next->inMode == GRAPH_MODE_LINEAR)
                    {
                        inX = current->x - next->x;
                        inY = current->y - next->y;
                    }

                    const float inSlope = width
                                          * ((next->y - (next->y - inY / GRAPH_TANGENT_DIVISOR))
                                             / (next->x - (next->x - inX / GRAPH_TANGENT_DIVISOR)));
                    const float outSlope = ((current->y + outY / GRAPH_TANGENT_DIVISOR) - current->y)
                                           / ((current->x + outX / GRAPH_TANGENT_DIVISOR) - current->x);
                    const float outScaled = width * outSlope;
                    const float offset = position - current->x;
                    const float inverseSquare = PRESET_UNIT / (width * width);
                    return offset
                               * (offset
                                      * (((((height + height + height) - outScaled) - outScaled) - inSlope)
                                             * inverseSquare
                                         + offset
                                               * ((((outScaled + inSlope) - height) - height) * inverseSquare / width))
                                  + outSlope)
                           + current->y;
                }

                return current->y;
            }
        }
    }

    return points[0].y;
}

extern "C" int AIL_resolve_raw_sound_preset(void* preset, void* instance, PresetEvalFunc evaluate)
{
    PresetHeader* header = static_cast<PresetHeader*>(preset);
    if (header == NULL || header->magic != PRESET_MAGIC)
    {
        return 0;
    }

    int* record = PresetBegin(header);
    int* end = PresetEnd(header);
    while (record != end)
    {
        PresetPair* pairs = PresetPairs(record);
        switch (record[0])
        {
        case PRESET_POSITION:
        case PRESET_CONE:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            record += 7;
            break;
        case PRESET_DISTANCES:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 0);
            record += 7;
            break;
        case PRESET_ORIENTATION:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            ResolvePair(header, instance, evaluate, pairs + 3, 1);
            ResolvePair(header, instance, evaluate, pairs + 4, 1);
            ResolvePair(header, instance, evaluate, pairs + 5, 1);
            record += 13;
            break;
        case PRESET_VELOCITY:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            ResolvePair(header, instance, evaluate, pairs + 3, 1);
            record += 9;
            break;
        case PRESET_VALUE:
        case PRESET_LOW_PASS:
        case PRESET_OCCLUSION:
        case PRESET_EXCLUSION:
        case PRESET_OBSTRUCTION:
        case PRESET_PITCH:
            ResolvePair(header, instance, evaluate, pairs, 1);
            record += 3;
            break;
        case PRESET_REVERB:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            record += 5;
            break;
        case PRESET_VOLUME_PAN:
            ResolvePair(header, instance, evaluate, pairs, 1);
            ResolvePair(header, instance, evaluate, pairs + 1, 1);
            ResolvePair(header, instance, evaluate, pairs + 2, 1);
            ResolvePair(header, instance, evaluate, pairs + 3, 1);
            ResolvePair(header, instance, evaluate, pairs + 4, 1);
            record += 11;
            break;
        case PRESET_FALLOFF:
            record += (record[4] + record[5] + record[3]) * PRESET_FALLOFF_POINT_WORDS + PRESET_FALLOFF_HEADER_WORDS;
            break;
        case PRESET_PROCESSOR:
        {
            PresetProcessorEntry* entries = reinterpret_cast<PresetProcessorEntry*>(record + 4);
            for (int index = 0; index < record[3]; ++index)
            {
                if (entries[index].expression != 0)
                {
                    evaluate(instance, reinterpret_cast<char*>(header) + entries[index].expression, PRESET_NO_INDEX,
                             &entries[index].value, 1);
                }
            }
            record += record[3] * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
            break;
        }
        default:
            record += 3;
            break;
        }
    }

    return 1;
}

extern "C" int AIL_apply_raw_sound_preset(_SAMPLE* sample, void* preset)
{
    PresetHeader* header = static_cast<PresetHeader*>(preset);
    if (sample == NULL || header == NULL || header->magic != PRESET_MAGIC)
    {
        return 0;
    }

    int speakers[SPEAKER_COUNT];
    float scales[SPEAKER_COUNT];
    int sources[CHANNEL_MATRIX_SIZE];
    int destinations[CHANNEL_MATRIX_SIZE];
    float levels[CHANNEL_MATRIX_SIZE];

    if ((header->flags & PRESET_FLAG_SPEAKER_SCALES) != 0)
    {
        for (int speaker = 0; speaker < SPEAKER_COUNT; ++speaker)
        {
            speakers[speaker] = speaker;
        }
        AIL_sample_speaker_scale_factors(sample, speakers, scales, SPEAKER_COUNT);
    }

    if ((header->flags & PRESET_FLAG_CHANNEL_LEVELS) != 0)
    {
        FillChannelMatrix(sources, destinations);
        AIL_sample_channel_levels(sample, sources, destinations, levels, CHANNEL_MATRIX_SIZE);
    }

    int* record = PresetBegin(header);
    int* end = PresetEnd(header);
    while (record != end)
    {
        PresetPair* pairs = PresetPairs(record);
        int words = PRESET_PAIR_WORDS + 1;
        switch (record[0])
        {
        case PRESET_CHANNEL_ROUTING:
            if (record[1] == 0)
            {
                AIL_set_sample_channel_levels(sample, NULL, NULL, NULL, 0);
            }
            else if (record[1] == 1)
            {
                AIL_set_sample_is_3D(sample, 1);
            }
            break;
        case PRESET_POSITION:
            AIL_set_sample_3D_position(sample, pairs[0].value.number, pairs[1].value.number, pairs[2].value.number);
            words = 7;
            break;
        case PRESET_CONE:
            AIL_set_sample_3D_cone(sample, pairs[0].value.number, pairs[1].value.number,
                                   pow(PRESET_DECIBEL_BASE, pairs[2].value.number / PRESET_DECIBELS_PER_DECADE));
            words = 7;
            break;
        case PRESET_DISTANCES:
            AIL_set_sample_3D_distances(sample, pairs[1].value.number, pairs[0].value.number, pairs[2].value.integer);
            words = 7;
            break;
        case PRESET_ORIENTATION:
            AIL_set_sample_3D_orientation(sample, pairs[0].value.number, pairs[1].value.number, pairs[2].value.number,
                                          pairs[3].value.number, pairs[4].value.number, pairs[5].value.number);
            words = 13;
            break;
        case PRESET_VELOCITY:
            AIL_set_sample_3D_velocity(sample, pairs[0].value.number, pairs[1].value.number, pairs[2].value.number,
                                       pairs[3].value.number);
            words = 9;
            break;
        case PRESET_VALUE:
            sample->presetValue = pairs[0].value.number;
            break;
        case PRESET_REVERB:
            AIL_set_sample_reverb_levels(sample,
                                         pow(PRESET_DECIBEL_BASE, pairs[1].value.number / PRESET_DECIBELS_PER_DECADE),
                                         pow(PRESET_DECIBEL_BASE, pairs[0].value.number / PRESET_DECIBELS_PER_DECADE));
            words = 5;
            break;
        case PRESET_LOW_PASS:
        {
            float cutOff = pairs[0].value.number / static_cast<float>(sample->playbackRate / 2);
            if (cutOff > PRESET_UNIT)
            {
                cutOff = PRESET_UNIT;
            }
            AIL_set_sample_low_pass_cut_off(sample, ALL_CHANNELS, cutOff);
            break;
        }
        case PRESET_OCCLUSION:
            AIL_set_sample_occlusion(sample, pairs[0].value.number);
            break;
        case PRESET_EXCLUSION:
            AIL_set_sample_exclusion(sample, pairs[0].value.number);
            break;
        case PRESET_OBSTRUCTION:
            AIL_set_sample_obstruction(sample, pairs[0].value.number);
            break;
        case PRESET_PITCH:
            AIL_set_sample_playback_rate_factor(
                sample, pow(PRESET_SEMITONE_BASE, pairs[0].value.number / PRESET_SEMITONES_PER_OCTAVE));
            break;
        case PRESET_VOLUME_PAN:
        {
            float volume = pow(pow(PRESET_DECIBEL_BASE, pairs[0].value.number / PRESET_DECIBELS_PER_DECADE),
                               PRESET_VOLUME_EXPONENT);
            float fade = pow(pow(PRESET_DECIBEL_BASE, pairs[3].value.number / PRESET_DECIBELS_PER_DECADE),
                             PRESET_VOLUME_EXPONENT);
            float center = pow(pow(PRESET_DECIBEL_BASE, pairs[4].value.number / PRESET_DECIBELS_PER_DECADE),
                               PRESET_VOLUME_EXPONENT);
            AIL_set_sample_51_volume_pan(sample, volume, pairs[1].value.number, pairs[2].value.number, fade, center);
            words = 11;
            break;
        }
        case PRESET_FALLOFF:
            FalloffGraph_Set(sample, record[1], reinterpret_cast<FalloffGraph*>(record + 1));
            words = (record[4] + record[5] + record[3]) * PRESET_FALLOFF_POINT_WORDS + PRESET_FALLOFF_HEADER_WORDS;
            break;
        case PRESET_PROCESSOR:
        {
            const int stage = record[1] + SAMPLE_STAGE_BASE;
            PresetProcessorEntry* entries = reinterpret_cast<PresetProcessorEntry*>(record + 4);
            int count = record[3];
            if (record[2] == 0)
            {
                AIL_set_sample_processor(sample, stage, 0);
                words = count * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
                break;
            }

            const char* filterName = reinterpret_cast<char*>(header) + record[2];
            int provider = 0;
            if (AIL_find_filter(filterName, &provider) == 0)
            {
                AIL_set_error(PRESET_FILTER_ERROR);
                return 0;
            }

            if (AIL_sample_processor(sample, stage) != provider)
            {
                if (sample->reverbBus != SAMPLE_REVERB_BUS_COMPRESSOR
                    || strcmp(filterName, PRESET_COMPRESSOR_FILTER) != 0)
                {
                    AIL_set_sample_processor(sample, stage, provider);
                }
                else
                {
                    _DIG_DRIVER* driver = sample->driver;
                    int bus = 0;
                    if (driver->busSamples[0] != sample)
                    {
                        if (driver->busSamples[1] == sample)
                        {
                            bus = 1;
                        }
                        else if (driver->busSamples[2] == sample)
                        {
                            bus = 2;
                        }
                        else if (driver->busSamples[3] == sample)
                        {
                            bus = 3;
                        }
                    }

                    unsigned int inputBus = 0;
                    for (int index = 0; index < count; ++index)
                    {
                        if (strcmp(reinterpret_cast<char*>(header) + entries[index].nameOffset, PRESET_COMPRESSOR_INPUT)
                            == 0)
                        {
                            inputBus = static_cast<unsigned int>(entries[index].value.number);
                        }
                    }

                    if (inputBus > PRESET_COMPRESSOR_BUS_LIMIT)
                    {
                        inputBus = 0;
                    }
                    AIL_install_bus_compressor(driver, bus, stage, inputBus);
                    count = record[3];
                }
            }

            for (int index = 0; index < count; ++index)
            {
                int value = entries[index].value.integer;
                AIL_sample_stage_property(sample, stage, reinterpret_cast<char*>(header) + entries[index].nameOffset,
                                          ALL_CHANNELS, 0, &value, 0);
                count = record[3];
            }

            words = count * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
            break;
        }
        case PRESET_CHANNEL_LEVEL:
        {
            const unsigned int packed = record[1];
            if ((packed & PRESET_CHANNEL_MASK) < PRESET_CHANNEL_LIMIT
                && (packed >> PRESET_CHANNEL_SHIFT) < PRESET_CHANNEL_LIMIT)
            {
                levels[(packed & PRESET_CHANNEL_MASK) * PRESET_CHANNEL_LIMIT + (packed >> PRESET_CHANNEL_SHIFT)] =
                    pairs[1].value.number;
            }
            words = PRESET_CHANNEL_LEVEL_WORDS;
            break;
        }
        default:
            break;
        }

        record += words;
    }

    if ((header->flags & PRESET_FLAG_SPEAKER_SCALES) != 0)
    {
        AIL_set_sample_speaker_scale_factors(sample, speakers, scales, SPEAKER_COUNT);
    }

    if ((header->flags & PRESET_FLAG_CHANNEL_LEVELS) != 0)
    {
        AIL_set_sample_channel_levels(sample, sources, destinations, levels, CHANNEL_MATRIX_SIZE);
    }

    return 1;
}

extern "C" int AIL_unapply_raw_sound_preset(_SAMPLE* sample, void* preset)
{
    PresetHeader* header = static_cast<PresetHeader*>(preset);
    if (sample == NULL || header == NULL || header->magic != PRESET_MAGIC)
    {
        return 0;
    }

    int* record = PresetBegin(header);
    int* end = PresetEnd(header);
    while (record != end)
    {
        int words = PRESET_PAIR_WORDS + 1;
        switch (record[0])
        {
        case PRESET_CHANNEL_ROUTING:
            AIL_set_sample_is_3D(sample, 0);
            AIL_set_sample_channel_levels(sample, NULL, NULL, NULL, 0);
            break;
        case PRESET_POSITION:
            AIL_set_sample_3D_position(sample, PRESET_ZERO, PRESET_ZERO, PRESET_ZERO);
            words = 7;
            break;
        case PRESET_CONE:
            AIL_set_sample_3D_cone(sample, PRESET_DEFAULT_CONE_ANGLE, PRESET_DEFAULT_CONE_ANGLE, PRESET_UNIT);
            words = 7;
            break;
        case PRESET_DISTANCES:
            AIL_set_sample_3D_distances(sample, PRESET_DEFAULT_MAXIMUM_DISTANCE, PRESET_ZERO, 1);
            words = 7;
            break;
        case PRESET_ORIENTATION:
            AIL_set_sample_3D_orientation(sample, PRESET_UNIT, PRESET_ZERO, PRESET_ZERO, PRESET_ZERO, PRESET_UNIT,
                                          PRESET_ZERO);
            words = 13;
            break;
        case PRESET_VELOCITY:
            AIL_set_sample_3D_velocity(sample, PRESET_UNIT, PRESET_ZERO, PRESET_ZERO, PRESET_ZERO);
            words = 9;
            break;
        case PRESET_VALUE:
            sample->presetValue = PRESET_ZERO;
            break;
        case PRESET_REVERB:
            AIL_set_sample_reverb_levels(sample, PRESET_UNIT, PRESET_ZERO);
            words = 5;
            break;
        case PRESET_LOW_PASS:
            AIL_set_sample_low_pass_cut_off(sample, ALL_CHANNELS, PRESET_UNIT);
            break;
        case PRESET_OCCLUSION:
            AIL_set_sample_occlusion(sample, PRESET_ZERO);
            break;
        case PRESET_EXCLUSION:
            AIL_set_sample_exclusion(sample, PRESET_ZERO);
            break;
        case PRESET_OBSTRUCTION:
            AIL_set_sample_obstruction(sample, PRESET_ZERO);
            break;
        case PRESET_PITCH:
            AIL_set_sample_playback_rate_factor(sample, PRESET_UNIT);
            break;
        case PRESET_VOLUME_PAN:
            AIL_set_sample_51_volume_pan(sample, PRESET_UNIT, PRESET_HALF, PRESET_HALF, PRESET_UNIT, PRESET_UNIT);
            words = 11;
            break;
        case PRESET_FALLOFF:
            FalloffGraph_Set(sample, record[1], NULL);
            words = (record[4] + record[5] + record[3]) * PRESET_FALLOFF_POINT_WORDS + PRESET_FALLOFF_HEADER_WORDS;
            break;
        case PRESET_PROCESSOR:
            if (record[2] != 0)
            {
                AIL_set_sample_processor(sample, record[1] + SAMPLE_STAGE_BASE, 0);
            }
            words = record[3] * PRESET_PROCESSOR_ENTRY_WORDS + PRESET_PROCESSOR_HEADER_WORDS;
            break;
        case PRESET_CHANNEL_LEVEL:
            words = PRESET_CHANNEL_LEVEL_WORDS;
            break;
        default:
            break;
        }

        record += words;
    }

    if ((header->flags & PRESET_FLAG_SPEAKER_SCALES) != 0)
    {
        int speakers[SPEAKER_COUNT];
        float scales[SPEAKER_COUNT];
        for (int speaker = 0; speaker < SPEAKER_COUNT; ++speaker)
        {
            speakers[speaker] = speaker;
            scales[speaker] = PRESET_UNIT;
        }
        AIL_set_sample_speaker_scale_factors(sample, speakers, scales, SPEAKER_COUNT);
    }

    if ((header->flags & PRESET_FLAG_CHANNEL_LEVELS) != 0)
    {
        int sources[CHANNEL_MATRIX_SIZE];
        int destinations[CHANNEL_MATRIX_SIZE];
        float levels[CHANNEL_MATRIX_SIZE];
        for (int index = 0; index < CHANNEL_MATRIX_SIZE; ++index)
        {
            levels[index] = PRESET_ZERO;
        }
        FillChannelMatrix(sources, destinations);
        for (int channel = 0; channel < PRESET_CHANNEL_LIMIT; ++channel)
        {
            levels[channel * PRESET_CHANNEL_LIMIT + channel] = PRESET_UNIT;
        }
        AIL_set_sample_channel_levels(sample, sources, destinations, levels, CHANNEL_MATRIX_SIZE);
    }

    return 1;
}

namespace
{
const char MILES_FILTER_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\flt.cpp";
const char MILES_WAVEFILE_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\wavefile.cpp";

const unsigned int RIB_FUNCTION = 0;
const unsigned int RIB_PROPERTY = 1;
const unsigned int RIB_ENTRY_NOT_FOUND = 0xFFFFFFFF;
const unsigned int RIB_ENUMERATE_FIRST = 0xFFFFFFFF;

const char ASI_STREAM_INTERFACE[] = "ASI stream";
const char ASI_CODEC_INTERFACE[] = "ASI codec";
const char MIXER_SERVICES_INTERFACE[] = "MSS mixer services";
const char PIPELINE_FILTER_INTERFACE[] = "MSS pipeline filter";
const char VOICE_FILTER_INTERFACE[] = "MSS voice filter";
const char FILTER_SAMPLE_INTERFACE[] = "Pipeline filter sample services";
const char COMPRESSOR_NOT_FOUND_ERROR[] = "Compressor Filter Not Found";
const char BUS_INDEX_ERROR[] = "Both indices must refer to installed busses.";
const char OUT_OF_MEMORY_ERROR[] = "Out of memory";
const char NOT_A_FILTER_ERROR[] = "Not a filter provider";
const char MATRIX_FILTER_ERROR[] = "Matrix filter already associated with driver";
const char FILTER_LOAD_ERROR[] = "Error loading filter";
const char CHANNEL_BUFFER_ERROR[] = "Channel buffer allocation failed.";

const int SPEAKER_CONFIG_COUNT = 10;
const int SPEAKER_ID_COUNT = 18;
const int SPEAKER_ID_LAST = 17;
const int SPEAKER_UNMAPPED = -1;

const signed char SPEAKER_CHANNEL_MAP[SPEAKER_CONFIG_COUNT][SPEAKER_ID_COUNT] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, -1, -1, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, -1, -1, 2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, -1, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, -1, 6, 7, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, -1, -1, -1, -1, -1, -1, -1}};

const unsigned int SPEAKER_TRAIT_LEFT = 0x01;
const unsigned int SPEAKER_TRAIT_RIGHT = 0x02;
const unsigned int SPEAKER_TRAIT_FRONT = 0x04;
const unsigned int SPEAKER_TRAIT_BACK = 0x08;
const unsigned int SPEAKER_TRAIT_SIDE = 0x10;
const unsigned int SPEAKER_TRAIT_CENTER = 0x20;
const unsigned int SPEAKER_TRAIT_TOP = 0x40;
const unsigned int SPEAKER_TRAIT_LOW_FREQUENCY = 0x80;
const unsigned int SPEAKER_TRAIT_SHARED = SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_CENTER
                                          | SPEAKER_TRAIT_TOP | SPEAKER_TRAIT_LOW_FREQUENCY;
const unsigned int SPEAKER_TRAIT_SIDES = SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_RIGHT;

const unsigned char SPEAKER_TRAITS[SPEAKER_ID_COUNT] = {SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_FRONT,
                                                        SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_FRONT,
                                                        SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER,
                                                        SPEAKER_TRAIT_LOW_FREQUENCY,
                                                        SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_BACK,
                                                        SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_BACK,
                                                        SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER,
                                                        SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_FRONT
                                                            | SPEAKER_TRAIT_CENTER,
                                                        SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_CENTER,
                                                        SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_SIDE,
                                                        SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_SIDE,
                                                        SPEAKER_TRAIT_TOP | SPEAKER_TRAIT_CENTER,
                                                        SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_TOP,
                                                        SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_CENTER | SPEAKER_TRAIT_TOP,
                                                        SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_FRONT | SPEAKER_TRAIT_TOP,
                                                        SPEAKER_TRAIT_LEFT | SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_TOP,
                                                        SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_CENTER | SPEAKER_TRAIT_TOP,
                                                        SPEAKER_TRAIT_RIGHT | SPEAKER_TRAIT_BACK | SPEAKER_TRAIT_TOP};

const unsigned int SAMPLE_FORMAT_16_BITS = 1;
const unsigned int SAMPLE_FORMAT_STEREO = 2;
const unsigned int SAMPLE_FORMAT_ADPCM = 4;
const unsigned int SAMPLE_FORMAT_MULTICHANNEL = 0x10;
const unsigned int SAMPLE_FORMAT_MASK = 0xffff;
const int SAMPLE_FORMAT_CHANNEL_SHIFT = 16;
const int SAMPLE_WIDTH_16 = 16;
const int SAMPLE_WIDTH_8 = 8;

const int SAMPLE_STATUS_PLAYING = 4;
const int SAMPLE_NO_REVERB_BUS = 4;
const float REVERB_SEND_THRESHOLD = 0.0001f;

const unsigned int CHANNEL_BUFFER_ALIGNMENT = 16;
const unsigned int CHANNEL_BUFFER_SLACK = 0x1e;
const unsigned int CHANNEL_ARRAY_ALIGNMENT = 4;
const int CHANNEL_HEADER_UNITY = 0x10000;
const int CHANNEL_LIMIT_DEFAULT = 0x7fff;
const float CHANNEL_BUS_LEVEL_DEFAULT = -1.0f;
const float CHANNEL_LOW_PASS_DEFAULT = 1.0f;
const int CHANNEL_MAP_LENGTH = 0x11;
const int CHANNEL_MASK_BITS = 32;
const int CHANNEL_SCRATCH_ROWS = 9;

const float CONE_ANGLE_DIVISOR = 114.59156f;
const float CONE_FULL_ANGLE = 360.0f;
const float CONE_FULL_VOLUME = 1.0f;
const float OCCLUSION_LOW_PASS_BASE = 1.01f;
const float DIRECTION_EPSILON = 0.0001f;

const float VOLUME_CURVE_EXPONENT = 1.6666666f;
const float PAN_CURVE_EXPONENT = 0.3f;
const float PAN_CENTER_GAIN = 0.8122522f;
const float PAN_CENTER = 0.5f;
const float PAN_UNIT = 1.0f;

const unsigned int FILTER_FLAG_MATRIX = 4;
const unsigned int BUS_COMPRESSOR_LINK_FLAGS = 3;
const int BUS_COMPRESSOR_STAGE_SHIFT = 16;
const unsigned int DRIVER_FLAG_16_BITS = 1;
const unsigned int DRIVER_FLAG_STEREO_OR_MULTICHANNEL = 0x12;
const int ASI_DEFAULT_CHANNELS = 2;
const int FILTER_STATE_LINE = 0xc07;
const int FILTER_DRIVER_STATE_LINE = 0x196;
const int FILTER_INSTANCE_LINE = 0xeb;
const int CHANNEL_BUFFER_LINE = 0x336;
const int CHANNEL_MAP_LINE = 0x377;
const unsigned int ASI_PROFILE_BASE = 0x1000;
const unsigned int ASI_PROFILE_LIMIT = 0x11;
const unsigned int ASI_CHANNEL_LIMIT = 3;
const int LOOP_MODE_ASI = 1;
const int LOOP_MODE_STREAMING = 2;
}

extern "C" void AIL_API_set_sample_speaker_scale_factors(_SAMPLE* pSample, const int* pSpeakers, const float* pScales,
                                                         int iCount)
{
    if (pSample == NULL || pSpeakers == NULL || pScales == NULL || iCount == 0)
    {
        return;
    }

    int iConfig = pSample->driver->speakerConfig;
    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iChannel = SPEAKER_CHANNEL_MAP[iConfig][pSpeakers[iIndex]];
        if (iChannel != SPEAKER_UNMAPPED)
        {
            pSample->speakerScales[iChannel] = pScales[iIndex];
        }
    }
}

extern "C" void AIL_API_sample_speaker_scale_factors(_SAMPLE* pSample, int* pSpeakers, float* pScales, int iCount)
{
    if (pSample == NULL || pSpeakers == NULL || pScales == NULL || iCount == 0)
    {
        return;
    }

    int iConfig = pSample->driver->speakerConfig;
    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iChannel = SPEAKER_CHANNEL_MAP[iConfig][pSpeakers[iIndex]];
        if (iChannel != SPEAKER_UNMAPPED)
        {
            pScales[iIndex] = pSample->speakerScales[iChannel];
        }
    }
}

extern "C" void AIL_API_sample_channel_levels(_SAMPLE* pSample, int* pSources, int* pDestinations, float* pLevels,
                                              int iCount)
{
    if (pSample == NULL || pSources == NULL || pDestinations == NULL || pLevels == NULL)
    {
        return;
    }

    int iConfig = pSample->driver->speakerConfig;
    if (iConfig == 0)
    {
        return;
    }

    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iSource = pSources[iIndex];
        if (pSample->channelMap != NULL)
        {
            iSource = pSample->channelMap[iSource];
        }
        else if (iSource >= pSample->channelCount)
        {
            iSource = -1;
        }

        int iSpeaker = SPEAKER_UNMAPPED;
        if (iSource != -1)
        {
            iSpeaker = SPEAKER_CHANNEL_MAP[iConfig][pDestinations[iIndex]];
        }

        if (iSpeaker == SPEAKER_UNMAPPED)
        {
            pLevels[iIndex] = 0.0f;
        }
        else
        {
            pLevels[iIndex] = pSample->levels[iSpeaker][iSource];
        }
    }
}

extern "C" void AIL_API_set_sample_channel_levels(_SAMPLE* pSample, const int* pSources, const int* pDestinations,
                                                  const float* pLevels, int iCount)
{
    if (pSample == NULL)
    {
        return;
    }

    _DIG_DRIVER* pDriver = pSample->driver;
    int iConfig = pDriver->speakerConfig;
    if (iConfig == 0)
    {
        return;
    }

    if (pSources == NULL || pDestinations == NULL || pLevels == NULL)
    {
        set_user_channel_defaults(iConfig, pSample->channelCount, pSample->levels, pSample->channelMap);
        return;
    }

    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iSource = pSources[iIndex];
        if (pSample->channelMap != NULL)
        {
            iSource = pSample->channelMap[iSource];
        }
        else if (iSource >= pSample->channelCount)
        {
            continue;
        }

        if (iSource != -1)
        {
            int iSpeaker = SPEAKER_CHANNEL_MAP[iConfig][pDestinations[iIndex]];
            if (iSpeaker != SPEAKER_UNMAPPED)
            {
                pSample->levels[iSpeaker][iSource] = pLevels[iIndex];
            }
        }
    }

    pSample->userLevels = 0;
    if ((pSample->channelFlags & SAMPLE_FORMAT_STEREO) != 0)
    {
        for (int iOutput = 0; iOutput < pDriver->outputCount; ++iOutput)
        {
            if (pDriver->outputs[iOutput].chans > 1
                && (pSample->levels[iOutput * 2 + 1][0] > 0.0f || pSample->levels[iOutput * 2][1] > 0.0f))
            {
                pSample->userLevels = 1;
            }
        }
    }
}

extern "C" void SS_update_sample_reverb_state(_SAMPLE* pSample)
{
    if (pSample->reverbBus == SAMPLE_NO_REVERB_BUS)
    {
        return;
    }

    if (pSample->reverbSecond <= REVERB_SEND_THRESHOLD)
    {
        return;
    }

    DriverBus* pBus = &pSample->driver->buses[pSample->reverbBus];
    if (pBus->sendLevel <= REVERB_SEND_THRESHOLD)
    {
        return;
    }

    if (pBus->returnLevel <= REVERB_SEND_THRESHOLD)
    {
        return;
    }

    pBus->updateValue = pSample->driver->sharedState;
    pSample->driver->buses[pSample->reverbBus].update = 1;
}

extern "C" void AIL_API_set_sample_reverb_levels(_SAMPLE* pSample, float fFirst, float fSecond)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->reverbFirst = fFirst;
    pSample->reverbSecond = fSecond;
    pSample->reverbDirty = 1;

    if (pSample->driver->sampleStatus[pSample->index] != SAMPLE_STATUS_PLAYING)
    {
        return;
    }

    SS_update_sample_reverb_state(pSample);
}

extern "C" void AIL_API_set_sample_low_pass_cut_off(_SAMPLE* pSample, int iChannel, float fCutOff)
{
    if (pSample == NULL)
    {
        return;
    }

    int iFirst;
    int iEnd;
    if (iChannel == ALL_CHANNELS)
    {
        iFirst = 0;
        iEnd = pSample->channelCount;
        if (iEnd < 1)
        {
            return;
        }
    }
    else
    {
        if (pSample->channelMap == NULL)
        {
            if (iChannel >= pSample->channelCount)
            {
                return;
            }
        }
        else
        {
            iChannel = pSample->channelMap[iChannel];
            if (iChannel == -1)
            {
                return;
            }
        }

        iFirst = iChannel;
        iEnd = iChannel + 1;
    }

    for (int iIndex = iFirst; iIndex < iEnd; ++iIndex)
    {
        pSample->channelStates[iIndex].lowPass = fCutOff;
        pSample->cacheState = pSample->cacheState | (1 << iIndex);
    }
}

extern "C" void AIL_API_set_sample_51_volume_pan(_SAMPLE* pSample, float fVolume, float fPan, float fFade,
                                                 float fCenter, float fLowFrequency)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->volumeCurve = static_cast<float>(pow(fVolume, VOLUME_CURVE_EXPONENT));
    pSample->mixInput0 = fFade;
    pSample->pan = fPan;
    pSample->mixInput2 = fLowFrequency;
    pSample->mixInput1 = fCenter;

    float fFront = PAN_CENTER_GAIN;
    float fBack = PAN_CENTER_GAIN;
    if (fFade != PAN_CENTER)
    {
        fFront = static_cast<float>(pow(PAN_UNIT - fFade, PAN_CURVE_EXPONENT));
        fBack = static_cast<float>(pow(fFade, PAN_CURVE_EXPONENT));
    }

    if (fPan == fFade)
    {
        float fCurve = pSample->volumeCurve;
        pSample->mixLevel2 = fCurve * fCenter;
        pSample->panLevelB = (fCurve * fBack) * fFront;
        pSample->panLevelA = (fCurve * fFront) * fFront;
        pSample->mixLevel0 = (fCurve * fFront) * fBack;
        pSample->mixLevel1 = (fCurve * fBack) * fBack;
        pSample->mixLevel3 = fCurve * fLowFrequency;
    }
    else if (fPan == PAN_CENTER)
    {
        float fCurve = pSample->volumeCurve;
        float fFrontLevel = (fCurve * PAN_CENTER_GAIN) * fFront;
        float fBackLevel = (fCurve * PAN_CENTER_GAIN) * fBack;
        pSample->panLevelA = fFrontLevel;
        pSample->panLevelB = fFrontLevel;
        pSample->mixLevel0 = fBackLevel;
        pSample->mixLevel2 = fCurve * fCenter;
        pSample->mixLevel1 = fBackLevel;
        pSample->mixLevel3 = fCurve * fLowFrequency;
    }
    else
    {
        float fLeft = pSample->volumeCurve * static_cast<float>(pow(PAN_UNIT - fPan, PAN_CURVE_EXPONENT));
        float fRight = static_cast<float>(pow(fPan, PAN_CURVE_EXPONENT));
        float fCurve = pSample->volumeCurve;
        pSample->panLevelA = fLeft * fFront;
        pSample->mixLevel0 = fLeft * fBack;
        pSample->panLevelB = (fCurve * fRight) * fFront;
        pSample->mixLevel2 = fCurve * fCenter;
        pSample->mixLevel1 = (fCurve * fRight) * fBack;
        pSample->mixLevel3 = fCurve * fLowFrequency;
    }
}

extern "C" int AIL_API_sample_processor(_SAMPLE* pSample, int iStage)
{
    if (iStage == SAMPLE_STAGE_ALL)
    {
        iStage = SAMPLE_STAGE_DECODER;
    }

    if (pSample->stages[iStage].filter.active == 0)
    {
        return 0;
    }

    return pSample->stages[iStage].filter.provider;
}

extern "C" int AIL_API_install_bus_compressor(_DIG_DRIVER* pDriver, int iBus, int iStage, int iInputBus)
{
    _SAMPLE** ppBuses = pDriver->busSamples;
    if (ppBuses[iBus] == NULL || ppBuses[iInputBus] == NULL)
    {
        AIL_set_error(BUS_INDEX_ERROR);
        return 0;
    }

    int iProvider = 0;
    if (AIL_find_filter("Compressor Filter", &iProvider) == 0)
    {
        AIL_set_error(COMPRESSOR_NOT_FOUND_ERROR);
        return 0;
    }

    AIL_set_sample_processor(ppBuses[iBus], iStage, iProvider);
    ppBuses[iBus]->compressorLink = (iStage << BUS_COMPRESSOR_STAGE_SHIFT) | BUS_COMPRESSOR_LINK_FLAGS;
    ppBuses[iInputBus]->compressorSources = ppBuses[iInputBus]->compressorSources | (1 << iBus);
    return 1;
}

extern "C" int AIL_API_sample_channel_count(_SAMPLE* pSample, unsigned int* puMask)
{
    if (pSample != NULL)
    {
        if (puMask != NULL)
        {
            *puMask = pSample->channelMask;
        }

        return pSample->channelCount;
    }

    return 0;
}

extern "C" int AIL_API_sample_playback_rate(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return 0;
    }

    return pSample->playbackRate;
}

extern "C" void AIL_API_set_sample_playback_rate(_SAMPLE* pSample, int iRate)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iRate < 1)
    {
        return;
    }

    if (iRate != pSample->playbackRate)
    {
        pSample->cacheState = -1;
    }

    pSample->playbackRate = iRate;
}

extern "C" void AIL_API_set_sample_3D_cone(_SAMPLE* pSample, float fInnerAngle, float fOuterAngle, float fOuterVolume)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->coneInner = fInnerAngle / CONE_ANGLE_DIVISOR;
    pSample->coneOuterVolume = fOuterVolume;
    int iEnabled = 0;
    pSample->coneOuter = fOuterAngle / CONE_ANGLE_DIVISOR;
    if (fInnerAngle < CONE_FULL_ANGLE && fOuterVolume < CONE_FULL_VOLUME)
    {
        iEnabled = 1;
    }

    pSample->coneEnabled = iEnabled;
}

extern "C" void AIL_API_set_sample_3D_orientation(_SAMPLE* pSample, float fForwardX, float fForwardY, float fForwardZ,
                                                  float fUpX, float fUpY, float fUpZ)
{
    if (pSample == NULL)
    {
        return;
    }

    float fLength = static_cast<float>(sqrt(fForwardZ * fForwardZ + (fForwardY * fForwardY + fForwardX * fForwardX)));
    if (fLength > DIRECTION_EPSILON)
    {
        fForwardY = fForwardY / fLength;
        fForwardX = fForwardX / fLength;
        fForwardZ = fForwardZ / fLength;
    }

    fLength = static_cast<float>(sqrt(fUpZ * fUpZ + (fUpY * fUpY + fUpX * fUpX)));
    if (fLength > DIRECTION_EPSILON)
    {
        fUpY = fUpY / fLength;
        fUpX = fUpX / fLength;
        fUpZ = fUpZ / fLength;
    }

    pSample->forwardX = fForwardX;
    pSample->upY = fUpY;
    pSample->upZ = fUpZ;
    pSample->upX = fUpX;
    pSample->forwardZ = fForwardZ;
    pSample->forwardY = fForwardY;
}

extern "C" void AIL_API_set_sample_3D_velocity_vector(_SAMPLE* pSample, float fX, float fY, float fZ)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->velocityX = fX;
    pSample->velocityY = fY;
    pSample->velocityZ = fZ;
}

extern "C" void AIL_API_set_sample_3D_velocity(_SAMPLE* pSample, float fDirectionX, float fDirectionY,
                                               float fDirectionZ, float fMagnitude)
{
    AIL_API_set_sample_3D_velocity_vector(pSample, fDirectionX * fMagnitude, fDirectionY * fMagnitude,
                                          fDirectionZ * fMagnitude);
}

extern "C" void AIL_API_set_sample_obstruction(_SAMPLE* pSample, float fObstruction)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->obstruction = fObstruction;
}

extern "C" void AIL_API_set_sample_exclusion(_SAMPLE* pSample, float fExclusion)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->exclusion = fExclusion;
}

extern "C" void AIL_API_set_sample_occlusion(_SAMPLE* pSample, float fOcclusion)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->occlusion = fOcclusion;
    AIL_set_sample_low_pass_cut_off(pSample, ALL_CHANNELS, OCCLUSION_LOW_PASS_BASE - fOcclusion);
}

extern "C" FLT_INSTANCE* FLT_find_provider_instance(unsigned int uProvider, _DIG_DRIVER* pDriver)
{
    FLT_INSTANCE* pInstance = FLT_list;
    while (pInstance != NULL)
    {
        if (pInstance->driver == pDriver && pInstance->provider == uProvider)
        {
            break;
        }

        pInstance = pInstance->next;
    }

    return pInstance;
}

extern "C" int AIL_find_filter(const char* pName, int* piProvider)
{
    unsigned int uIndex = RIB_ENUMERATE_FIRST;
    unsigned int uProvider;
    unsigned int uFlags;
    RIB_PROVIDER_PROPERTY_CALLBACK pProperty;
    const char* pPipelineName;
    const char* pVoiceName;
    RIB_INTERFACE_ENTRY entries[2] = {{RIB_PROPERTY, "Flags", &uFlags, 0},
                                      {RIB_FUNCTION, "PROVIDER_property", &pProperty, 0}};
    pProperty = NULL;

    while (RIB_enumerate_providers(PIPELINE_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 2, entries);
        if (pProperty(RIB_PROVIDER_NAME, &pPipelineName, 0, 0) != 0 && strcmp(pPipelineName, pName) == 0)
        {
            if (piProvider != NULL)
            {
                *piProvider = uProvider;
            }
            return 1;
        }
    }

    uIndex = RIB_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(VOICE_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        RIB_request_interface(uProvider, VOICE_FILTER_INTERFACE, 2, entries);
        if (pProperty(RIB_PROVIDER_NAME, &pVoiceName, 0, 0) != 0 && strcmp(pVoiceName, pName) == 0)
        {
            if (piProvider != NULL)
            {
                *piProvider = uProvider;
            }
            return 1;
        }
    }

    return 0;
}

extern "C" int AIL_API_open_filter(unsigned int uProvider, _DIG_DRIVER* pDriver)
{
    FLT_INSTANCE* pExisting = FLT_list;
    while (pExisting != NULL)
    {
        if (pExisting->driver == pDriver && pExisting->provider == uProvider)
        {
            return -1;
        }

        pExisting = pExisting->next;
    }

    FLT_INSTANCE* pFilter = static_cast<FLT_INSTANCE*>(
        AIL_mem_alloc_lock_info(sizeof(FLT_INSTANCE), MILES_FILTER_FILE, FILTER_INSTANCE_LINE));
    if (pFilter == NULL)
    {
        AIL_set_error(OUT_OF_MEMORY_ERROR);
        return 0;
    }

    memset(pFilter, 0, sizeof(FLT_INSTANCE));

    unsigned int uFlagsToken = 0;
    unsigned int uSampleStateSizeToken = RIB_ENTRY_NOT_FOUND;
    unsigned int uDriverStateSizeToken = RIB_ENTRY_NOT_FOUND;

    RIB_INTERFACE_ENTRY startupEntries[4] = {{RIB_FUNCTION, "PROVIDER_property", &pFilter->property, 0},
                                             {RIB_PROPERTY, "Flags", &uFlagsToken, 0},
                                             {RIB_FUNCTION, "FLT_startup", &pFilter->startup, 0},
                                             {RIB_FUNCTION, "FLT_error", &pFilter->error, 0}};
    RIB_INTERFACE_ENTRY driverEntries[3] = {{RIB_FUNCTION, "FLT_shutdown", &pFilter->shutdown, 0},
                                            {RIB_FUNCTION, "FLT_open_driver", &pFilter->openDriver, 0},
                                            {RIB_FUNCTION, "FLT_close_driver", &pFilter->closeDriver, 0}};
    RIB_INTERFACE_ENTRY mixEntries[2] = {{RIB_FUNCTION, "FLT_premix_process", &pFilter->premixProcess, 0},
                                         {RIB_FUNCTION, "FLT_postmix_process", &pFilter->postmixProcess, 0}};
    RIB_INTERFACE_ENTRY sizeEntries[2] = {{RIB_PROPERTY, "Sample_state_size", &uSampleStateSizeToken, 0},
                                          {RIB_PROPERTY, "Driver_state_size", &uDriverStateSizeToken, 0}};
    RIB_INTERFACE_ENTRY sampleEntries[4] = {{RIB_FUNCTION, "FLTSMP_open_sample", &pFilter->openSample, 0},
                                            {RIB_FUNCTION, "FLTSMP_close_sample", &pFilter->closeSample, 0},
                                            {RIB_FUNCTION, "FLTSMP_sample_process", &pFilter->sampleProcess, 0},
                                            {RIB_FUNCTION, "FLTSMP_sample_property", &pFilter->sampleProperty, 0}};

    int iStartup = RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 4, startupEntries);
    int iDriver = RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 3, driverEntries);
    int iMix = RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 2, mixEntries);
    if (iStartup != 0 || iDriver != 0 || iMix != 0)
    {
        AIL_set_error(NOT_A_FILTER_ERROR);
        AIL_mem_free_lock(pFilter);
        return 0;
    }

    RIB_request_interface(uProvider, PIPELINE_FILTER_INTERFACE, 2, sizeEntries);
    RIB_request_interface(uProvider, FILTER_SAMPLE_INTERFACE, 4, sampleEntries);

    pFilter->flags = 0;
    pFilter->property(uFlagsToken, &pFilter->flags, 0, 0);
    pFilter->sampleStateSize = 0;
    if (uSampleStateSizeToken != RIB_ENTRY_NOT_FOUND)
    {
        pFilter->property(uSampleStateSizeToken, &pFilter->sampleStateSize, 0, 0);
    }

    pFilter->driverStateSize = 0;
    if (uDriverStateSizeToken != RIB_ENTRY_NOT_FOUND)
    {
        pFilter->property(uDriverStateSizeToken, &pFilter->driverStateSize, 0, 0);
    }

    if ((pFilter->flags & FILTER_FLAG_MATRIX) != 0)
    {
        if (pDriver->matrixFilter != NULL)
        {
            AIL_set_error(MATRIX_FILTER_ERROR);
            AIL_mem_free_lock(pFilter);
            return 0;
        }

        pDriver->matrixFilter = pFilter;
    }

    OutMilesMutex();
    void* pDriverState = NULL;
    if (pFilter->driverStateSize != 0)
    {
        pDriverState = AIL_mem_alloc_lock_info(pFilter->driverStateSize, MILES_FILTER_FILE, FILTER_DRIVER_STATE_LINE);
    }

    pFilter->driverState = pFilter->openDriver(MSS_alloc_info, MSS_free_info, 0, pDriver, pDriverState);
    InMilesMutex();

    if (pFilter->driverState != NULL)
    {
        pFilter->driver = pDriver;
        pFilter->provider = uProvider;
        pFilter->next = FLT_list;
        FLT_list = pFilter;
        return static_cast<int>(reinterpret_cast<intptr_t>(pFilter));
    }

    const char* pError = pFilter->error();
    if (pError == NULL)
    {
        pError = FILTER_LOAD_ERROR;
    }

    AIL_set_error(pError);
    AIL_mem_free_lock(pFilter);
    return 0;
}

unsigned char* assign_ptr(unsigned char** ppCursor, unsigned int uSize)
{
    unsigned char* pResult = *ppCursor;
    *ppCursor = pResult + ((uSize + CHANNEL_ARRAY_ALIGNMENT - 1) & ~(CHANNEL_ARRAY_ALIGNMENT - 1));
    return pResult;
}

unsigned int assign_chan_ptrs(_SAMPLE* pSample, unsigned char* pBuffer, int iKeepLevelArrays)
{
    int iChannels = pSample->channelCount;
    unsigned char* pCursor = reinterpret_cast<unsigned char*>(
        (reinterpret_cast<uintptr_t>(pBuffer) + CHANNEL_BUFFER_ALIGNMENT - 1) & ~static_cast<uintptr_t>(CHANNEL_BUFFER_ALIGNMENT - 1));
    unsigned int uRows = (pSample->driver->speakerConfig + 1) & ~1u;
    float* pScratch[CHANNEL_SCRATCH_ROWS];
    int* pScratchInt[CHANNEL_SCRATCH_ROWS];

    pSample->channelStates = reinterpret_cast<ChannelState*>(assign_ptr(&pCursor, iChannels * sizeof(ChannelState)));

    float** ppPreviousLevels = pSample->levels;
    int** ppPreviousC = pSample->levelsC;
    int** ppPreviousB = pSample->levelsB;
    int** ppPreviousD = pSample->levelsD;

    pSample->levels = reinterpret_cast<float**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));
    pSample->levelsB = reinterpret_cast<int**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));
    pSample->levelsC = reinterpret_cast<int**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));
    pSample->levelsD = reinterpret_cast<int**>(assign_ptr(&pCursor, (uRows + 1) * sizeof(float*)));

    float** ppLevels = ppPreviousLevels;
    int** ppLevelsB = ppPreviousB;
    int** ppLevelsC = ppPreviousC;
    int** ppLevelsD = ppPreviousD;
    if (iKeepLevelArrays == 0)
    {
        ppLevelsC = pSample->levelsC;
        ppLevelsB = pSample->levelsB;
        ppLevels = pSample->levels;
        ppLevelsD = pSample->levelsD;
    }

    if (pBuffer == NULL)
    {
        ppLevelsD = pScratchInt;
        ppLevels = pScratch;
        ppLevelsB = pScratchInt;
        ppLevelsC = pScratchInt;
    }

    for (unsigned int uRow = 0; uRow < uRows; ++uRow)
    {
        ppLevels[uRow] = reinterpret_cast<float*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
        ppLevelsB[uRow] = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
        ppLevelsC[uRow] = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
        ppLevelsD[uRow] = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(float)));
    }

    pSample->channelPositionA = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(int)));
    pSample->channelPositionB = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(int)));
    pSample->channelHeader = reinterpret_cast<int*>(assign_ptr(&pCursor, iChannels * sizeof(int)));
    pSample->speakerLevelsA = reinterpret_cast<float*>(assign_ptr(&pCursor, uRows * sizeof(float)));
    pSample->speakerScales = reinterpret_cast<float*>(assign_ptr(&pCursor, uRows * sizeof(float)));
    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pCursor));
}

void free_channel_buffers(_SAMPLE* pSample)
{
    if (pSample->channelBuffer != NULL)
    {
        AIL_mem_free_lock(pSample->channelBuffer);
        pSample->channelBuffer = NULL;
    }

    pSample->channelBufferCapacity = 0;
    pSample->channelBufferSize = 0;

    if (pSample->channelMap != NULL)
    {
        AIL_mem_free_lock(pSample->channelMap);
    }
}

void clear_last_sample(_SAMPLE* pSample)
{
    for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
    {
        pSample->channelPositionA[iChannel] = 0;
        pSample->channelPositionB[iChannel] = 0;
    }

    *pSample->channelHeader = CHANNEL_HEADER_UNITY;
    pSample->resampleFraction = 0;
}

void set_user_channel_defaults(int iSpeakerConfig, int iChannelCount, float** ppLevels, signed char* pChannelMap)
{
    if (iSpeakerConfig == 0)
    {
        return;
    }

    const signed char* pSpeakers = SPEAKER_CHANNEL_MAP[iSpeakerConfig];

    for (int iSpeaker = 0; iSpeaker < iSpeakerConfig; ++iSpeaker)
    {
        for (int iChannel = 0; iChannel < iChannelCount; ++iChannel)
        {
            ppLevels[iSpeaker][iChannel] = 0.0f;
        }
    }

    if (iChannelCount == 2)
    {
        ppLevels[0][0] = 1.0f;
        ppLevels[1][1] = 1.0f;
    }
    else if (iChannelCount == 1)
    {
        ppLevels[0][0] = 1.0f;
        ppLevels[1][0] = 1.0f;
    }
    else
    {
        if (iSpeakerConfig == 1)
        {
            for (int iId = 0; iId < SPEAKER_ID_COUNT; ++iId)
            {
                if ((SPEAKER_TRAITS[iId] & SPEAKER_TRAIT_LOW_FREQUENCY) != 0)
                {
                    continue;
                }

                int iSource;
                if (pChannelMap == NULL)
                {
                    if (iId >= iChannelCount)
                    {
                        continue;
                    }
                    iSource = iId;
                }
                else
                {
                    iSource = pChannelMap[iId];
                }

                if (iSource != -1)
                {
                    ppLevels[0][iSource] = 1.0f;
                }
            }
        }
        else
        {
            for (int iId = 0; iId < SPEAKER_ID_COUNT; ++iId)
            {
                int iSource;
                if (pChannelMap == NULL)
                {
                    if (iId >= iChannelCount)
                    {
                        continue;
                    }
                    iSource = iId;
                }
                else
                {
                    iSource = pChannelMap[iId];
                    if (iSource == -1)
                    {
                        continue;
                    }
                }

                int iSpeaker = pSpeakers[iId];
                if (iSpeaker != SPEAKER_UNMAPPED)
                {
                    ppLevels[iSpeaker][iSource] = 1.0f;
                    continue;
                }

                unsigned int uShared = SPEAKER_TRAITS[iId] & SPEAKER_TRAIT_SHARED;
                int iAssigned = 0;
                for (int iOther = 0; iOther < SPEAKER_ID_COUNT; ++iOther)
                {
                    if ((SPEAKER_TRAITS[iOther] & uShared) != 0 && pSpeakers[iOther] != SPEAKER_UNMAPPED)
                    {
                        ++iAssigned;
                        ppLevels[pSpeakers[iOther]][iSource] = 1.0f;
                    }
                }

                if (iAssigned == 0)
                {
                    unsigned int uTraits = SPEAKER_TRAITS[iId];
                    int iOther = 0;
                    while (true)
                    {
                        if ((uTraits & SPEAKER_TRAIT_LOW_FREQUENCY) == 0 && pSpeakers[iOther] != SPEAKER_UNMAPPED)
                        {
                            ppLevels[pSpeakers[iOther]][iSource] = 1.0f;
                        }

                        if (iOther + 1 > SPEAKER_ID_LAST)
                        {
                            break;
                        }

                        int iNext = iOther + 1;
                        while ((SPEAKER_TRAITS[iNext] & SPEAKER_TRAIT_SIDES) == 0)
                        {
                            ++iNext;
                            if (iNext > SPEAKER_ID_LAST)
                            {
                                break;
                            }
                        }

                        if (iNext > SPEAKER_ID_LAST)
                        {
                            break;
                        }

                        iOther = iNext;
                    }
                }
            }
        }

        for (int iId = 0; iId < SPEAKER_ID_COUNT; ++iId)
        {
            int iSpeaker = pSpeakers[iId];
            if (iSpeaker == SPEAKER_UNMAPPED)
            {
                continue;
            }

            int iContributors = 0;
            for (int iChannel = 0; iChannel < iChannelCount; ++iChannel)
            {
                if (ppLevels[iSpeaker][iChannel] > 0.0f)
                {
                    ++iContributors;
                }
            }

            if (iContributors > 1)
            {
                float fScale = 1.0f / static_cast<float>(iContributors);
                for (int iChannel = 0; iChannel < iChannelCount; ++iChannel)
                {
                    ppLevels[iSpeaker][iChannel] = ppLevels[iSpeaker][iChannel] * fScale;
                }
            }
        }
    }
}

extern "C" int SS_set_sample_type(_SAMPLE* pSample, unsigned int uFlags, int iChannelMask)
{
    if (pSample == NULL)
    {
        return 0;
    }

    unsigned int uFormat = uFlags & SAMPLE_FORMAT_MASK;
    int iChannels = static_cast<int>(uFlags) >> SAMPLE_FORMAT_CHANNEL_SHIFT;
    if ((uFormat & SAMPLE_FORMAT_STEREO) != 0)
    {
        iChannels = 2;
    }
    else if ((uFormat & SAMPLE_FORMAT_MULTICHANNEL) == 0)
    {
        iChannels = 1;
    }

    if (uFormat != pSample->channelFlags || iChannelMask != pSample->channelMask || iChannels != pSample->channelCount)
    {
        pSample->channelFlags = uFormat;
        pSample->channelCount = iChannels;
        pSample->channelMask = iChannelMask;

        int iConfig = pSample->driver->speakerConfig;
        unsigned int uRequired = assign_chan_ptrs(pSample, NULL, 0);
        unsigned int uSize = (uRequired + CHANNEL_BUFFER_SLACK) & ~(CHANNEL_BUFFER_ALIGNMENT - 1);
        if (pSample->channelBufferCapacity < uSize || uSize < (pSample->channelBufferCapacity >> 1))
        {
            free_channel_buffers(pSample);
        }

        if (pSample->channelBuffer == NULL)
        {
            pSample->channelBuffer = static_cast<unsigned char*>(
                AIL_mem_alloc_lock_info(uSize, MILES_WAVEFILE_FILE, CHANNEL_BUFFER_LINE));
            if (pSample->channelBuffer == NULL)
            {
                AIL_set_error(CHANNEL_BUFFER_ERROR);
                return 0;
            }

            pSample->channelBufferCapacity = uSize;
        }

        pSample->channelBufferSize = uSize;
        memset(pSample->channelBuffer, 0, uSize);
        assign_chan_ptrs(pSample, pSample->channelBuffer, 0);

        for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
        {
            pSample->channelStates[iChannel].lowPass = CHANNEL_LOW_PASS_DEFAULT;
            pSample->channelStates[iChannel].appliedLowPass = CHANNEL_BUS_LEVEL_DEFAULT;
            pSample->channelStates[iChannel].limitB = CHANNEL_LIMIT_DEFAULT;
            pSample->channelStates[iChannel].lowPassInfo.coefficientA = CHANNEL_LIMIT_DEFAULT;
        }

        for (int iSpeaker = 0; iSpeaker < pSample->driver->speakerConfig; ++iSpeaker)
        {
            pSample->speakerLevelsA[iSpeaker] = CHANNEL_LOW_PASS_DEFAULT;
            pSample->speakerScales[iSpeaker] = CHANNEL_LOW_PASS_DEFAULT;
        }

        clear_last_sample(pSample);

        if (iConfig != 0)
        {
            if (pSample->channelMap != NULL)
            {
                AIL_mem_free_lock(pSample->channelMap);
            }

            signed char* pMap = NULL;
            unsigned int uMask = pSample->channelMask;
            pSample->channelMap = NULL;
            if (uMask != 0)
            {
                bool bHasGap = false;
                while (uMask != 0)
                {
                    if ((uMask & 1) == 0)
                    {
                        bHasGap = true;
                    }
                    uMask = uMask >> 1;
                }

                if (bHasGap)
                {
                    pMap = static_cast<signed char*>(
                        AIL_mem_alloc_lock_info(CHANNEL_MAP_LENGTH, MILES_WAVEFILE_FILE, CHANNEL_MAP_LINE));
                    memset(pMap, -1, CHANNEL_MAP_LENGTH);

                    int iNext = 0;
                    unsigned int uBit = 1;
                    for (int iId = 0; iId < CHANNEL_MASK_BITS; ++iId)
                    {
                        if (iNext >= iChannels || iId >= CHANNEL_MAP_LENGTH)
                        {
                            break;
                        }

                        if ((pSample->channelMask & uBit) != 0)
                        {
                            pMap[iId] = static_cast<signed char>(iNext);
                            iNext = static_cast<signed char>(iNext + 1);
                        }

                        uBit = uBit << 1;
                    }

                    pSample->channelMap = pMap;
                }
            }

            set_user_channel_defaults(pSample->driver->speakerConfig, pSample->channelCount, pSample->levels, pMap);
        }

        if ((uFormat & SAMPLE_FORMAT_ADPCM) != 0)
        {
            pSample->resetA = 0;
            pSample->resetB = 0;
        }
    }

    return 1;
}

int SP_ASI_DECODER_callback(unsigned int uUser, void* pDestination, int iCount, int iOffset)
{
    _SAMPLE* pSample = reinterpret_cast<_SAMPLE*>(uUser);
    int iCurrent = pSample->currentBuffer;
    SampleBuffer* pBuffer = &pSample->buffers[iCurrent];

    if (pBuffer->size < iOffset)
    {
        iOffset = pBuffer->size;
    }

    if (iOffset != -1)
    {
        pBuffer->position = iOffset;
    }

    unsigned int uPosition;
    unsigned int uEnd;
    if (pSample->bufferRewind == 0)
    {
        uPosition = pBuffer->position;
        uEnd = pBuffer->size;
    }
    else
    {
        pSample->bufferRewind = 0;
        uPosition = pBuffer->size;
        pBuffer->position = uPosition;
        uEnd = uPosition;
    }

    if (pSample->loopMode == LOOP_MODE_ASI && pSample->loopState != 1)
    {
        unsigned int uLoopEnd = pSample->loopEnd;
        if (uLoopEnd != ALL_BITS_SET
            && uLoopEnd < static_cast<unsigned int>(pSample->buffers[pSample->currentBuffer].size))
        {
            uEnd = uLoopEnd;
        }

        if (uEnd <= uPosition)
        {
            return 0;
        }
    }

    int iCopied = iCount;
    if (uEnd < uPosition + iCount)
    {
        iCopied = uEnd - uPosition;
    }

    memcpy(pDestination, pBuffer->data + uPosition, iCopied);
    pDestination = static_cast<unsigned char*>(pDestination) + iCopied;
    iCount = iCount - iCopied;
    pBuffer->position = pBuffer->position + iCopied;

    while (iCount > 0)
    {
        if (pSample->endOfBuffer != NULL)
        {
            pSample->endCounter = pSample->endCounter + 1;
        }

        if (pSample->loopMode != LOOP_MODE_STREAMING)
        {
            return iCopied;
        }

        if (pBuffer->reset != 0)
        {
            return iCopied;
        }

        iCurrent = (iCurrent + 1) % pSample->bufferCount;
        pBuffer = &pSample->buffers[iCurrent];
        if (pBuffer->done != 0)
        {
            break;
        }

        if (iCurrent == pSample->lastBuffer || pBuffer->size == 0 || pBuffer->position != 0)
        {
            pSample->starved = 1;
            return iCopied;
        }

        pSample->currentBuffer = iCurrent;
        int iPosition = pBuffer->position;
        int iChunk = iCount;
        if (static_cast<unsigned int>(pBuffer->size) < static_cast<unsigned int>(iPosition + iCount))
        {
            iChunk = pBuffer->size - iPosition;
        }

        memcpy(pDestination, pBuffer->data + iPosition, iChunk);
        pDestination = static_cast<unsigned char*>(pDestination) + iChunk;
        iCount = iCount - iChunk;
        iCopied = iCopied + iChunk;
        pBuffer->position = pBuffer->position + iChunk;
    }

    return iCopied;
}

extern "C" int AIL_API_set_sample_processor(_SAMPLE* pSample, int iStage, int iProvider)
{
    unsigned int uIndex = iStage;
    if (iStage == SAMPLE_STAGE_ALL)
    {
        uIndex = SAMPLE_STAGE_DECODER;
    }

    SampleStage* pStage;
    do
    {
        pStage = &pSample->stages[uIndex];
        if (pStage->filter.active != 0)
        {
            if (uIndex == SAMPLE_STAGE_DECODER)
            {
                pStage->decoder.close(pStage->decoder.stream);
                pStage->decoder.active = 0;
            }
            else if (uIndex < SAMPLE_STAGE_MERGE)
            {
                FilterStage& filter = pStage->filter;
                for (int iHandle = 0; iHandle < SAMPLE_FILTER_HANDLE_COUNT; ++iHandle)
                {
                    if (filter.handles[iHandle] != NULL)
                    {
                        filter.instance->closeSample(filter.handles[iHandle]);
                        if (filter.handles[iHandle] != NULL)
                        {
                            AIL_mem_free_lock(filter.handles[iHandle]);
                            filter.handles[iHandle] = NULL;
                        }
                    }
                }

                pSample->filterCount = pSample->filterCount - 1;
                filter.active = 0;
            }
            else
            {
                pStage->merge.active = 0;
            }
        }

        if (iStage != SAMPLE_STAGE_ALL)
        {
            break;
        }

        ++uIndex;
    } while (uIndex < SAMPLE_STAGE_COUNT);

    int iPrevious = pStage->filter.provider;
    pStage->filter.provider = iProvider;
    if (iProvider == 0)
    {
        return iPrevious;
    }

    if (uIndex != SAMPLE_STAGE_DECODER)
    {
        if (uIndex > SAMPLE_FILTER_STAGE_COUNT)
        {
            if (uIndex != SAMPLE_STAGE_MERGE)
            {
                return iPrevious;
            }

            RIB_INTERFACE_ENTRY mergeEntry = {RIB_FUNCTION, "MIXER_merge", &pStage->merge.merge, 0};
            RIB_request_interface(pStage->merge.provider, MIXER_SERVICES_INTERFACE, 1, &mergeEntry);
            pStage->merge.active = 1;
            return iPrevious;
        }

        if (AIL_open_filter(iProvider, pSample->driver) != 0)
        {
            FLT_INSTANCE* pInstance = FLT_find_provider_instance(iProvider, pSample->driver);
            FilterStage& filter = pStage->filter;
            for (int iHandle = 0; iHandle < SAMPLE_FILTER_HANDLE_COUNT; ++iHandle)
            {
                filter.handles[iHandle] = NULL;
            }
            filter.instance = pInstance;

            if (pInstance != NULL && pInstance->openSample != NULL)
            {
                for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
                {
                    void* pState = NULL;
                    if (pInstance->sampleStateSize != 0)
                    {
                        pState = AIL_mem_alloc_lock_info(pInstance->sampleStateSize, MILES_WAVEFILE_FILE,
                                                         FILTER_STATE_LINE);
                    }

                    filter.handles[iChannel] = pInstance->openSample(pInstance->driverState, pSample, pState);
                    if (filter.handles[iChannel] == NULL)
                    {
                        pStage->filter.provider = 0;
                        return iPrevious;
                    }
                }

                filter.active = 1;
                pSample->filterCount = pSample->filterCount + 1;
                return iPrevious;
            }
        }

        pStage->filter.provider = 0;
        return iPrevious;
    }

    DecoderStage& decoder = pStage->decoder;
    RIB_INTERFACE_ENTRY streamEntries[4] = {{RIB_FUNCTION, "ASI_stream_property", &decoder.property, 0},
                                            {RIB_FUNCTION, "ASI_stream_open", &decoder.open, 0},
                                            {RIB_FUNCTION, "ASI_stream_seek", &decoder.seek, 0},
                                            {RIB_FUNCTION, "ASI_stream_close", &decoder.close, 0}};
    RIB_INTERFACE_ENTRY inputEntries[3] = {{RIB_FUNCTION, "ASI_stream_process", &decoder.process, 0},
                                           {RIB_PROPERTY, "Input bit rate", &decoder.inputBitRate, 0},
                                           {RIB_PROPERTY, "Input sample rate", &decoder.inputSampleRate, 0}};
    RIB_INTERFACE_ENTRY inputFormatEntries[4] = {{RIB_PROPERTY, "Input sample width", &decoder.inputSampleWidth, 0},
                                                 {RIB_PROPERTY, "Input channels", &decoder.inputChannels, 0},
                                                 {RIB_PROPERTY, "Output bit rate", &decoder.outputBitRate, 0},
                                                 {RIB_PROPERTY, "Output sample rate", &decoder.outputSampleRate, 0}};
    RIB_INTERFACE_ENTRY outputEntries[4] = {{RIB_PROPERTY, "Output sample width", &decoder.outputSampleWidth, 0},
                                            {RIB_PROPERTY, "Output channels", &decoder.outputChannels, 0},
                                            {RIB_PROPERTY, "Output reservoir", &decoder.outputReservoir, 0},
                                            {RIB_PROPERTY, "Position", &decoder.position, 0}};
    RIB_INTERFACE_ENTRY progressEntries[4] = {
        {RIB_PROPERTY, "Percent done", &decoder.percentDone, 0},
        {RIB_PROPERTY, "Minimum input block size", &decoder.minimumInputBlockSize, 0},
        {RIB_PROPERTY, "Raw source sample rate", &decoder.rawSourceSampleRate, 0},
        {RIB_PROPERTY, "Raw source sample width", &decoder.rawSourceSampleWidth, 0}};
    RIB_INTERFACE_ENTRY requestEntries[4] = {{RIB_PROPERTY, "Raw source channels", &decoder.rawSourceChannels, 0},
                                             {RIB_PROPERTY, "Requested sample rate", &decoder.requestedSampleRate, 0},
                                             {RIB_PROPERTY, "Requested sample width", &decoder.requestedSampleWidth, 0},
                                             {RIB_PROPERTY, "Requested channels", &decoder.requestedChannels, 0}};
    RIB_INTERFACE_ENTRY layoutEntries[4] = {{RIB_PROPERTY, "Output channel mask", &decoder.outputChannelMask, 0},
                                            {RIB_PROPERTY, "Stream seek position", &decoder.streamSeekPosition, 0},
                                            {RIB_PROPERTY, "Data offset", &decoder.dataOffset, 0},
                                            {RIB_PROPERTY, "Data length", &decoder.dataLength, 0}};
    RIB_INTERFACE_ENTRY seekEntries[5] = {{RIB_PROPERTY, "Exact seek", &decoder.exactSeek, 0},
                                          {RIB_PROPERTY, "Exact get position", &decoder.exactGetPosition, 0},
                                          {RIB_PROPERTY, "Seek table lookup", &decoder.seekTableLookup, 0},
                                          {RIB_PROPERTY, "Set Looping Samples", &decoder.setLoopingSamples, 0},
                                          {RIB_PROPERTY, "Clear Loop Meta", &decoder.clearLoopMeta, 0}};

    unsigned int uProfile = RIB_ENTRY_NOT_FOUND;
    RIB_INTERFACE_ENTRY profileEntry = {RIB_PROPERTY, "Profile index", &uProfile, 0};

    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, streamEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 3, inputEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, inputFormatEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, outputEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, progressEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, requestEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 4, layoutEntries);
    RIB_request_interface(decoder.provider, ASI_STREAM_INTERFACE, 5, seekEntries);

    decoder.stream = decoder.open(MSS_alloc_info, MSS_free_info, static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pSample)),
                                  SP_ASI_DECODER_callback, pSample->buffers[0].size);

    RIB_request_interface(decoder.provider, ASI_CODEC_INTERFACE, 1, &profileEntry);
    uProfile = uProfile - ASI_PROFILE_BASE;
    if (uProfile >= ASI_PROFILE_LIMIT)
    {
        uProfile = 0;
    }

    if (decoder.stream == NULL)
    {
        decoder.provider = 0;
        return iPrevious;
    }

    _DIG_DRIVER* pDriver = pSample->driver;
    decoder.property(decoder.stream, decoder.requestedSampleRate, 0, &pDriver->outputRate, 0);

    int iRequestedChannels = ASI_DEFAULT_CHANNELS - ((pDriver->flags & DRIVER_FLAG_STEREO_OR_MULTICHANNEL) == 0);
    decoder.property(decoder.stream, decoder.requestedChannels, 0, &iRequestedChannels, 0);

    int iRequestedWidth = (pDriver->flags & DRIVER_FLAG_16_BITS) * SAMPLE_WIDTH_8 + SAMPLE_WIDTH_8;
    decoder.property(decoder.stream, decoder.requestedSampleWidth, 0, &iRequestedWidth, 0);

    unsigned int uOutputChannels = RIB_ENTRY_NOT_FOUND;
    decoder.property(decoder.stream, decoder.outputChannels, &uOutputChannels, 0, 0);
    int iOutputRate = -1;
    decoder.property(decoder.stream, decoder.outputSampleRate, &iOutputRate, 0, 0);
    int iOutputWidth = -1;
    decoder.property(decoder.stream, decoder.outputSampleWidth, &iOutputWidth, 0, 0);

    unsigned int uMask = 0;
    unsigned int uSampleChannels = AIL_sample_channel_count(pSample, &uMask);
    if (AIL_sample_playback_rate(pSample) != iOutputRate)
    {
        AIL_set_sample_playback_rate(pSample, iOutputRate);
    }

    int iResult;
    if (uOutputChannels < ASI_CHANNEL_LIMIT && pSample->channelMask == -1)
    {
        if (uOutputChannels != uSampleChannels)
        {
            if (uOutputChannels == 1)
            {
                iResult = SS_set_sample_type(pSample, iOutputWidth == SAMPLE_WIDTH_16, -1);
            }
            else
            {
                unsigned int uFormat = SAMPLE_FORMAT_STEREO;
                if (iOutputWidth == SAMPLE_WIDTH_16)
                {
                    uFormat = SAMPLE_FORMAT_STEREO | SAMPLE_FORMAT_16_BITS;
                }

                iResult = SS_set_sample_type(pSample, uFormat, -1);
            }

            if (iResult == 0)
            {
                return -1;
            }
        }
    }
    else
    {
        unsigned int uOutputMask = RIB_ENTRY_NOT_FOUND;
        if (decoder.outputChannelMask != RIB_ENTRY_NOT_FOUND)
        {
            decoder.property(decoder.stream, decoder.outputChannelMask, &uOutputMask, 0, 0);
        }

        if (uOutputMask != uMask || uSampleChannels != uOutputChannels)
        {
            iResult = SS_set_sample_type(pSample,
                                         (uOutputChannels << SAMPLE_FORMAT_CHANNEL_SHIFT) | SAMPLE_FORMAT_MULTICHANNEL
                                             | SAMPLE_FORMAT_16_BITS,
                                         uOutputMask);
            if (iResult == 0)
            {
                return -1;
            }
        }
    }

    if (pSample->loopMode == LOOP_MODE_ASI)
    {
        if (decoder.dataOffset != RIB_ENTRY_NOT_FOUND)
        {
            unsigned int uDataOffset = 0;
            decoder.property(decoder.stream, decoder.dataOffset, &uDataOffset, 0, 0);
            pSample->buffers[0].position = 0;
            pSample->buffers[0].data = pSample->buffers[0].data + uDataOffset;
        }

        if (decoder.dataLength != RIB_ENTRY_NOT_FOUND)
        {
            decoder.property(decoder.stream, decoder.dataLength, &pSample->buffers[0].size, 0, 0);
        }
    }

    decoder.active = uProfile + 1;
    return iPrevious;
}

namespace
{
const char WAV_RIFF_TYPE[] = "WAVE";
const char WAV_FORMAT_CHUNK[] = "fmt ";
const char WAV_DATA_CHUNK[] = "data";
const char WAV_FACT_CHUNK[] = "fact";
const char WAV_NOT_WAV_ERROR[] = "Not a WAV file.";
const char WAV_NOT_EXTENSIBLE_ERROR[] = "Not a WAVEFORMATPCMEX file.";
const char WAV_NOT_PCM_SUBTYPE_ERROR[] = "Not KSDATAFORMAT_SUBTYPE_PCM.";
const char WAV_MULTICHANNEL_ERROR[] = "Unsupported multichannel format.";
const char CREATIVE_VOC_SIGNATURE[] = "Creative";
const char OGG_SIGNATURE[] = "OggS";
const char SPEEX_SIGNATURE[] = "Speex";
const char INPUT_FILE_TYPES[] = "Input file types";
const char BINK_AUDIO_EXTENSION[] = ".binka";
const char OGG_EXTENSION[] = ".ogg";
const char SPEEX_EXTENSION[] = ".spx";
const char RAW_EXTENSION[] = ".raw";
const char MP3_EXTENSION[] = ".mp3";
const char WAV_EXTENSION[] = ".WAV";
const char VOC_EXTENSION[] = ".VOC";
const char ASI_CODEC_INTERFACE_NAME[] = "ASI codec";
const char INPUT_WAVE_TAG[] = "Input wave tag";
const char OUTPUT_FILE_TYPES[] = "Output file types";
const char BINK_PROVIDER_MISSING_ERROR[] = "Bink Audio provider not found.";
const char BINK_PROVIDER_FAILED_ERROR[] = "Bink Audio provider failed to initialize.";
const char UNSUPPORTED_WAVE_ERROR[] = "Unsupported wave file format.";
const char UNKNOWN_FILE_TYPE_ERROR[] = "Unknown file type.";
const char PROVIDER_MISSING_ERROR[] =
    "Couldn't find provider for this file type -- check your AIL_set_redist_directory() call.";
const char PROVIDER_FAILED_ERROR[] = "ASI provider for this file type failed to initialize";
const char OUT_OF_BUSSES_ERROR[] = "Out of available busses.";
const char BUS_BUFFER_ERROR[] = "Out of memory for bus buffer.";
const char OUT_OF_HANDLES_ERROR[] = "Out of sample handles.";
const char INVALID_INTERVAL_ERROR[] = "Invalid sample interval specified.";
const char LOOP_ACCURACY_ERROR[] = "Sample's ASI Decoder doesn't support looping at sample accuracy.";
const char MIXER_FLUSH_ENTRY[] = "MIXER_flush";
const char MIXER_COPY_ENTRY[] = "MIXER_copy";
const char MIXER_MULTICHANNEL_COPY_ENTRY[] = "MIXER_mc_copy";
const char MIXER_ADPCM_DECODE_ENTRY[] = "MIXER_adpcm_decode";

const unsigned int WAV_FORMAT_PCM = 1;
const unsigned int WAV_FORMAT_IMA_ADPCM = 0x11;
const unsigned int WAV_FORMAT_EXTENSIBLE = 0xfffe;
const unsigned int WAV_EXTENSION_SIZE_MINIMUM = 0x16;
const int WAV_SIGNATURE_LENGTH = 4;
const int WAV_HEADER_SIZE = 12;
const int WAV_CHUNK_HEADER_SIZE = 8;
const int WAV_CHUNK_SIZE_OFFSET = 4;
const int WAV_RIFF_TYPE_OFFSET = 8;
const int WAV_FORMAT_TAG_OFFSET = 8;
const int WAV_CHANNELS_OFFSET = 10;
const int WAV_RATE_OFFSET = 12;
const int WAV_BLOCK_ALIGN_OFFSET = 0x14;
const int WAV_BITS_OFFSET = 0x16;
const int WAV_EXTENSION_SIZE_OFFSET = 0x18;
const int WAV_CHANNEL_MASK_OFFSET = 0x1c;
const int WAV_SUBTYPE_OFFSET = 0x20;
const int WAV_SUBTYPE_SIZE = 16;
const int WAV_FACT_SAMPLES_OFFSET = 8;
const int WAV_BITS_PER_BYTE = 8;
const int WAV_IMA_ADPCM_BITS = 4;
const int WAV_IMA_ADPCM_HEADER_BASE = 4;
const unsigned char WAV_SUBTYPE_PCM[WAV_SUBTYPE_SIZE] = {1, 0, 0, 0, 0, 0, 16, 0, 128, 0, 0, 170, 0, 56, 155, 113};

const unsigned int BINK_AUDIO_TAG = 0x42434631;
const int BINK_AUDIO_SIZE_OFFSET = 0x10;
const int OGG_SPEEX_SCAN_LENGTH = 0x80;
const int SPEEX_SIGNATURE_LENGTH = 5;
const int OGG_SIGNATURE_LENGTH = 4;
const int CREATIVE_SIGNATURE_LENGTH = 8;
const int MP3_EXTENSION_LENGTH = 4;
const int ID3_HEADER_SIZE = 10;
const int ID3_SYNCSAFE_LIMIT = 0x80;
const int ID3_VERSION_LIMIT = 0xff;

const int VOC_HEADER_SIZE_OFFSET = 0x14;
const int VOC_BLOCK_TERMINATOR = 0;
const int VOC_BLOCK_SOUND_DATA = 1;
const int VOC_BLOCK_CONTINUATION = 2;
const int VOC_BLOCK_SILENCE = 3;
const int VOC_BLOCK_MARKER = 4;
const int VOC_BLOCK_TEXT = 5;
const int VOC_BLOCK_REPEAT = 6;
const int VOC_BLOCK_END_REPEAT = 7;
const int VOC_BLOCK_EXTENDED = 8;
const int VOC_BLOCK_NEW_DATA = 9;
const int VOC_REPEAT_FOREVER = 0xffff;
const int VOC_BLOCK_HEADER_SIZE = 4;
const int VOC_SOUND_DATA_HEADER = 6;
const int VOC_NEW_DATA_HEADER = 0x10;
const int VOC_NEW_DATA_TRAILER = 0xc;
const int VOC_RATE_NUMERATOR = 1000000;
const int VOC_RATE_BASE = 0x100;
const int VOC_EXTENDED_RATE_BASE = 0x10000;
const int VOC_EXTENDED_MONO_NUMERATOR = 256000000;
const int VOC_EXTENDED_STEREO_NUMERATOR = 128000000;
const int VOC_CODEC_PCM = 0;
const int VOC_CODEC_ADPCM = 4;

const int SAMPLE_CHANNEL_MASK_ALL = -1;
const int SAMPLE_FORMAT_MONO_16 = 1;
const int SAMPLE_FORMAT_STEREO_16 = 3;
const int SAMPLE_FORMAT_MULTICHANNEL_16 = 0x11;
const int SAMPLE_FORMAT_MONO_8 = 0;
const int SAMPLE_FORMAT_STEREO_8 = 2;
const int SAMPLE_STAGE_MIXER_COPY_PROVIDER = 1;
const int SAMPLE_STAGE_FILTER_FIRST = 1;
const int SAMPLE_STATUS_STOPPED = 8;
const int SAMPLE_STATUS_PLAYING_RELEASED = 0x10;
const int SAMPLE_DEFAULT_BUFFER_COUNT = 2;
const int SAMPLE_MINIMUM_BUFFER_COUNT = 2;
const int SAMPLE_MAXIMUM_BUFFER_COUNT = 8;
const int SAMPLE_DEFAULT_PLAYBACK_RATE = 0x2b11;
const int SAMPLE_DEFAULT_ADPCM_BLOCK_SIZE = 0x100;
const int SAMPLE_LOOP_CURRENT = -2;
const unsigned int SAMPLE_LOOP_CURRENT_UNSIGNED = 0xfffffffe;
const unsigned int SAMPLE_LOOP_TO_END = 0xffffffff;
const int SAMPLE_LOOP_FOREVER = 1;
const int SAMPLE_LOOP_INFINITE_END = -1;
const int SAMPLE_BUFFER_CURRENT = -1;
const int SAMPLE_DELAY_SHIFT = 8;
const int SAMPLE_PHASE_MASK = 0x3f;
const int SAMPLE_PROCESSOR_ALL = 7;
const int DRIVER_PROCESSOR_FLUSH = 0;
const int DRIVER_PROCESSOR_PROVIDER_FIRST = 1;
const int DRIVER_PROCESSOR_PROVIDER_SECOND = 2;
const int DRIVER_PROCESSOR_COPY = 3;
const int DRIVER_PROCESSOR_MULTICHANNEL_COPY = 4;
const int DRIVER_PROCESSOR_ADPCM_DECODE = 5;
const int DRIVER_PROCESSOR_ENTRY_COUNT = 1;
const unsigned int DRIVER_FLAG_NO_REVERB = 0x20;

const int PREFERENCE_RESAMPLE_FILTER = 4;
const int PREFERENCE_RESAMPLING_TOLERANCE = 5;
const int PREFERENCE_MINIMUM_BUFFER_MARGIN = 8;
const int PREFERENCE_RAMP_LENGTH = 9;
const int PREFERENCE_REVERB_MILLISECONDS = 10;

const float DEFAULT_LOW_PASS_CUT_OFF = 96000.0f;
const float DEFAULT_MAXIMUM_DISTANCE = 200.0f;
const float DEFAULT_CONE_ANGLE = 3.1415925f;
const float DEFAULT_ATTENUATION = 1.0f;
const float NO_FALLOFF_RESULT = -1.0f;
const float ZERO = 0.0f;
const float ONE = 1.0f;
const float HALF = 0.5f;
const float LEVEL_EPSILON = 0.0001f;
const float RATE_FACTOR_EPSILON = 0.0001f;
const float MILLISECONDS_PER_SECOND = 1000.0f;
const float PAN_RATIO_EXPONENT = 3.3333333f;
const float PAN_VOLUME_EXPONENT = -0.3f;
const float REVERB_LINE_SECONDS_A = 0.0297f;
const float REVERB_LINE_SECONDS_B = 0.0371f;
const float REVERB_LINE_SECONDS_C = 0.0411f;
const float REVERB_LINE_SECONDS_D = 0.0437f;
const float REVERB_LINE_SECONDS_E = 0.005f;
const float REVERB_LINE_SECONDS_F = 0.0017f;
const float REVERB_BYTES_PER_SAMPLE = 4.0f;
const double REVERB_DECAY_LEVEL = 0.001f;
const double REVERB_DECAY_EXPONENT_E = 0.0516243577003479;
const double REVERB_DECAY_EXPONENT_F = 0.05163406580686569;
const float REVERB_DAMPING_ANGLE = 0.125f;
const float REVERB_PI = 3.1415927f;
const double REVERB_SQRT_ARGUMENT = 2.0;
const int REVERB_ALIGNMENT = 16;
const int REVERB_BUS_FIRST_ALLOCATED = 1;
const int REVERB_ALLOCATION_LINE = 0x19e7;
const int BUS_BUFFER_LINE = 0x120d;
const int BUS_FIRST = 0;

const int MIXER_EFFECT_STAGE = 0;
const int ADPCM_BLOCK_HEADER_MONO = 4;
const int ADPCM_STEREO_SHIFT = 1;
const int ADPCM_BITS_PER_BYTE = 8;
const int FALLOFF_POLYLINE_MINIMUM = 2;

const int NIBBLES_MONO_8 = 2;
const int NIBBLES_MONO_16 = 4;
const int NIBBLES_STEREO_8 = 4;
const int NIBBLES_STEREO_16 = 8;
const int NIBBLES_ADPCM_MONO = 1;
const int NIBBLES_ADPCM_STEREO = 2;
const int NIBBLES_MULTICHANNEL_SHIFT = 2;
const unsigned int NIBBLE_FORMAT_MASK = 0xffff;
const int NIBBLE_CHANNEL_SHIFT = 16;
const int NIBBLE_FORMAT_ADPCM_MONO = 5;
const int NIBBLE_FORMAT_ADPCM_STEREO = 7;
const unsigned int NIBBLE_FORMAT_MULTICHANNEL = 0x11;

struct VocBlocks
{
    unsigned char* current;
    unsigned char* repeat;
};

unsigned int ReadLittleEndian32(const unsigned char* pBytes)
{
    return (static_cast<unsigned int>(pBytes[3]) << 24) | (static_cast<unsigned int>(pBytes[2]) << 16)
           | (static_cast<unsigned int>(pBytes[1]) << 8) | static_cast<unsigned int>(pBytes[0]);
}

unsigned int ReadLittleEndian16(const unsigned char* pBytes)
{
    return (static_cast<unsigned int>(pBytes[1]) << 8) | static_cast<unsigned int>(pBytes[0]);
}

const unsigned char* NextWavChunk(const unsigned char* pChunk)
{
    return pChunk + ReadLittleEndian32(pChunk + WAV_CHUNK_SIZE_OFFSET) + (pChunk[WAV_CHUNK_SIZE_OFFSET] & 1)
           + WAV_CHUNK_HEADER_SIZE;
}

VocBlocks* VocState(_SAMPLE* pSample)
{
    return reinterpret_cast<VocBlocks*>(&pSample->falloffHandle);
}
}

unsigned int AIL_VOC_block_len(void* pBlock)
{
    return *static_cast<unsigned int*>(pBlock) >> 8;
}

void reset_asi(_SAMPLE* pSample, int iSeekOffset, int iPosition)
{
    DecoderStage& decoder = pSample->stages[SAMPLE_STAGE_DECODER].decoder;
    if (decoder.active == 0)
    {
        return;
    }

    decoder.seek(decoder.stream, iSeekOffset);
    if (decoder.streamSeekPosition != RIB_ENTRY_NOT_FOUND)
    {
        int iValue = iPosition;
        decoder.property(decoder.stream, decoder.streamSeekPosition, 0, &iValue, &iValue);
    }
}

extern "C" void SS_update_driver_reverb_state(_DIG_DRIVER* pDriver)
{
    for (int iBus = 0; iBus < DRIVER_BUS_COUNT; ++iBus)
    {
        DriverBus* pBus = &pDriver->buses[iBus];
        if (pBus->sendLevel > REVERB_SEND_THRESHOLD && pBus->returnLevel > REVERB_SEND_THRESHOLD)
        {
            pBus->update = 1;
            pBus->updateValue = pDriver->sharedState;
        }
    }
}

extern "C" int AIL_API_WAV_info(const void* pData, _AILSOUNDINFO* pInfo)
{
    if (pData != NULL)
    {
        const unsigned char* pFile = static_cast<const unsigned char*>(pData);
        const unsigned char* pEnd = pFile + ReadLittleEndian32(pFile + WAV_CHUNK_SIZE_OFFSET);
        if (AIL_strnicmp(reinterpret_cast<const char*>(pFile + WAV_RIFF_TYPE_OFFSET), WAV_RIFF_TYPE,
                         WAV_SIGNATURE_LENGTH)
            == 0)
        {
            const unsigned char* pFirst = pFile + WAV_HEADER_SIZE;
            pInfo->channelMask = ALL_BITS_SET;
            const unsigned char* pFormat = pFirst;
            bool bFound = true;
            while (AIL_strnicmp(reinterpret_cast<const char*>(pFormat), WAV_FORMAT_CHUNK, WAV_SIGNATURE_LENGTH) != 0)
            {
                pFormat = NextWavChunk(pFormat);
                if (pFormat >= pEnd)
                {
                    bFound = false;
                    break;
                }
            }

            if (bFound)
            {
                pInfo->format = ReadLittleEndian16(pFormat + WAV_FORMAT_TAG_OFFSET);
                pInfo->rate = ReadLittleEndian32(pFormat + WAV_RATE_OFFSET);
                pInfo->bits = ReadLittleEndian16(pFormat + WAV_BITS_OFFSET);
                pInfo->channels = ReadLittleEndian16(pFormat + WAV_CHANNELS_OFFSET);
                pInfo->blockSize = ReadLittleEndian16(pFormat + WAV_BLOCK_ALIGN_OFFSET);

                const unsigned char* pDataChunk = pFirst;
                while (AIL_strnicmp(reinterpret_cast<const char*>(pDataChunk), WAV_DATA_CHUNK, WAV_SIGNATURE_LENGTH)
                       != 0)
                {
                    pDataChunk = NextWavChunk(pDataChunk);
                    if (pDataChunk >= pEnd)
                    {
                        strcpy(AIL_error, WAV_NOT_WAV_ERROR);
                        return 0;
                    }
                }

                pInfo->data = pDataChunk + WAV_CHUNK_HEADER_SIZE;
                unsigned int uLength = ReadLittleEndian32(pDataChunk + WAV_CHUNK_SIZE_OFFSET);
                pInfo->dataLength = uLength;

                if (static_cast<unsigned int>(pInfo->format) == WAV_FORMAT_EXTENSIBLE)
                {
                    if (ReadLittleEndian16(pFormat + WAV_EXTENSION_SIZE_OFFSET) < WAV_EXTENSION_SIZE_MINIMUM)
                    {
                        strcpy(AIL_error, WAV_NOT_EXTENSIBLE_ERROR);
                        return 0;
                    }

                    for (int iByte = 0; iByte < WAV_SUBTYPE_SIZE; ++iByte)
                    {
                        if (pFormat[WAV_SUBTYPE_OFFSET + iByte] != WAV_SUBTYPE_PCM[iByte])
                        {
                            strcpy(AIL_error, WAV_NOT_PCM_SUBTYPE_ERROR);
                            return 0;
                        }
                    }

                    if (pInfo->blockSize != static_cast<unsigned int>(pInfo->channels) * 2)
                    {
                        strcpy(AIL_error, WAV_MULTICHANNEL_ERROR);
                        return 0;
                    }

                    pInfo->format = WAV_FORMAT_PCM;
                    pInfo->channelMask = ReadLittleEndian32(pFormat + WAV_CHANNEL_MASK_OFFSET);
                    unsigned int uSamples = 0;
                    if (pInfo->bits != 0)
                    {
                        uSamples = (pInfo->dataLength * WAV_BITS_PER_BYTE) / pInfo->bits;
                    }

                    pInfo->initial = pInfo->data;
                    pInfo->samples = uSamples;
                    return 1;
                }

                unsigned int uBits = pInfo->bits;
                if (static_cast<unsigned int>(pInfo->format) == WAV_FORMAT_IMA_ADPCM && uBits == WAV_IMA_ADPCM_BITS)
                {
                    const unsigned char* pFact = pFirst;
                    while (AIL_strnicmp(reinterpret_cast<const char*>(pFact), WAV_FACT_CHUNK, WAV_SIGNATURE_LENGTH)
                           != 0)
                    {
                        pFact = NextWavChunk(pFact);
                        if (pFact >= pEnd)
                        {
                            unsigned int uAlign = pInfo->blockSize;
                            unsigned int uHeader = WAV_IMA_ADPCM_HEADER_BASE << (pInfo->channels / 2);
                            pInfo->initial = pInfo->data;
                            pInfo->samples = (((pInfo->dataLength + uAlign) - 1) / uAlign)
                                             * ((((uAlign - uHeader) * WAV_BITS_PER_BYTE) / uHeader) + 1);
                            return 1;
                        }
                    }

                    pInfo->initial = pInfo->data;
                    pInfo->samples = ReadLittleEndian32(pFact + WAV_FACT_SAMPLES_OFFSET);
                    return 1;
                }

                unsigned int uSamples = 0;
                if (uBits != 0)
                {
                    uSamples = (uLength * WAV_BITS_PER_BYTE) / uBits;
                }

                pInfo->samples = uSamples;
                pInfo->initial = pInfo->data;
                return 1;
            }
        }
    }

    strcpy(AIL_error, WAV_NOT_WAV_ERROR);
    return 0;
}

extern "C" int AIL_process_WAV_image(_AILSOUNDINFO* pInfo, _SAMPLE* pSample)
{
    unsigned int uFormat = 0;
    if (pInfo->channels <= 2 && pInfo->channelMask == ALL_BITS_SET)
    {
        if (pInfo->channels == 2)
        {
            uFormat = SAMPLE_FORMAT_STEREO;
        }
    }
    else
    {
        uFormat = (pInfo->channels << SAMPLE_FORMAT_CHANNEL_SHIFT) | SAMPLE_FORMAT_MULTICHANNEL;
    }

    if (pInfo->bits == WAV_IMA_ADPCM_BITS)
    {
        uFormat = uFormat | SAMPLE_FORMAT_ADPCM;
    }

    if (pInfo->bits == WAV_IMA_ADPCM_BITS || pInfo->bits == SAMPLE_WIDTH_16)
    {
        uFormat = uFormat | SAMPLE_FORMAT_16_BITS;
    }

    if (SS_set_sample_type(pSample, uFormat, pInfo->channelMask) == 0)
    {
        return 0;
    }

    AIL_set_sample_playback_rate(pSample, pInfo->rate);
    if ((uFormat & SAMPLE_FORMAT_ADPCM) != 0)
    {
        AIL_set_sample_adpcm_block_size(pSample, pInfo->blockSize);
    }

    AIL_set_sample_address(pSample, pInfo->data, pInfo->dataLength);
    return 1;
}

void SS_start_playback(_SAMPLE* pSample)
{
    SS_start_DIG_driver_playback(pSample);
}

extern "C" float SS_default_falloff_function_callback(_SAMPLE* pSample, float fDistance, float fRolloffFactor,
                                                      float fMinimumDistance, float fMaximumDistance)
{
    if (pSample != NULL && pSample->presetValue != ZERO)
    {
        fRolloffFactor = pSample->presetValue;
    }

    return fMinimumDistance / (fRolloffFactor * (fDistance - fMinimumDistance) + fMinimumDistance);
}

extern "C" void AIL_request_EOB_ASI_reset(_SAMPLE* pSample, int iBuffer, int iPosition)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iBuffer == SAMPLE_BUFFER_CURRENT)
    {
        for (int iIndex = 0; iIndex < pSample->bufferCount; ++iIndex)
        {
            pSample->buffers[iIndex].reset = 0;
            pSample->buffers[iIndex].resetPosition = 0;
        }

        return;
    }

    pSample->buffers[iBuffer].reset = 1;
    pSample->buffers[iBuffer].resetPosition = iPosition;
}

extern "C" _SAMPLE* AIL_API_bus_sample_handle(_DIG_DRIVER* pDriver, int iBus)
{
    return pDriver->busSamples[iBus];
}

extern "C" int AIL_allocate_reverb_buffers(_DIG_DRIVER* pDriver, int iBus)
{
    float fRate = static_cast<float>(pDriver->outputRate);
    if ((pDriver->flags & DRIVER_FLAG_NO_REVERB) != 0)
    {
        return 1;
    }

    unsigned int uChunk = pDriver->outputs[0].bytes >> (pDriver->outputs[0].chans - 1);
    DriverBus* pBus = &pDriver->buses[iBus];
    pBus->chunkBytes = uChunk;

    int iFirstLine = static_cast<int>(fRate * REVERB_LINE_SECONDS_B) * 4;
    int iSecondLine = static_cast<int>(fRate * REVERB_LINE_SECONDS_A) * 4;
    int iThirdLine = static_cast<int>(fRate * REVERB_LINE_SECONDS_C) * 4;
    int iFourthLine = static_cast<int>(fRate * REVERB_LINE_SECONDS_D) * 4;
    int iFifthLine = static_cast<int>(fRate * REVERB_LINE_SECONDS_E) * 4;
    int iSixthLine = static_cast<int>(fRate * REVERB_LINE_SECONDS_F) * 4;
    int iLength = static_cast<int>(
                      (static_cast<float>(AIL_preference[PREFERENCE_REVERB_MILLISECONDS]) / MILLISECONDS_PER_SECOND)
                      * fRate * REVERB_BYTES_PER_SAMPLE)
                  + uChunk - 1;
    iLength = (iLength / uChunk) * uChunk;
    pBus->length = iLength;

    const unsigned int uMask = ~(REVERB_ALIGNMENT - 1);
    unsigned int uFirstSize = (iFirstLine + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uSecondSize = (iSecondLine + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uThirdSize = (iThirdLine + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uFourthSize = (iFourthLine + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uFifthSize = (iFifthLine + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uSixthSize = (iSixthLine + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uDelaySize = (iLength + REVERB_ALIGNMENT - 1) & uMask;
    unsigned int uTotal = uDelaySize + uSecondSize + uFirstSize + uThirdSize + uFourthSize + uFifthSize + uSixthSize;
    pBus->bufferBytes = uTotal;
    pBus->buffer = static_cast<unsigned char*>(
        AIL_mem_alloc_lock_info(uTotal, MILES_WAVEFILE_FILE, REVERB_ALLOCATION_LINE));
    if (pBus->buffer == NULL)
    {
        return 0;
    }

    memset(pBus->buffer, 0, uTotal);
    pBus->writeOffset = 0;
    pBus->readOffset = 0;

    unsigned char* pLines = pBus->buffer + uDelaySize;
    ReverbState& reverb = pBus->reverb;
    reverb.starts[0] = reinterpret_cast<float*>(pLines);
    reverb.starts[1] = reinterpret_cast<float*>(pLines + uSecondSize);
    reverb.starts[2] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize);
    reverb.starts[3] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + uThirdSize);
    reverb.starts[4] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + uThirdSize + uFourthSize);
    reverb.starts[5] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + uThirdSize + uFourthSize
                                                + uFifthSize);
    reverb.ends[0] = reinterpret_cast<float*>(pLines + iSecondLine);
    reverb.ends[1] = reinterpret_cast<float*>(pLines + uSecondSize + iFirstLine);
    reverb.ends[2] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + iThirdLine);
    reverb.ends[3] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + uThirdSize + iFourthLine);
    reverb.ends[4] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + uThirdSize + uFourthSize
                                              + iFifthLine);
    reverb.ends[5] = reinterpret_cast<float*>(pLines + uSecondSize + uFirstSize + uThirdSize + uFourthSize + uFifthSize
                                              + iSixthLine);
    for (int iLine = 0; iLine < DRIVER_REVERB_POINTER_COUNT; ++iLine)
    {
        reverb.lines[iLine] = reverb.starts[iLine];
    }

    for (int iState = 0; iState < 4; ++iState)
    {
        reverb.state[iState] = ZERO;
    }

    return 1;
}

extern "C" int AIL_API_set_sample_file(_SAMPLE* pSample, const void* pFile, int iBlock)
{
    if (pSample == NULL)
    {
        return 0;
    }

    if (pFile == NULL)
    {
        return 0;
    }

    if (AIL_init_sample(pSample, SAMPLE_FORMAT_MONO_16) == 0)
    {
        return 0;
    }

    const char* pText = static_cast<const char*>(pFile);
    const unsigned char* pBytes = static_cast<const unsigned char*>(pFile);
    if (AIL_strnicmp(pText, CREATIVE_VOC_SIGNATURE, CREATIVE_SIGNATURE_LENGTH) == 0)
    {
        VocBlocks* pVoc = VocState(pSample);
        unsigned short usHeaderSize = *reinterpret_cast<const unsigned short*>(pBytes + VOC_HEADER_SIZE_OFFSET);
        pSample->vocMarker = iBlock;
        unsigned char* pBlock = static_cast<unsigned char*>(const_cast<void*>(pFile)) + usHeaderSize;
        pVoc->current = pBlock;
        pSample->vocMarkerFound = (iBlock == -1);
        for (;;)
        {
            int iDone = 0;
            switch (*pBlock)
            {
            case VOC_BLOCK_TERMINATOR:
                return 1;
            case VOC_BLOCK_SOUND_DATA:
                if (pSample->vocMarkerFound != 0)
                {
                    int iLength = AIL_VOC_block_len(pBlock);
                    AIL_set_sample_address(pSample, pBlock + VOC_SOUND_DATA_HEADER, iLength - 2);
                    AIL_set_sample_playback_rate(pSample, VOC_RATE_NUMERATOR / (VOC_RATE_BASE - pBlock[4]));
                    if (SS_set_sample_type(pSample, SAMPLE_FORMAT_MONO_8, SAMPLE_CHANNEL_MASK_ALL) == 0)
                    {
                        return 0;
                    }

                    iDone = 1;
                }

                break;
            case VOC_BLOCK_CONTINUATION:
            case VOC_BLOCK_SILENCE:
            case VOC_BLOCK_TEXT:
                break;
            case VOC_BLOCK_MARKER:
                if (pSample->vocMarker == -1)
                {
                    break;
                }

                pSample->vocMarkerFound = (pSample->vocMarker == *reinterpret_cast<const short*>(pBlock + 4));
                pBlock = pBlock + AIL_VOC_block_len(pBlock) + VOC_BLOCK_HEADER_SIZE;
                pVoc->current = pBlock;
                continue;
            case VOC_BLOCK_REPEAT:
                pVoc->repeat = pBlock;
                pSample->vocRepeatCount = *reinterpret_cast<const unsigned short*>(pBlock + 4);
                pBlock = pBlock + AIL_VOC_block_len(pBlock) + VOC_BLOCK_HEADER_SIZE;
                pVoc->current = pBlock;
                continue;
            case VOC_BLOCK_END_REPEAT:
            {
                int iRemaining = pSample->vocRepeatCount;
                if (iRemaining != VOC_REPEAT_FOREVER)
                {
                    pSample->vocRepeatCount = iRemaining - 1;
                    if (iRemaining == 0)
                    {
                        break;
                    }
                }

                unsigned char* pRepeat = pVoc->repeat;
                pBlock = pRepeat + AIL_VOC_block_len(pRepeat) + VOC_BLOCK_HEADER_SIZE;
                pVoc->current = pBlock;
                continue;
            }
            case VOC_BLOCK_EXTENDED:
                if (pSample->vocMarkerFound == 0)
                {
                    break;
                }

                if (pBlock[7] == 0)
                {
                    if (SS_set_sample_type(pSample, SAMPLE_FORMAT_MONO_8, SAMPLE_CHANNEL_MASK_ALL) == 0)
                    {
                        return 0;
                    }

                    AIL_set_sample_playback_rate(
                        pSample, VOC_EXTENDED_MONO_NUMERATOR
                                     / (VOC_EXTENDED_RATE_BASE - *reinterpret_cast<const unsigned short*>(pBlock + 4)));
                }
                else
                {
                    if (SS_set_sample_type(pSample, SAMPLE_FORMAT_STEREO_8, SAMPLE_CHANNEL_MASK_ALL) == 0)
                    {
                        return 0;
                    }

                    AIL_set_sample_playback_rate(
                        pSample, VOC_EXTENDED_STEREO_NUMERATOR
                                     / (VOC_EXTENDED_RATE_BASE - *reinterpret_cast<const unsigned short*>(pBlock + 4)));
                }

                pBlock = pBlock + AIL_VOC_block_len(pBlock) + VOC_BLOCK_HEADER_SIZE;
                AIL_set_sample_address(pSample, pBlock + VOC_SOUND_DATA_HEADER, AIL_VOC_block_len(pBlock) - 2);
                iDone = 1;
                break;
            case VOC_BLOCK_NEW_DATA:
                if (pSample->vocMarkerFound != 0)
                {
                    AIL_set_sample_address(pSample, pBlock + VOC_NEW_DATA_HEADER,
                                           AIL_VOC_block_len(pBlock) - VOC_NEW_DATA_TRAILER);
                    AIL_set_sample_playback_rate(pSample, *reinterpret_cast<const unsigned int*>(pBlock + 4));
                    if (pBlock[9] == 1)
                    {
                        int iCodec = *reinterpret_cast<const short*>(pBlock + 10);
                        if (iCodec == VOC_CODEC_PCM)
                        {
                            if (SS_set_sample_type(pSample, SAMPLE_FORMAT_MONO_8, SAMPLE_CHANNEL_MASK_ALL) == 0)
                            {
                                return 0;
                            }
                        }
                        else if (iCodec == VOC_CODEC_ADPCM)
                        {
                            if (SS_set_sample_type(pSample, SAMPLE_FORMAT_MONO_16, SAMPLE_CHANNEL_MASK_ALL) == 0)
                            {
                                return 0;
                            }
                        }
                    }
                    else if (pBlock[9] == 2)
                    {
                        int iCodec = *reinterpret_cast<const short*>(pBlock + 10);
                        if (iCodec == VOC_CODEC_PCM)
                        {
                            if (SS_set_sample_type(pSample, SAMPLE_FORMAT_STEREO_8, SAMPLE_CHANNEL_MASK_ALL) == 0)
                            {
                                return 0;
                            }
                        }
                        else if (iCodec == VOC_CODEC_ADPCM)
                        {
                            if (SS_set_sample_type(pSample, SAMPLE_FORMAT_STEREO_16, SAMPLE_CHANNEL_MASK_ALL) == 0)
                            {
                                return 0;
                            }
                        }
                    }

                    iDone = 1;
                }

                break;
            }

            pBlock = pBlock + AIL_VOC_block_len(pBlock) + VOC_BLOCK_HEADER_SIZE;
            pVoc->current = pBlock;
            if (iDone != 0)
            {
                return 1;
            }
        }
    }

    if (ReadLittleEndian32(pBytes) == BINK_AUDIO_TAG)
    {
        unsigned int uProvider = find_ASI_provider(INPUT_FILE_TYPES, BINK_AUDIO_EXTENSION);
        if (uProvider == 0)
        {
            AIL_set_error(BINK_PROVIDER_MISSING_ERROR);
            return 0;
        }

        clear_last_sample(pSample);
        AIL_set_sample_address(pSample, pFile, ReadLittleEndian32(pBytes + BINK_AUDIO_SIZE_OFFSET));
        if (AIL_set_sample_processor(pSample, SAMPLE_STAGE_DECODER, uProvider) == -1)
        {
            return 0;
        }

        if (AIL_sample_processor(pSample, SAMPLE_STAGE_DECODER) == 0)
        {
            AIL_set_error(BINK_PROVIDER_FAILED_ERROR);
            return 0;
        }

        return 1;
    }

    unsigned int uProvider;
    if (AIL_strnicmp(pText, OGG_SIGNATURE, OGG_SIGNATURE_LENGTH) == 0)
    {
        const char* pExtension = OGG_EXTENSION;
        for (int iOffset = 0; iOffset < OGG_SPEEX_SCAN_LENGTH; ++iOffset)
        {
            if (AIL_strnicmp(pText + iOffset, SPEEX_SIGNATURE, SPEEX_SIGNATURE_LENGTH) == 0)
            {
                pExtension = SPEEX_EXTENSION;
                break;
            }
        }

        uProvider = find_ASI_provider(INPUT_FILE_TYPES, pExtension);
        if (uProvider == 0)
        {
            AIL_set_error(UNSUPPORTED_WAVE_ERROR);
            return 0;
        }

        if (AIL_set_sample_processor(pSample, SAMPLE_STAGE_DECODER, uProvider) == -1)
        {
            return 0;
        }

        return 1;
    }

    _AILSOUNDINFO info;
    if (AIL_WAV_info(pFile, &info) == 0)
    {
        AIL_set_error(UNKNOWN_FILE_TYPE_ERROR);
        return 0;
    }

    if (info.format == WAV_FORMAT_PCM || (info.format == WAV_FORMAT_IMA_ADPCM && info.bits == WAV_IMA_ADPCM_BITS))
    {
        if (AIL_process_WAV_image(&info, pSample) != 0)
        {
            return 1;
        }

        return 0;
    }

    uProvider = RIB_find_file_dec_provider(ASI_CODEC_INTERFACE_NAME, INPUT_WAVE_TAG, info.format, OUTPUT_FILE_TYPES,
                                           RAW_EXTENSION);
    if (uProvider == 0)
    {
        AIL_set_error(UNSUPPORTED_WAVE_ERROR);
        return 0;
    }

    AIL_set_sample_address(pSample, info.data, info.dataLength);
    if (AIL_set_sample_processor(pSample, SAMPLE_STAGE_DECODER, uProvider) == -1)
    {
        return 0;
    }

    return 1;
}

extern "C" int AIL_API_set_named_sample_file(_SAMPLE* pSample, const char* pFileName, const void* pFile, int iSize,
                                             int iBlock)
{
    if (pSample == NULL || pFile == NULL)
    {
        return 0;
    }

    const unsigned char* pBytes = static_cast<const unsigned char*>(pFile);
    size_t uNameLength;
    if (pBytes[0] == 'I' && pBytes[1] == 'D' && pBytes[2] == '3' && pBytes[3] < ID3_VERSION_LIMIT
        && pBytes[4] < ID3_VERSION_LIMIT && pBytes[6] < ID3_SYNCSAFE_LIMIT && pBytes[7] < ID3_SYNCSAFE_LIMIT
        && pBytes[8] < ID3_SYNCSAFE_LIMIT && pBytes[9] < ID3_SYNCSAFE_LIMIT)
    {
        pFileName = MP3_EXTENSION;
        int iTagSize = ((pBytes[9] | (pBytes[8] << 7)) | (pBytes[7] << 14) | (pBytes[6] << 21)) + ID3_HEADER_SIZE;
        pBytes = pBytes + iTagSize;
        iSize = iSize - iTagSize;
        uNameLength = strlen(pFileName);
    }
    else
    {
        if (pFileName == NULL)
        {
            return AIL_set_sample_file(pSample, pBytes, iBlock);
        }

        uNameLength = strlen(pFileName);
    }

    if (uNameLength == 0)
    {
        return AIL_set_sample_file(pSample, pBytes, iBlock);
    }

    if (AIL_init_sample(pSample, SAMPLE_FORMAT_MONO_16) == 0)
    {
        return 0;
    }

    unsigned int uProvider = find_ASI_provider(INPUT_FILE_TYPES, pFileName);
    if (uProvider == 0)
    {
        size_t uLength = strlen(pFileName);
        if (uLength > MP3_EXTENSION_LENGTH - 1)
        {
            if (AIL_stricmp(pFileName + (uLength - MP3_EXTENSION_LENGTH), WAV_EXTENSION) == 0
                || AIL_stricmp(pFileName + (strlen(pFileName) - MP3_EXTENSION_LENGTH), VOC_EXTENSION) == 0)
            {
                return AIL_set_sample_file(pSample, pBytes, iBlock);
            }
        }

        AIL_set_error(PROVIDER_MISSING_ERROR);
        return 0;
    }

    clear_last_sample(pSample);
    AIL_set_sample_address(pSample, pBytes, iSize);
    if (AIL_set_sample_processor(pSample, SAMPLE_STAGE_DECODER, uProvider) == -1)
    {
        return 0;
    }

    if (AIL_sample_processor(pSample, SAMPLE_STAGE_DECODER) == 0)
    {
        AIL_set_error(PROVIDER_FAILED_ERROR);
        return 0;
    }

    return 1;
}

extern "C" int AIL_API_set_digital_driver_processor(_DIG_DRIVER* pDriver, int iStage, int iProvider)
{
    if (pDriver == NULL)
    {
        return 0;
    }

    int iIndex = 0;
    if (iStage != SAMPLE_PROCESSOR_ALL)
    {
        if (iStage == DRIVER_PROCESSOR_PROVIDER_FIRST || iStage == DRIVER_PROCESSOR_PROVIDER_SECOND)
        {
            DriverProcessor* pSelected = &pDriver->processors[iStage];
            int iPrevious = pSelected->provider;
            pSelected->provider = iProvider;
            return iPrevious;
        }

        iIndex = iStage;
    }

    DriverProcessor* pProcessor;
    do
    {
        pProcessor = &pDriver->processors[iIndex];
        if (pProcessor->active != 0)
        {
            pProcessor->active = 0;
        }

        if (iStage != SAMPLE_PROCESSOR_ALL)
        {
            break;
        }

        ++iIndex;
    } while (iIndex < DRIVER_PROCESSOR_COUNT);

    if (iProvider == 0)
    {
        return 0;
    }

    int iPrevious = pProcessor->provider;
    pProcessor->provider = iProvider;

    const char* pEntryName = NULL;
    if (iIndex == DRIVER_PROCESSOR_FLUSH)
    {
        pEntryName = MIXER_FLUSH_ENTRY;
    }
    else if (iIndex == DRIVER_PROCESSOR_COPY)
    {
        pEntryName = MIXER_COPY_ENTRY;
    }
    else if (iIndex == DRIVER_PROCESSOR_MULTICHANNEL_COPY)
    {
        pEntryName = MIXER_MULTICHANNEL_COPY_ENTRY;
    }
    else if (iIndex == DRIVER_PROCESSOR_ADPCM_DECODE)
    {
        pEntryName = MIXER_ADPCM_DECODE_ENTRY;
    }

    if (pEntryName != NULL)
    {
        RIB_INTERFACE_ENTRY entry;
        entry.uType = RIB_FUNCTION;
        entry.pEntryName = pEntryName;
        entry.pToken = &pProcessor->function;
        entry.uSubType = 0;
        RIB_request_interface(pProcessor->provider, MIXER_SERVICES_INTERFACE, DRIVER_PROCESSOR_ENTRY_COUNT, &entry);
        pProcessor->active = 1;
    }

    return iPrevious;
}

extern "C" _SAMPLE* AIL_API_allocate_bus(_DIG_DRIVER* pDriver)
{
    int iBus = 0;
    while (iBus < DRIVER_BUS_COUNT && pDriver->busSamples[iBus] != NULL)
    {
        ++iBus;
    }

    if (iBus == DRIVER_BUS_COUNT)
    {
        AIL_set_error(OUT_OF_BUSSES_ERROR);
        return NULL;
    }

    _SAMPLE* pSample = AIL_allocate_sample_handle(pDriver);
    if (pSample == NULL)
    {
        return NULL;
    }

    int iInitialised;
    int iSpeakers = pDriver->speakerConfig;
    if (iSpeakers > 2)
    {
        iInitialised = AIL_init_sample(pSample,
                                       (iSpeakers << SAMPLE_FORMAT_CHANNEL_SHIFT) | SAMPLE_FORMAT_MULTICHANNEL_16);
    }
    else if (iSpeakers > 1)
    {
        iInitialised = AIL_init_sample(pSample, SAMPLE_FORMAT_STEREO_16);
    }
    else
    {
        iInitialised = AIL_init_sample(pSample, SAMPLE_FORMAT_MONO_16);
    }

    if (iInitialised == 0)
    {
        AIL_release_sample_handle(pSample);
        return 0;
    }

    AIL_set_sample_playback_rate(pSample, pDriver->outputRate);
    pDriver->busSamples[iBus] = pSample;
    pSample->reverbBus = SAMPLE_NO_REVERB_BUS;
    unsigned char* pBuffer = static_cast<unsigned char*>(AIL_mem_alloc_lock_info(
        pDriver->framesPerBuffer * pSample->channelCount * 2, MILES_WAVEFILE_FILE, BUS_BUFFER_LINE));
    if (pBuffer == NULL)
    {
        AIL_set_error(BUS_BUFFER_ERROR);
        AIL_release_sample_handle(pSample);
        pDriver->busSamples[iBus] = NULL;
        return 0;
    }

    pDriver->busBuffers[iBus] = pBuffer;
    _SAMPLE* pResult = pSample;
    if (iBus == BUS_FIRST)
    {
        pResult = AIL_API_allocate_bus(pDriver);
        if (pResult == NULL)
        {
            AIL_release_sample_handle(pDriver->busSamples[0]);
            AIL_mem_free_lock(pDriver->busBuffers[0]);
            pDriver->busSamples[0] = NULL;
            pDriver->busBuffers[0] = NULL;
            return 0;
        }
    }
    else
    {
        DriverBus* pBus = &pDriver->buses[iBus];
        pBus->returnGain = ONE;
        pBus->sendLevel = ONE;
        pBus->roomType = 0;
        if (AIL_allocate_reverb_buffers(pDriver, iBus) == 0)
        {
            AIL_mem_free_lock(pBuffer);
            AIL_release_sample_handle(pSample);
            pDriver->busBuffers[iBus] = NULL;
            pDriver->busSamples[iBus] = NULL;
            return 0;
        }
    }

    return pResult;
}

extern "C" _SAMPLE* AIL_API_allocate_sample_handle(_DIG_DRIVER* pDriver)
{
    if (pDriver == NULL)
    {
        return NULL;
    }

    InMilesMutex();
    int iIndex = 0;
    while (iIndex < pDriver->sampleCount && pDriver->sampleStatus[iIndex] != SAMPLE_STATUS_FREE)
    {
        ++iIndex;
    }

    if (iIndex == pDriver->sampleCount)
    {
        AIL_set_error(OUT_OF_HANDLES_ERROR);
        OutMilesMutex();
        return NULL;
    }

    _SAMPLE* pSample = &pDriver->samples[iIndex];
    if (AIL_init_sample(pSample, SAMPLE_FORMAT_MONO_16) == 0)
    {
        return NULL;
    }

    OutMilesMutex();
    return pSample;
}

extern "C" void AIL_API_release_sample_handle(_SAMPLE* pSample)
{
    if (pSample != NULL)
    {
        pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_FREE;
        AIL_set_sample_processor(pSample, SAMPLE_STAGE_ALL, 0);
        free_channel_buffers(pSample);
    }
}

extern "C" void AIL_API_set_sample_volume_levels(_SAMPLE* pSample, float fLeft, float fRight)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->panLevelB = fRight;
    pSample->panLevelA = fLeft;

    float fVolume;
    if (fLeft <= LEVEL_EPSILON)
    {
        if (fRight > LEVEL_EPSILON)
        {
            pSample->pan = ONE;
            pSample->volumeCurve = fRight;
            pSample->mixLevel1 = pSample->panLevelB;
            pSample->mixLevel2 = fRight;
            pSample->mixLevel3 = fRight;
            pSample->mixInput0 = HALF;
            pSample->mixLevel0 = pSample->panLevelA;
            return;
        }

        fVolume = ZERO;
        pSample->pan = HALF;
    }
    else
    {
        float fRatio = static_cast<float>(pow(static_cast<float>(fRight / fLeft), PAN_RATIO_EXPONENT));
        float fPan = fRatio / (fRatio + ONE);
        pSample->pan = fPan;
        if (fPan >= LEVEL_EPSILON)
        {
            float fAdjusted = fRight * static_cast<float>(pow(fPan, PAN_VOLUME_EXPONENT));
            pSample->mixInput0 = HALF;
            pSample->mixLevel0 = pSample->panLevelA;
            pSample->mixLevel3 = fAdjusted;
            pSample->mixLevel2 = fAdjusted;
            pSample->volumeCurve = fAdjusted;
            pSample->mixLevel1 = pSample->panLevelB;
            return;
        }

        fVolume = fLeft;
    }

    pSample->mixLevel2 = fVolume;
    pSample->mixLevel1 = pSample->panLevelB;
    pSample->volumeCurve = fVolume;
    pSample->mixLevel0 = pSample->panLevelA;
    pSample->mixLevel3 = fVolume;
    pSample->mixInput0 = HALF;
}

extern "C" int AIL_API_init_sample(_SAMPLE* pSample, unsigned int uFormat)
{
    if (pSample == NULL)
    {
        return 0;
    }

    pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_FREE;
    AIL_set_sample_processor(pSample, SAMPLE_STAGE_ALL, 0);

    pSample->coneEnabled = 0;
    pSample->falloffLowPass = NO_FALLOFF_RESULT;
    pSample->maximumDistance = DEFAULT_MAXIMUM_DISTANCE;
    pSample->z = ZERO;
    pSample->distanceVolume = ONE;
    pSample->upZ = ZERO;
    pSample->dopplerScale = ONE;
    pSample->x = ZERO;
    pSample->coneOuterVolume = ONE;
    pSample->velocityY = ZERO;
    pSample->minimumDistance = ONE;
    pSample->upY = ONE;
    pSample->forwardX = ONE;
    pSample->falloffExclusion = ZERO;
    pSample->distancesChanged = 1;
    pSample->coneOuter = DEFAULT_CONE_ANGLE;
    pSample->falloffFunction = NULL;
    pSample->forwardZ = ZERO;
    pSample->velocityZ = ZERO;
    pSample->upX = ZERO;
    pSample->falloffUser = pSample;
    pSample->automaticWetAttenuation = 1;
    pSample->falloffVolume = NO_FALLOFF_RESULT;
    pSample->is3D = 0;
    pSample->dopplerActive = 0;
    pSample->forwardY = ZERO;
    pSample->y = ZERO;
    pSample->velocityX = ZERO;
    pSample->coneInner = DEFAULT_CONE_ANGLE;
    memset(pSample->pathPoints, 0, sizeof(pSample->pathPoints));
    pSample->pathPointCount = 0;
    memset(pSample->falloffPoints[0], 0, sizeof(pSample->falloffPoints[0]));
    memset(pSample->falloffPoints[1], 0, sizeof(pSample->falloffPoints[1]));
    memset(pSample->falloffPoints[2], 0, sizeof(pSample->falloffPoints[2]));
    pSample->falloffPointCounts[0] = 0;
    pSample->falloffPointCounts[1] = 0;
    pSample->falloffPointCounts[2] = 0;
    AIL_set_sample_buffer_count(pSample, SAMPLE_DEFAULT_BUFFER_COUNT);

    pSample->fadeSamples = 0;
    pSample->playbackRateFactor = ONE;
    pSample->fadeEnds = 0;
    pSample->loopStartRequest = 0;
    pSample->playbackRate = SAMPLE_DEFAULT_PLAYBACK_RATE;
    pSample->channelCount = 0;
    pSample->loopEnd = SAMPLE_LOOP_TO_END;
    pSample->loopCount = SAMPLE_LOOP_FOREVER;
    pSample->loopStart = 0;
    pSample->fadePosition = 0;
    pSample->loopEndRequest = SAMPLE_LOOP_TO_END;
    pSample->loopState = SAMPLE_LOOP_FOREVER;

    if (SS_set_sample_type(pSample, uFormat, SAMPLE_CHANNEL_MASK_ALL) != 0)
    {
        clear_last_sample(pSample);
        pSample->loopMode = LOOP_MODE_ASI;
        AIL_API_set_sample_volume_levels(pSample, ONE, ONE);
        pSample->resetA = 0;
        pSample->lowPassActive = 0;
        pSample->startCounter = 0;
        pSample->rate = DEFAULT_LOW_PASS_CUT_OFF;
        pSample->occlusion = ZERO;
        pSample->sampleEndCounter = 0;
        pSample->endOfBuffer = NULL;
        pSample->limiterEnabled = 0;
        pSample->reverbBus = 0;
        pSample->levelB = ONE;
        pSample->levelA = ONE;
        pSample->resetB = 0;
        pSample->busPeak = 0;
        pSample->reverbFirst = ONE;
        pSample->endCounter = 0;
        pSample->playedHigh = 0;
        pSample->limiterLevel = 0;
        pSample->exclusion = ZERO;
        pSample->compressorSources = 0;
        pSample->obstruction = ZERO;
        pSample->startOfBuffer = NULL;
        pSample->levelD = ONE;
        pSample->compressorLink = 0;
        pSample->outputToDRC = false;
        pSample->playedLow = 0;
        pSample->reverbSecond = ZERO;
        pSample->peak = 0;
        pSample->adpcmBlockSize = SAMPLE_DEFAULT_ADPCM_BLOCK_SIZE;
        pSample->endOfSample = NULL;
        pSample->levelC = ONE;
        pSample->reverbDirty = 0;
        pSample->rampLength = AIL_preference[PREFERENCE_RAMP_LENGTH];
        pSample->resampleTolerance = AIL_preference[PREFERENCE_RESAMPLING_TOLERANCE];
        pSample->resampleFilter = AIL_preference[PREFERENCE_RESAMPLE_FILTER];
        pSample->playbackDelay = 0;
        pSample->filterCount = 0;
        for (int iStage = 0; iStage < SAMPLE_STAGE_COUNT; ++iStage)
        {
            pSample->stages[iStage].filter.active = 0;
        }

        _DIG_DRIVER* pDriver = pSample->driver;
        if (AIL_set_sample_processor(pSample, SAMPLE_STAGE_MERGE,
                                     pDriver->processors[DRIVER_PROCESSOR_PROVIDER_SECOND].provider)
                != -1
            && AIL_set_sample_processor(pSample, SAMPLE_STAGE_FILTER_FIRST,
                                        pDriver->processors[DRIVER_PROCESSOR_PROVIDER_FIRST].provider)
                   != -1)
        {
            pSample->userLevels = 0;
            pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_DONE;
            return 1;
        }
    }

    return 0;
}

extern "C" int AIL_API_set_sample_is_3D(_SAMPLE* pSample, int iEnabled)
{
    if (pSample == NULL)
    {
        return 0;
    }

    int iPrevious = pSample->is3D;
    pSample->is3D = iEnabled;
    return iPrevious;
}

extern "C" int AIL_API_sample_status(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return 0;
    }

    return pSample->driver->sampleStatus[pSample->index];
}

extern "C" void AIL_API_set_sample_adpcm_block_size(_SAMPLE* pSample, unsigned int uBlockSize)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->adpcmBlockSize = uBlockSize;
}

extern "C" void AIL_API_set_sample_address(_SAMPLE* pSample, const void* pStart, unsigned int uLength)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->buffers[0].data = static_cast<const unsigned char*>(pStart);
    pSample->buffers[0].size = uLength;
    for (int iIndex = 1; iIndex < pSample->bufferCount; ++iIndex)
    {
        pSample->buffers[iIndex].data = NULL;
        pSample->buffers[iIndex].size = 0;
    }

    pSample->loopMode = LOOP_MODE_ASI;
}

extern "C" float AIL_API_sample_playback_rate_factor(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return ZERO;
    }

    return pSample->playbackRateFactor;
}

extern "C" void AIL_API_set_sample_playback_rate_factor(_SAMPLE* pSample, float fFactor)
{
    if (pSample == NULL)
    {
        return;
    }

    if (fFactor <= ZERO)
    {
        return;
    }

    if (fabs(static_cast<float>(fFactor - pSample->playbackRateFactor)) > RATE_FACTOR_EPSILON)
    {
        pSample->cacheState = -1;
    }

    pSample->playbackRateFactor = fFactor;
}

extern "C" void AIL_API_set_sample_playback_delay(_SAMPLE* pSample, int iDelay)
{
    if (pSample == NULL)
    {
        return;
    }

    if (iDelay < 1)
    {
        return;
    }

    pSample->playbackDelay = iDelay << SAMPLE_DELAY_SHIFT;
}

extern "C" void AIL_API_sample_volume_levels(_SAMPLE* pSample, float* pLeftLevel, float* pRightLevel)
{
    if (pSample == NULL)
    {
        return;
    }

    if (pLeftLevel != NULL)
    {
        *pLeftLevel = pSample->panLevelA;
    }

    if (pRightLevel != NULL)
    {
        *pRightLevel = pSample->panLevelB;
    }
}

extern "C" void AIL_API_set_speaker_reverb_levels(_DIG_DRIVER* pDriver, const float* pWetLevels,
                                                  const float* pDryLevels, const int* pSpeakers, int iCount)
{
    if (pDriver == NULL || pDriver->speakerConfig == 0)
    {
        return;
    }

    if (pWetLevels == NULL || pSpeakers == NULL)
    {
        for (int iSpeaker = 0; iSpeaker < pDriver->speakerConfig; ++iSpeaker)
        {
            pDriver->reverbSendA[iSpeaker] = ONE;
        }
    }

    if (pDryLevels == NULL || pSpeakers == NULL)
    {
        for (int iSpeaker = 0; iSpeaker < pDriver->speakerConfig; ++iSpeaker)
        {
            pDriver->reverbSendB[iSpeaker] = ONE;
        }

        if (pSpeakers == NULL)
        {
            return;
        }
    }

    int iConfig = pDriver->speakerConfig;
    if (iConfig < iCount)
    {
        iCount = iConfig;
    }

    for (int iIndex = 0; iIndex < iCount; ++iIndex)
    {
        int iChannel = SPEAKER_CHANNEL_MAP[iConfig][pSpeakers[iIndex]];
        if (iChannel != SPEAKER_UNMAPPED)
        {
            if (pWetLevels != NULL)
            {
                pDriver->reverbSendA[iChannel] = pWetLevels[iIndex];
            }

            if (pDryLevels != NULL)
            {
                pDriver->reverbSendB[iChannel] = pDryLevels[iIndex];
            }
        }
    }
}

extern "C" void AIL_API_set_speaker_configuration(_DIG_DRIVER* pDriver, const float* pSpeakerPositions, int iChannels,
                                                  float fAzimuth)
{
    if (pDriver == NULL)
    {
        return;
    }

    if (pDriver->speakerConfig == 0)
    {
        return;
    }

    pDriver->speakers.panExponent = fAzimuth;
    SS_set_speaker_configuration(&pDriver->speakers, pSpeakerPositions, iChannels, pDriver->speakerConfig);
}

extern "C" unsigned int SS_granularity(_SAMPLE* pSample)
{
    switch (pSample->channelFlags)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 2;
    case 3:
        return 4;
    case SAMPLE_FORMAT_MULTICHANNEL_16:
        return pSample->channelCount * 2;
    default:
        return 1;
    }
}

extern "C" int AIL_API_sample_granularity(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return 0;
    }

    if (pSample->stages[SAMPLE_STAGE_DECODER].decoder.active != 0)
    {
        return 1;
    }

    if ((pSample->channelFlags & SAMPLE_FORMAT_ADPCM) != 0)
    {
        return pSample->adpcmBlockSize;
    }

    return SS_granularity(pSample);
}

extern "C" unsigned int AIL_API_sample_position(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return 0;
    }

    return pSample->buffers[pSample->currentBuffer].position;
}

extern "C" void AIL_API_set_sample_position(_SAMPLE* pSample, unsigned int uPosition)
{
    if (pSample != NULL)
    {
        unsigned int uSize = pSample->buffers[pSample->currentBuffer].size;
        if (uPosition > uSize)
        {
            uPosition = uSize;
        }

        unsigned int uGranularity = AIL_sample_granularity(pSample);
        uPosition = ((uPosition + (uGranularity >> 1)) / uGranularity) * uGranularity;
        if ((pSample->channelFlags & SAMPLE_FORMAT_ADPCM) != 0)
        {
            pSample->resetB = 0;
            pSample->resetA = 0;
        }

        reset_asi(pSample, SAMPLE_BUFFER_CURRENT, uPosition);
        SampleBuffer* pBuffer = &pSample->buffers[pSample->currentBuffer];
        if (uPosition <= static_cast<unsigned int>(pBuffer->size))
        {
            pBuffer->position = uPosition;
        }
    }
}

extern "C" void AIL_API_set_sample_loop_count(_SAMPLE* pSample, int iLoopCount)
{
    if (pSample == NULL)
    {
        return;
    }

    pSample->loopState = iLoopCount;
    pSample->loopCount = iLoopCount;
}

int nibbles_per_sample(int iFormat)
{
    unsigned int uFormat = iFormat & NIBBLE_FORMAT_MASK;
    switch (uFormat)
    {
    case 0:
        return NIBBLES_MONO_8;
    case 1:
        return NIBBLES_MONO_16;
    case 2:
        return NIBBLES_STEREO_8;
    case NIBBLE_FORMAT_ADPCM_MONO:
        return NIBBLES_ADPCM_MONO;
    case NIBBLE_FORMAT_ADPCM_STEREO:
        return NIBBLES_ADPCM_STEREO;
    case NIBBLE_FORMAT_MULTICHANNEL:
        return (iFormat >> NIBBLE_CHANNEL_SHIFT) << NIBBLES_MULTICHANNEL_SHIFT;
    default:
        return NIBBLES_STEREO_16;
    }
}

extern "C" int AIL_set_sample_loop_samples(_SAMPLE* pSample, int iLoopStart, int iLoopEnd)
{
    if (iLoopStart < 0)
    {
        if (iLoopStart == SAMPLE_LOOP_CURRENT && iLoopEnd == SAMPLE_LOOP_CURRENT)
        {
            return 1;
        }
    }
    else if (iLoopEnd >= 0 && iLoopEnd <= iLoopStart)
    {
        AIL_set_error(INVALID_INTERVAL_ERROR);
        return 0;
    }

    if (pSample == NULL)
    {
        return 0;
    }

    DecoderStage& decoder = pSample->stages[SAMPLE_STAGE_DECODER].decoder;
    if (decoder.active != 0)
    {
        if (decoder.setLoopingSamples != RIB_ENTRY_NOT_FOUND)
        {
            int iRequest[2];
            int iResult[2];
            iRequest[0] = iLoopStart;
            iRequest[1] = iLoopEnd;
            if (decoder.property(decoder.stream, decoder.setLoopingSamples, 0, iRequest, iResult) != 0)
            {
                if (iResult[0] == SAMPLE_LOOP_CURRENT)
                {
                    iResult[0] = pSample->loopStartRequest;
                }

                if (iResult[1] == SAMPLE_LOOP_CURRENT)
                {
                    iResult[1] = pSample->loopEndRequest;
                }

                pSample->loopStartRequest = iResult[0];
                pSample->loopStart = iResult[0];
                pSample->loopEnd = iResult[1];
                pSample->loopEndRequest = iResult[1];
                return 1;
            }
        }

        AIL_set_error(LOOP_ACCURACY_ERROR);
        return 0;
    }

    int iNibbles = nibbles_per_sample(pSample->channelFlags);
    int iStart = (iLoopStart * iNibbles) / 2;
    int iEnd = (iLoopEnd * iNibbles) / 2;
    unsigned int uGranularity = AIL_API_sample_granularity(pSample);
    iStart = ((iStart + (uGranularity >> 1)) / uGranularity) * uGranularity;
    if (iLoopStart == SAMPLE_LOOP_CURRENT)
    {
        iStart = SAMPLE_LOOP_CURRENT;
    }

    AIL_set_sample_loop_block(pSample, iStart, ((iEnd + (uGranularity >> 1)) / uGranularity) * uGranularity);
    return 1;
}

extern "C" void AIL_API_set_sample_loop_block(_SAMPLE* pSample, int iLoopStart, int iLoopEnd)
{
    unsigned int uStart = iLoopStart;
    unsigned int uEnd = iLoopEnd;
    if (pSample == NULL)
    {
        return;
    }

    if (uStart == SAMPLE_LOOP_CURRENT_UNSIGNED && uEnd == SAMPLE_LOOP_CURRENT_UNSIGNED)
    {
        return;
    }

    DecoderStage& decoder = pSample->stages[SAMPLE_STAGE_DECODER].decoder;
    if (decoder.active != 0 && decoder.clearLoopMeta != RIB_ENTRY_NOT_FOUND)
    {
        unsigned int uFlags = (uStart != SAMPLE_LOOP_CURRENT_UNSIGNED);
        if (uEnd != SAMPLE_LOOP_CURRENT_UNSIGNED)
        {
            uFlags = uFlags | 2;
        }

        decoder.property(decoder.stream, decoder.clearLoopMeta, 0, &uFlags, 0);
    }

    if (uStart == SAMPLE_LOOP_CURRENT_UNSIGNED)
    {
        uStart = pSample->loopStartRequest;
    }

    if (uEnd == SAMPLE_LOOP_CURRENT_UNSIGNED)
    {
        uEnd = pSample->loopEndRequest;
    }

    unsigned int uLow = uStart;
    if (uEnd < uStart)
    {
        uLow = uEnd;
        uEnd = uStart;
    }

    pSample->loopStartRequest = uLow;
    pSample->loopEndRequest = uEnd;
    unsigned int uGranularity = AIL_sample_granularity(pSample);
    pSample->loopStart = ((uLow + (uGranularity >> 1)) / uGranularity) * uGranularity;
    if (uEnd != SAMPLE_LOOP_TO_END)
    {
        uEnd = ((uEnd + (uGranularity >> 1)) / uGranularity) * uGranularity;
    }

    pSample->loopEnd = uEnd;
}

extern "C" void AIL_API_set_digital_master_volume_level(_DIG_DRIVER* pDriver, float fLevel)
{
    if (pDriver == NULL)
    {
        return;
    }

    pDriver->masterVolume = fLevel;
}

extern "C" void AIL_API_set_digital_master_reverb_levels(_DIG_DRIVER* pDriver, int iBus, float fWetLevel,
                                                         float fDryLevel)
{
    if (pDriver == NULL)
    {
        return;
    }

    DriverBus* pBus = &pDriver->buses[iBus];
    pBus->returnGain = fWetLevel;
    pBus->sendLevel = fDryLevel;
    SS_update_driver_reverb_state(pDriver);
}

extern "C" void AIL_API_set_digital_master_reverb(_DIG_DRIVER* pDriver, int iBus, float fReverbTime,
                                                  float fReverbPredelay, float fReverbDamping)
{
    if (pDriver == NULL)
    {
        return;
    }

    float fMaximumPredelay = static_cast<float>(AIL_preference[PREFERENCE_REVERB_MILLISECONDS]);
    if (fReverbPredelay > fMaximumPredelay)
    {
        fReverbPredelay = fMaximumPredelay;
    }

    DriverBus* pBus = &pDriver->buses[iBus];
    float fTail = (fReverbTime + fReverbPredelay) * MILLISECONDS_PER_SECOND;
    pBus->returnLevel = fReverbTime;
    pBus->predelay = fReverbPredelay;
    pBus->damping = fReverbDamping;
    pBus->tail = static_cast<int>(fTail);
    SS_update_driver_reverb_state(pDriver);

    unsigned int uPredelay = (static_cast<int>(fReverbPredelay * static_cast<float>(pDriver->outputRate)) * 4
                              + REVERB_ALIGNMENT - 1)
                             & ~(REVERB_ALIGNMENT - 1);
    if (pBus->length <= static_cast<int>(uPredelay))
    {
        uPredelay = pBus->length - 1;
    }

    pBus->readOffset = pDriver->buses[0].writeOffset - uPredelay;
    if (pBus->readOffset < 0)
    {
        pBus->readOffset = pBus->readOffset + pBus->length;
    }

    float fTime = pBus->returnLevel;
    float fDamping = pBus->damping;
    ReverbState& reverb = pBus->reverb;
    reverb.coefficients[0] = static_cast<float>(pow(REVERB_DECAY_LEVEL, REVERB_LINE_SECONDS_A / fTime));
    reverb.coefficients[1] = static_cast<float>(pow(REVERB_DECAY_LEVEL, REVERB_LINE_SECONDS_B / fTime));
    reverb.coefficients[2] = static_cast<float>(pow(REVERB_DECAY_LEVEL, REVERB_LINE_SECONDS_C / fTime));
    reverb.coefficients[3] = static_cast<float>(pow(REVERB_DECAY_LEVEL, REVERB_LINE_SECONDS_D / fTime));
    reverb.coefficients[4] = static_cast<float>(pow(REVERB_DECAY_LEVEL, REVERB_DECAY_EXPONENT_E));
    reverb.coefficients[5] = static_cast<float>(pow(REVERB_DECAY_LEVEL, REVERB_DECAY_EXPONENT_F));
    if (fDamping >= LEVEL_EPSILON)
    {
        float fTangent = ONE / static_cast<float>(tan(REVERB_PI * fDamping * REVERB_DAMPING_ANGLE));
        float fSquare = fTangent * fTangent;
        float fScale = ONE / (static_cast<float>(sqrt(REVERB_SQRT_ARGUMENT)) * fTangent + ONE + fSquare);
        reverb.lowPass[0] = fScale;
        reverb.lowPass[1] = ((ONE - fSquare) + (ONE - fSquare)) * fScale;
        reverb.lowPass[2] = (-(static_cast<float>(sqrt(REVERB_SQRT_ARGUMENT)) * fTangent - ONE) + fSquare)
                            * reverb.lowPass[0];
    }
    else
    {
        reverb.lowPass[0] = ZERO;
        reverb.lowPass[2] = ZERO;
        reverb.lowPass[1] = ZERO;
    }
}

void set_lp_info(LOWPASS_INFO* pInfo, int iFirst, int iSecond, int iThird)
{
    unsigned int uHistoryA = static_cast<unsigned int>(pInfo->historyA) << FIXED_POINT_SHIFT;
    int iPreviousA = static_cast<int>(uHistoryA | (static_cast<unsigned int>(pInfo->historyA) >> FIXED_POINT_SHIFT))
                     >> FIXED_POINT_SHIFT;
    unsigned int uHistoryB = static_cast<unsigned int>(pInfo->historyB) << FIXED_POINT_SHIFT;
    pInfo->coefficientA = (pInfo->coefficientA << FIXED_POINT_SHIFT) | (iFirst & WORD_MASK_16);
    unsigned int uNewB = static_cast<unsigned int>(
        (((pInfo->sampleA + pInfo->sampleB * 3) * iFirst - iSecond * iPreviousA)
         - iThird
               * (static_cast<int>(uHistoryB | (static_cast<unsigned int>(pInfo->historyB) >> FIXED_POINT_SHIFT))
                  >> FIXED_POINT_SHIFT))
        >> LOW_PASS_COEFFICIENT_SHIFT);
    pInfo->coefficientB = (pInfo->coefficientB << FIXED_POINT_SHIFT) | (iSecond & WORD_MASK_16);
    pInfo->coefficientC = (pInfo->coefficientC << FIXED_POINT_SHIFT) | (iThird & WORD_MASK_16);
    pInfo->historyB = uHistoryB | (uNewB & WORD_MASK_16);
    pInfo->historyA = uHistoryA
                      | (((((pInfo->sampleA * 3 + pInfo->sampleB) * iFirst - iSecond * uNewB) - iThird * iPreviousA)
                          >> LOW_PASS_COEFFICIENT_SHIFT)
                         & WORD_MASK_16);
    int iMode = LOW_PASS_MODE_ACTIVE;
    if (pInfo->mode != 0)
    {
        iMode = LOW_PASS_MODE_PENDING;
    }

    pInfo->mode = iMode;
}

extern "C" void AIL_API_start_sample(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return;
    }

    int iStatus = pSample->driver->sampleStatus[pSample->index];
    if (iStatus == SAMPLE_STATUS_FREE)
    {
        return;
    }

    if ((iStatus == SAMPLE_STATUS_PLAYING || iStatus == SAMPLE_STATUS_PLAYING_RELEASED)
        && pSample->loopMode == LOOP_MODE_STREAMING)
    {
        return;
    }

    SampleBuffer* pBuffer = &pSample->buffers[pSample->currentBuffer];
    if (pBuffer->size == 0 || pBuffer->data == NULL)
    {
        return;
    }

    clear_last_sample(pSample);
    pSample->endMode = 0;
    pSample->fadeEnds = 0;
    pSample->resetA = 0;
    pSample->fadePosition = 0;
    pSample->resetB = 0;
    pSample->fadeSamples = 0;
    pSample->phaseMask = 1 << (pSample->driver->phase & SAMPLE_PHASE_MASK);
    pSample->buffers[pSample->currentBuffer].position = 0;
    pSample->bufferRewind = 0;

    DecoderStage* pDecoder = &pSample->stages[SAMPLE_STAGE_DECODER].decoder;
    if (pDecoder->active != 0)
    {
        pDecoder->seek(pDecoder->stream, SAMPLE_BUFFER_CURRENT);
        if (pSample->loopMode == LOOP_MODE_ASI && pDecoder->streamSeekPosition != RIB_ENTRY_NOT_FOUND)
        {
            int* pPosition = &pSample->buffers[pSample->currentBuffer].position;
            pDecoder->property(pDecoder->stream, pDecoder->streamSeekPosition, 0, pPosition, pPosition);
        }
    }

    SS_update_sample_reverb_state(pSample);
    pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_PLAYING;
    SS_start_playback(pSample);
}

extern "C" void AIL_API_stop_sample(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return;
    }

    if (pSample->driver->sampleStatus[pSample->index] != SAMPLE_STATUS_PLAYING)
    {
        return;
    }

    pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_STOPPED;
}

extern "C" void AIL_API_resume_sample(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return;
    }

    int iStatus = pSample->driver->sampleStatus[pSample->index];
    if (iStatus != SAMPLE_STATUS_STOPPED)
    {
        if (iStatus != SAMPLE_STATUS_DONE)
        {
            return;
        }

        if (pSample->buffers[pSample->currentBuffer].size == 0)
        {
            return;
        }

        if (pSample->buffers[pSample->currentBuffer].data == NULL)
        {
            return;
        }
    }

    pSample->fadeEnds = 0;
    pSample->fadeSamples = 0;
    pSample->endMode = 0;
    pSample->fadePosition = 0;
    pSample->phaseMask = 1 << (pSample->driver->phase & SAMPLE_PHASE_MASK);
    clear_last_sample(pSample);
    SS_update_sample_reverb_state(pSample);
    pSample->driver->sampleStatus[pSample->index] = SAMPLE_STATUS_PLAYING;
    SS_start_playback(pSample);
}

extern "C" AIL_FALLOFF_CALLBACK AIL_API_register_falloff_function_callback(_SAMPLE* pSample,
                                                                           AIL_FALLOFF_CALLBACK pCallback)
{
    if (pSample == NULL)
    {
        return NULL;
    }

    AIL_FALLOFF_CALLBACK pPrevious = pSample->falloffFunction;
    if (pCallback == NULL)
    {
        pCallback = SS_default_falloff_function_callback;
    }

    pSample->falloffFunction = pCallback;
    return pPrevious;
}

extern "C" int AIL_API_minimum_sample_buffer_size(_DIG_DRIVER* pDriver, int iPlaybackRate, int iFormat)
{
    int iNibbles = nibbles_per_sample(iFormat);
    int iGranularity = iNibbles / 2;
    if (iGranularity == 0)
    {
        iGranularity = 1;
    }

    int iOutputRate = pDriver->outputRate;
    int iSize = (((pDriver->fillBytes * iNibbles) / (pDriver->speakerConfig * pDriver->bufferCount * 2))
                 * iPlaybackRate)
                / iOutputRate;
    iSize = iSize + (iSize * AIL_preference[PREFERENCE_RESAMPLING_TOLERANCE]) / 32768;
    if (iOutputRate != iPlaybackRate && iOutputRate != iPlaybackRate * 2 && iOutputRate != iPlaybackRate * 4)
    {
        iSize = iSize + 4;
    }

    int iMinimum = ((iSize + BUFFER_SIZE_ROUNDING) & ~BUFFER_SIZE_ROUNDING)
                   * AIL_preference[PREFERENCE_MINIMUM_BUFFER_MARGIN];
    return iMinimum - iMinimum % iGranularity;
}

extern "C" int AIL_API_set_sample_buffer_count(_SAMPLE* pSample, int iCount)
{
    if (pSample == NULL
        || static_cast<unsigned int>(iCount - SAMPLE_MINIMUM_BUFFER_COUNT)
               >= static_cast<unsigned int>(SAMPLE_MAXIMUM_BUFFER_COUNT - 1))
    {
        return 0;
    }

    pSample->currentBuffer = 0;
    pSample->bufferCount = iCount;
    pSample->lastBuffer = 0;
    pSample->starved = 1;
    pSample->bufferRewind = 0;
    memset(pSample->buffers, 0, iCount * sizeof(SampleBuffer));
    for (int iIndex = 0; iIndex < pSample->bufferCount; ++iIndex)
    {
        pSample->buffers[iIndex].data = NULL;
        pSample->buffers[iIndex].size = 0;
        pSample->buffers[iIndex].position = 0;
        pSample->buffers[iIndex].reset = 0;
        pSample->buffers[iIndex].resetPosition = 0;
        pSample->buffers[iIndex].done = (iIndex != 0);
    }

    return 1;
}

extern "C" int AIL_API_load_sample_buffer(_SAMPLE* pSample, int iBuffer, const void* pData, unsigned int uLength)
{
    if (pSample == NULL || pSample->bufferCount <= iBuffer)
    {
        return -1;
    }

    if (iBuffer == SAMPLE_BUFFER_CURRENT)
    {
        iBuffer = pSample->lastBuffer;
        pSample->lastBuffer = (iBuffer + 1) % pSample->bufferCount;
    }

    if (pData != NULL)
    {
        pSample->starved = 0;
        SampleBuffer* pBuffer = &pSample->buffers[iBuffer];
        pBuffer->done = (uLength == 0);
        pBuffer->data = static_cast<const unsigned char*>(pData);
        pBuffer->size = uLength;
        pBuffer->position = 0;
        if (uLength != 0 && pSample->driver->sampleStatus[pSample->index] != SAMPLE_STATUS_PLAYING)
        {
            pSample->loopMode = LOOP_MODE_STREAMING;
        }

        return iBuffer;
    }

    int iNext = (iBuffer + 1) % pSample->bufferCount;
    int iDestination = iBuffer;
    while (iNext != pSample->currentBuffer)
    {
        iBuffer = iNext;
        memmove(&pSample->buffers[iDestination], &pSample->buffers[iBuffer], sizeof(SampleBuffer));
        iDestination = iBuffer;
        iNext = (iBuffer + 1) % pSample->bufferCount;
    }

    int iLast = pSample->bufferCount;
    if (pSample->lastBuffer != 0)
    {
        iLast = pSample->lastBuffer;
    }

    pSample->lastBuffer = iLast - 1;
    return iBuffer;
}

extern "C" int AIL_API_sample_buffer_available(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return -1;
    }

    if (pSample->lastBuffer == pSample->currentBuffer && pSample->starved == 0)
    {
        return 0;
    }

    return 1;
}

extern "C" int AIL_API_sample_loaded_len(_SAMPLE* pSample)
{
    if (pSample == NULL)
    {
        return 0;
    }

    int iIndex = pSample->currentBuffer;
    int iLast = pSample->lastBuffer;
    if (iIndex == iLast && pSample->starved == 0)
    {
        ++iIndex;
        if (iIndex >= pSample->bufferCount)
        {
            iIndex = 0;
        }
    }

    int iTotal = 0;
    for (;;)
    {
        iTotal = iTotal + (pSample->buffers[iIndex].size - pSample->buffers[iIndex].position);
        if (iIndex == iLast)
        {
            break;
        }

        ++iIndex;
        if (iIndex >= pSample->bufferCount)
        {
            iIndex = 0;
        }
    }

    return iTotal;
}

extern "C" void AIL_API_set_sample_ms_position(_SAMPLE* pSample, unsigned int uMilliseconds)
{
    if (pSample == NULL)
    {
        return;
    }

    DecoderStage& decoder = pSample->stages[SAMPLE_STAGE_DECODER].decoder;
    if (decoder.active != 0 && decoder.exactSeek != 0)
    {
        unsigned int uResult = 0;
        if (decoder.property(decoder.stream, decoder.exactSeek, 0, &uMilliseconds, &uResult) == 1)
        {
            AIL_set_sample_position(pSample, uResult);
            return;
        }
    }

    unsigned int uFlags = pSample->channelFlags;
    int iSamplesPerSecond = static_cast<int>(static_cast<float>(pSample->playbackRate) * pSample->playbackRateFactor);
    unsigned int uBytesPerSecond;
    int iPosition;
    if ((uFlags & SAMPLE_FORMAT_ADPCM) == 0)
    {
        if (decoder.active == 0)
        {
            int iNibbles = nibbles_per_sample(uFlags | (pSample->channelCount << NIBBLE_CHANNEL_SHIFT));
            uBytesPerSecond = (iSamplesPerSecond * iNibbles) / 2;
        }
        else
        {
            uBytesPerSecond = ALL_BITS_SET;
            decoder.property(decoder.stream, decoder.inputBitRate, &uBytesPerSecond, 0, 0);
            uBytesPerSecond = uBytesPerSecond >> 3;
        }
    }
    else
    {
        unsigned int uHeader = ADPCM_BLOCK_HEADER_MONO << ((uFlags >> 1) & 1);
        uBytesPerSecond = static_cast<unsigned int>(iSamplesPerSecond * pSample->adpcmBlockSize)
                          / ((((pSample->adpcmBlockSize - uHeader) * ADPCM_BITS_PER_BYTE) / uHeader) + 1);
    }

    iPosition = static_cast<int>(
        (static_cast<float>(uBytesPerSecond) * static_cast<float>(static_cast<int>(uMilliseconds)))
        / MILLISECONDS_PER_SECOND);
    AIL_set_sample_position(pSample, iPosition);
}

namespace
{
const float SPEAKER_DIRECTION_EPSILON = 0.0001f;
const float ANGLE_ALIGNED_DOT = 0.9999f;
const float ANGLE_OPPOSED_DOT = -0.9999f;
const float DISTANCE_GRAPH_DECIBELS = 20.0f;
const double DISTANCE_GRAPH_BASE = 10.0;
const float DISTANCE_GRAPH_SILENCE = 3.0518509e-05f;
const float DOPPLER_THRESHOLD = 0.355f;
const float DOPPLER_MAXIMUM_SCALE = 4.0f;
const float DOPPLER_MINIMUM_SCALE = 0.25f;
const float PI_CONSTANT = 3.1415927f;
const int PATH_WEIGHT_CAPACITY = 10;
const int LEVEL_CHANNEL_CAPACITY = 8;

inline float DistanceOf(float fX, float fY, float fZ)
{
    return static_cast<float>(sqrt(static_cast<double>(fZ * fZ + fX * fX + fY * fY)));
}
}

extern "C" void SS_set_speaker_configuration(SpeakerState* pState, const float* pPositions, unsigned int uProvided,
                                             unsigned int uSpeakers)
{
    if (static_cast<int>(uSpeakers) < static_cast<int>(uProvided))
    {
        uProvided = uSpeakers;
    }

    for (int iSpeaker = 0; iSpeaker < static_cast<int>(uProvided); ++iSpeaker)
    {
        pState->positions[iSpeaker][0] = pPositions[iSpeaker * 3];
        pState->positions[iSpeaker][1] = pPositions[iSpeaker * 3 + 1];
        pState->positions[iSpeaker][2] = pPositions[iSpeaker * 3 + 2];
    }

    pState->directionCount = 0;
    pState->lowFrequencyCount = 0;
    pState->speakerCount = 0;

    float fSumX = ZERO;
    float fSumY = ZERO;
    float fSumZ = ZERO;
    for (int iSpeaker = 0; iSpeaker < static_cast<int>(uSpeakers); ++iSpeaker)
    {
        float fX = pState->positions[iSpeaker][0];
        float fY = pState->positions[iSpeaker][1];
        float fZ = pState->positions[iSpeaker][2];
        float fLength = DistanceOf(fX, fY, fZ);
        if (fLength < SPEAKER_DIRECTION_EPSILON)
        {
            pState->lowFrequencySpeakers[pState->lowFrequencyCount++] = iSpeaker;
            continue;
        }

        fX = fX / fLength;
        fY = fY / fLength;
        fZ = fZ / fLength;
        int iRegular = pState->speakerCount++;
        pState->speakerChannels[iRegular] = iSpeaker;
        float fLocalX = fZ * pState->listenerFace[0] + fX * pState->listenerCross[0] + fY * pState->listenerUp[0];
        pState->speakerDirections[iRegular][0] = fLocalX;
        float fLocalY = fZ * pState->listenerFace[1] + fX * pState->listenerCross[1] + fY * pState->listenerUp[1];
        pState->speakerDirections[iRegular][1] = fLocalY;
        float fLocalZ = fZ * pState->listenerFace[2] + fX * pState->listenerCross[2] + fY * pState->listenerUp[2];
        pState->speakerDirections[iRegular][2] = fLocalZ;
        SpeakerDirection* pDirection = &pState->directions[pState->directionCount];
        pDirection->x = fLocalX;
        pDirection->y = fLocalY;
        pDirection->z = fLocalZ;
        pDirection->members[0] = iRegular;
        pDirection->gains[0] = ONE;
        pDirection->memberCount = 1;
        fSumX = fSumX + fLocalX;
        fSumY = fSumY + fLocalY;
        fSumZ = fSumZ + fLocalZ;
        ++pState->directionCount;
    }

    if (pState->speakerCount <= 0)
    {
        return;
    }

    float fCount = static_cast<float>(pState->speakerCount);
    fSumY = fSumY / fCount;
    fSumX = fSumX / fCount;
    fSumZ = fSumZ / fCount;
    float fMagnitude = DistanceOf(fSumX, fSumY, fSumZ);
    if (fMagnitude <= SPEAKER_DIRECTION_EPSILON)
    {
        return;
    }

    int iCount = pState->speakerCount;
    fSumZ = -(fSumZ / fMagnitude);
    fSumY = -(fSumY / fMagnitude);
    fSumX = -(fSumX / fMagnitude);
    int iFound = 0;
    while (iFound < iCount)
    {
        const float* pDirection = pState->speakerDirections[iFound];
        if (ZERO < pDirection[2] * fSumZ + pDirection[0] * fSumX + pDirection[1] * fSumY)
        {
            break;
        }

        ++iFound;
    }

    if (iFound == iCount)
    {
        int iVirtual = pState->speakerCount++;
        pState->speakerChannels[iVirtual] = -1;
        pState->speakerDirections[iVirtual][0] = fSumX;
        pState->speakerDirections[iVirtual][1] = fSumY;
        pState->speakerDirections[iVirtual][2] = fSumZ;
        SpeakerDirection* pDirection = &pState->directions[pState->directionCount];
        pDirection->x = pState->speakerDirections[iVirtual][0];
        pDirection->y = pState->speakerDirections[iVirtual][1];
        pDirection->z = pState->speakerDirections[iVirtual][2];
        pDirection->members[0] = iVirtual;
        pDirection->gains[0] = ONE;
        ++pState->directionCount;
        pDirection->memberCount = 1;
    }
}

extern "C" void SS_calculate_3D_channel_levels(SpeakerState* pSpeakers, unsigned int uCount, _SAMPLE* pSample,
                                               float* pLevels)
{
    float fWeights[PATH_WEIGHT_CAPACITY];
    float fAccumulators[LEVEL_CHANNEL_CAPACITY];
    float fDirections[PATH_WEIGHT_CAPACITY][3];
    float fAngleTerms[SPEAKER_DIRECTION_COUNT];
    FalloffFunc pFalloff = pSample->falloffFunction;
    unsigned int uCandidates = 1;
    float fDistance;
    float fExclusion = NO_FALLOFF_RESULT;

    if (pSample->pathPointCount < FALLOFF_POLYLINE_MINIMUM)
    {
        fDirections[0][0] = pSample->x - pSpeakers->listenerPosition[0];
        fDirections[0][1] = pSample->y - pSpeakers->listenerPosition[1];
        fDirections[0][2] = pSample->z - pSpeakers->listenerPosition[2];
        fDistance = DistanceOf(fDirections[0][0], fDirections[0][1], fDirections[0][2]);
        fWeights[0] = ONE;
    }
    else
    {
        uCandidates = 0;
        unsigned int uClosest = ALL_BITS_SET;
        float fBest = NO_FALLOFF_RESULT;
        for (int iSegment = 0; iSegment < pSample->pathPointCount - 1; ++iSegment)
        {
            const float* pStart = pSample->pathPoints[iSegment];
            const float* pEnd = pSample->pathPoints[iSegment + 1];
            double dX = pEnd[0] - pStart[0];
            double dY = pEnd[1] - pStart[1];
            double dZ = pEnd[2] - pStart[2];
            double dLengthSquared = static_cast<float>(dZ * dZ
                                                       + static_cast<float>(dX * dX + static_cast<float>(dY * dY)));
            if (dLengthSquared > SPEAKER_DIRECTION_EPSILON)
            {
                const float* pListener = pSpeakers->listenerPosition;
                double dT = static_cast<float>(
                    (static_cast<float>(pListener[2] - pStart[2]) * dZ
                     + static_cast<float>(static_cast<float>(pListener[0] - pStart[0]) * dX
                                          + static_cast<float>(static_cast<float>(pListener[1] - pStart[1]) * dY)))
                    / dLengthSquared);
                if (dT < ZERO)
                {
                    dT = ZERO;
                }
                else if (dT > ONE)
                {
                    dT = ONE;
                }

                float* pCandidate = fDirections[uCandidates];
                pCandidate[0] = pListener[0] - static_cast<float>(dX * dT + pStart[0]);
                pCandidate[1] = pListener[1] - static_cast<float>(dY * dT + pStart[1]);
                pCandidate[2] = pListener[2] - static_cast<float>(dZ * dT + pStart[2]);
                float fCandidateSquared = pCandidate[2] * pCandidate[2] + pCandidate[0] * pCandidate[0]
                                          + pCandidate[1] * pCandidate[1];
                if (fBest < ZERO || fCandidateSquared < fBest)
                {
                    fBest = fCandidateSquared;
                    uClosest = uCandidates;
                }

                ++uCandidates;
            }
        }

        if (fBest < ZERO)
        {
            fDirections[0][1] = pSample->pathPoints[0][1] - pSpeakers->listenerPosition[1];
            fDirections[0][0] = pSample->pathPoints[0][0] - pSpeakers->listenerPosition[0];
            fDirections[0][2] = pSample->pathPoints[0][2] - pSpeakers->listenerPosition[2];
        }

        if (uClosest != 0 && uClosest != ALL_BITS_SET)
        {
            float fX = fDirections[uClosest][0];
            float fY = fDirections[uClosest][1];
            float fZ = fDirections[uClosest][2];
            fDirections[uClosest][0] = fDirections[0][0];
            fDirections[uClosest][1] = fDirections[0][1];
            fDirections[uClosest][2] = fDirections[0][2];
            fDirections[0][0] = fX;
            fDirections[0][1] = fY;
            fDirections[0][2] = fZ;
        }

        fDistance = sqrtf(fBest);
        if (fDistance < pSample->minimumDistance && pSample->falloffPointCounts[0] == 0
            && pSample->falloffPointCounts[3] == 0)
        {
            fExclusion = ONE - fDistance / pSample->minimumDistance;
        }

        fWeights[0] = ONE;
        for (unsigned int uCandidate = 1; uCandidate < uCandidates; ++uCandidate)
        {
            float* pCandidate = fDirections[uCandidate];
            float fLength = DistanceOf(pCandidate[0], pCandidate[1], pCandidate[2]);
            float fWeight = (fDistance / fLength) * (fDistance / fLength);
            fWeights[uCandidate] = fWeight;
            fWeights[0] = fWeights[0] + fWeight;
            if (fLength < SPEAKER_DIRECTION_EPSILON)
            {
                pCandidate[0] = ONE;
                pCandidate[1] = ZERO;
                pCandidate[2] = ZERO;
            }
            else
            {
                pCandidate[0] = pCandidate[0] / fLength;
                pCandidate[1] = pCandidate[1] / fLength;
                pCandidate[2] = pCandidate[2] / fLength;
            }
        }
    }

    float fBaseLength = DistanceOf(fDirections[0][0], fDirections[0][1], fDirections[0][2]);
    if (fBaseLength < SPEAKER_DIRECTION_EPSILON)
    {
        fDirections[0][1] = ZERO;
        fDirections[0][0] = ONE;
        fDirections[0][2] = ZERO;
    }
    else
    {
        fDirections[0][0] = fDirections[0][0] / fBaseLength;
        fDirections[0][1] = fDirections[0][1] / fBaseLength;
        fDirections[0][2] = fDirections[0][2] / fBaseLength;
    }

    pSample->falloffVolume = NO_FALLOFF_RESULT;
    pSample->falloffLowPass = NO_FALLOFF_RESULT;
    if (pSample->maximumDistance < fDistance)
    {
        fDistance = pSample->maximumDistance;
        if (pSpeakers->active != 0)
        {
            pSample->distanceVolume = ZERO;
            for (unsigned int uChannel = 0; static_cast<int>(uChannel) < static_cast<int>(uCount); ++uChannel)
            {
                pLevels[uChannel] = ZERO;
            }

            return;
        }
    }

    if (pSample->falloffPointCounts[1] != 0)
    {
        pSample->falloffVolume = static_cast<float>(evaluate_graph(
            fDistance, reinterpret_cast<const GraphPoint*>(pSample->falloffPoints[1]), pSample->falloffPointCounts[1]));
    }

    if (pSample->falloffPointCounts[2] != 0)
    {
        pSample->falloffLowPass = static_cast<float>(evaluate_graph(
            fDistance, reinterpret_cast<const GraphPoint*>(pSample->falloffPoints[2]), pSample->falloffPointCounts[2]));
    }

    if (pSample->falloffPointCounts[3] != 0)
    {
        pSample->falloffExclusion = static_cast<float>(evaluate_graph(
            fDistance, reinterpret_cast<const GraphPoint*>(pSample->falloffPoints[3]), pSample->falloffPointCounts[3]));
    }

    pSample->distanceVolume = ONE;
    float fGain;
    if (pSample->falloffPointCounts[0] == 0)
    {
        float fMinimum = pSample->minimumDistance;
        if (fMinimum < SPEAKER_DIRECTION_EPSILON)
        {
            fMinimum = SPEAKER_DIRECTION_EPSILON;
        }

        fGain = ONE;
        if (static_cast<float>(fMinimum + SPEAKER_DIRECTION_EPSILON) < fDistance)
        {
            if (pFalloff == NULL)
            {
                pFalloff = SS_default_falloff_function_callback;
            }

            fGain = pFalloff(pSample->falloffUser, fDistance, pSpeakers->rolloffFactor, fMinimum,
                             pSample->maximumDistance);
            pSample->distanceVolume = fGain;
        }
    }
    else
    {
        float fDecibels = static_cast<float>(evaluate_graph(
            fDistance, reinterpret_cast<const GraphPoint*>(pSample->falloffPoints[0]), pSample->falloffPointCounts[0]));
        double dLinear = pow(DISTANCE_GRAPH_BASE, static_cast<double>(fDecibels / DISTANCE_GRAPH_DECIBELS));
        fGain = static_cast<float>(dLinear);
        if (fGain <= DISTANCE_GRAPH_SILENCE)
        {
            pSample->distanceVolume = ZERO;
            fGain = ZERO;
        }
        else
        {
            pSample->distanceVolume = static_cast<float>(dLinear);
        }
    }

    if (pSample->coneEnabled != 0)
    {
        double dDot = -pSample->forwardZ * fDirections[0][2] + -pSample->forwardX * fDirections[0][0]
                      + -pSample->forwardY * fDirections[0][1];
        float fAngle;
        bool bOutside;
        if (dDot <= ANGLE_ALIGNED_DOT)
        {
            if (ANGLE_OPPOSED_DOT <= dDot)
            {
                fAngle = static_cast<float>(acos(dDot));
            }
            else
            {
                fAngle = PI_CONSTANT;
            }
        }
        else
        {
            fAngle = ZERO;
        }

        bOutside = pSample->coneOuter < fAngle;
        if (bOutside)
        {
            fGain = static_cast<float>(fGain * pSample->coneOuterVolume);
            pSample->distanceVolume = fGain;
        }
        else if (pSample->coneInner <= fAngle
                 && static_cast<float>(pSample->coneOuter - pSample->coneInner) > SPEAKER_DIRECTION_EPSILON)
        {
            float fRange = static_cast<float>(pSample->coneOuter - pSample->coneInner);
            fGain = static_cast<float>(
                fGain
                * static_cast<float>(static_cast<float>(ONE - pSample->coneOuterVolume)
                                         * static_cast<float>(static_cast<float>(pSample->coneOuter - fAngle) / fRange)
                                     + pSample->coneOuterVolume));
            pSample->distanceVolume = fGain;
        }
    }

    if (fExclusion == NO_FALLOFF_RESULT)
    {
        fExclusion = pSample->falloffExclusion;
    }

    if (fDistance > SPEAKER_DIRECTION_EPSILON)
    {
        for (int iChannel = 0; iChannel < LEVEL_CHANNEL_CAPACITY; ++iChannel)
        {
            fAccumulators[iChannel] = ZERO;
        }

        float fShare = static_cast<float>(
            sqrt(static_cast<double>(ONE / static_cast<float>(static_cast<int>(uCount)))));
        for (int iLow = 0; iLow < pSpeakers->lowFrequencyCount; ++iLow)
        {
            fAccumulators[pSpeakers->lowFrequencySpeakers[iLow]] = static_cast<float>(fGain * fShare);
        }

        float fExclusionScale = ONE - fExclusion;
        for (unsigned int uCandidate = 0; static_cast<int>(uCandidate) < static_cast<int>(uCandidates); ++uCandidate)
        {
            float fWeight = fWeights[uCandidate] / fWeights[0];
            const float* pEmitter = fDirections[uCandidate];
            float fTotal = ZERO;
            for (int iDirection = 0; iDirection < pSpeakers->directionCount; ++iDirection)
            {
                const SpeakerDirection& direction = pSpeakers->directions[iDirection];
                float fDot = direction.z * pEmitter[2] + direction.x * pEmitter[0] + direction.y * pEmitter[1];
                float fTerm;
                if (fDot > ANGLE_ALIGNED_DOT)
                {
                    fTerm = -(fExclusionScale * (PI_CONSTANT - PI_CONSTANT) - PI_CONSTANT);
                }
                else if (fDot < ANGLE_OPPOSED_DOT)
                {
                    fTerm = -(fExclusionScale * (PI_CONSTANT - ZERO) - PI_CONSTANT);
                }
                else
                {
                    float fRadians = static_cast<float>(acos(fDot));
                    fTerm = -(fExclusionScale
                                  * static_cast<float>(PI_CONSTANT - static_cast<float>(PI_CONSTANT - fRadians))
                              - PI_CONSTANT);
                }

                fAngleTerms[iDirection] = fTerm;
                fTotal = fTotal + fTerm;
            }

            for (int iDirection = 0; iDirection < pSpeakers->directionCount; ++iDirection)
            {
                const SpeakerDirection& direction = pSpeakers->directions[iDirection];
                double dAmplitude = sqrt(
                    pow(static_cast<double>(fAngleTerms[iDirection]), static_cast<double>(pSpeakers->panExponent))
                    / fTotal);
                for (int iMember = 0; iMember < direction.memberCount; ++iMember)
                {
                    int iChannel = pSpeakers->speakerChannels[direction.members[iMember]];
                    float fContribution = fWeight * static_cast<float>(dAmplitude) * direction.gains[iMember];
                    if (iChannel != -1)
                    {
                        fAccumulators[iChannel] = fAccumulators[iChannel] + fContribution;
                    }
                    else
                    {
                        for (int iOther = 0; iOther < pSpeakers->directionCount; ++iOther)
                        {
                            const SpeakerDirection& spread = pSpeakers->directions[iOther];
                            for (int iSpread = 0; iSpread < spread.memberCount; ++iSpread)
                            {
                                int iSpreadChannel = pSpeakers->speakerChannels[spread.members[iSpread]];
                                if (iSpreadChannel != -1)
                                {
                                    fAccumulators[iSpreadChannel] =
                                        fAccumulators[iSpreadChannel]
                                        + fContribution / static_cast<float>(pSpeakers->directionCount * 2);
                                }
                            }
                        }
                    }
                }
            }
        }

        float fNormaliser = static_cast<float>(sqrt(static_cast<double>(pSpeakers->panExponent)))
                            * static_cast<float>(uCandidates);
        for (int iChannel = 0; iChannel < static_cast<int>(uCount); ++iChannel)
        {
            double dLevel = fAccumulators[iChannel] / fNormaliser;
            if (dLevel > ZERO)
            {
                if (dLevel >= ONE)
                {
                    dLevel = ONE;
                }

                pLevels[iChannel] = static_cast<float>(dLevel * fGain);
            }
            else
            {
                pLevels[iChannel] = static_cast<float>(ZERO * fGain);
            }
        }
    }
    else
    {
        for (int iChannel = 0; iChannel < static_cast<int>(uCount); ++iChannel)
        {
            pLevels[iChannel] = static_cast<float>(
                fGain
                * static_cast<float>(sqrt(static_cast<double>(ONE / static_cast<float>(static_cast<int>(uCount))))));
        }
    }

    float fVelocityX = pSample->velocityX - pSpeakers->listenerVelocity[0];
    float fVelocityY = pSample->velocityY - pSpeakers->listenerVelocity[1];
    float fVelocityZ = pSample->velocityZ - pSpeakers->listenerVelocity[2];
    float fSpeed = DistanceOf(fVelocityX, fVelocityY, fVelocityZ);
    pSample->dopplerActive = 0;
    if (fSpeed < SPEAKER_DIRECTION_EPSILON)
    {
        fVelocityX = ZERO;
        fVelocityY = ZERO;
        fVelocityZ = ZERO;
    }
    else
    {
        fVelocityY = fVelocityY / fSpeed;
        fVelocityX = fVelocityX / fSpeed;
        fVelocityZ = fVelocityZ / fSpeed;
    }

    if (fSpeed > SPEAKER_DIRECTION_EPSILON)
    {
        float fRadial =
            static_cast<float>(
                fSpeed
                * static_cast<float>(fDirections[0][2] * fVelocityZ
                                     + static_cast<float>(fDirections[0][0] * fVelocityX
                                                          + static_cast<float>(fDirections[0][1] * fVelocityY))))
            * pSpeakers->distanceFactor * pSpeakers->dopplerFactor;
        float fScale = DOPPLER_MINIMUM_SCALE;
        if (DOPPLER_THRESHOLD < fRadial)
        {
            pSample->dopplerScale = fScale;
        }
        else if (fRadial < -DOPPLER_THRESHOLD)
        {
            pSample->dopplerScale = DOPPLER_MAXIMUM_SCALE;
        }
        else
        {
            fScale = DOPPLER_THRESHOLD / (fRadial + DOPPLER_THRESHOLD);
            if (DOPPLER_MAXIMUM_SCALE < fScale)
            {
                pSample->dopplerScale = DOPPLER_MAXIMUM_SCALE;
            }
            else if (fScale < DOPPLER_MINIMUM_SCALE)
            {
                pSample->dopplerScale = DOPPLER_MINIMUM_SCALE;
            }
            else
            {
                pSample->dopplerScale = fScale;
            }
        }

        pSample->dopplerActive = 1;
    }
}

namespace
{
const unsigned int MIX_FLAG_STEREO_DESTINATION = 1;
const unsigned int MIX_FLAG_16_BITS = 2;
const unsigned int MIX_FLAG_STEREO_SOURCE = 8;
const unsigned int MIX_FLAG_LOW_VOLUME = 0x20;
const int MIX_LOW_VOLUME_LIMIT = 0x800;
const int MIX_VOLUME_SHIFT = 16;
const int MIX_DESTINATION_ALIGNMENT = 0xf;
const int MIX_DESTINATION_PAIR_ALIGNMENT = 8;
const int FILTER_CHANNEL_SEARCH_LENGTH = 17;
const int LIMITER_CHUNK_FRAMES = 0x100;
const int LIMITER_THRESHOLD = 0x1770000;
const int LIMITER_SLOPE = -0x1000;
const int LIMITER_DIVISOR = 0x511f;
const int LIMITER_UNITY = 0x7fff;
const int LIMITER_SMOOTHING_LIMIT = 200;
const int LIMITER_PEAK_SHIFT = 11;
const int LIMITER_GAIN_SHIFT = 15;
const int OUTPUT_SAMPLE_SHIFT = 11;
const int OUTPUT_SAMPLE_MINIMUM = -0x7fff;
const int OUTPUT_SAMPLE_MAXIMUM = 0x7ffe;
const int OUTPUT_SAMPLE_CLIPPED_LOW = 0x80;
const int OUTPUT_SAMPLE_CLIPPED_HIGH = 0x7fff;
const float REVERB_RIGHT_FEEDBACK = 0.15f;
const float REVERB_LEFT_FEEDBACK = 0.25f;
const float REVERB_DENORMAL_BIAS = 2.3841858e-06f;
const int FILTER_SAMPLE_PROPERTY_FLAG = 0x80;
const unsigned long long SAMPLE_START_DELAY_UNITS = 256000;
const int FINAL_BUS = 4;
const int PROFILE_MINIMUM_TICKS = 500;
const int TIMER_ADPCM = 0;
const int TIMER_DEINTERLEAVE = 1;
const int TIMER_MIX = 2;
const int TIMER_REVERB = 3;
const int TIMER_CONVERSION = 4;
const int TIMER_LOW_PASS = 5;
const int TIMER_FILTERS = 6;
const int TIMER_FINAL_COPY = 7;
const int TIMER_START_CALLBACKS = 8;
const int TIMER_END_BUFFER_CALLBACKS = 9;
const int TIMER_END_SAMPLE_CALLBACKS = 10;
const int TIMER_SPU_WAIT = 11;
const int TIMER_DECODER_BASE = 12;
const int TIMER_BINK = 13;

struct ProfileEntry
{
    int timer;
    int count;
    int flags;
    int line;
    const char* label;
};

struct ProfilePlot
{
    int count;
    const char* label;
};

typedef void (*ProfileTickFunc)(MilesProfile* pProfile);
typedef void (*ProfileZoneFunc)(MilesProfile* pProfile, const unsigned long long* pStart, unsigned int uHigh,
                                unsigned int uLow, int iCount, int iFlags, const char* pFile, int iLine,
                                MilesProfileSite* pSite, const char* pLabel);

const char PROFILE_ADPCM_LABEL[] = "ADCPM decompression";
const char PROFILE_DEINTERLACE_LABEL[] = "Input deinterlace";
const char PROFILE_MIX_LABEL[] = "Mix";
const char PROFILE_REVERB_LABEL[] = "Reverb mix";
const char PROFILE_CONVERSION_LABEL[] = "Format conversion";
const char PROFILE_LOW_PASS_LABEL[] = "Low-pass";
const char PROFILE_FILTERS_LABEL[] = "DSP filters";
const char PROFILE_COPY_LABEL[] = "Copy to final output";
const char PROFILE_START_CALLBACK_LABEL[] = "Start of buffer callbacks";
const char PROFILE_END_BUFFER_LABEL[] = "End of buffer callbacks";
const char PROFILE_END_SAMPLE_LABEL[] = "End of sample callbacks";
const char PROFILE_SPU_LABEL[] = "Waiting on SPU";
const char PROFILE_OTHER_LABEL[] = "Other decompression";
const char PROFILE_BINK_LABEL[] = "Bink Audio decompression";
const char PROFILE_MP3_LABEL[] = "MP3 decompression";
const char PROFILE_OGG_LABEL[] = "Ogg decompression";
const char PROFILE_PLOT_ADPCM[] = "Miles/ADPCM (samples)";
const char PROFILE_PLOT_DEINTERLACE[] = "Miles/Input deinterlace (samples)";
const char PROFILE_PLOT_MIX[] = "Miles/Mix (samples)";
const char PROFILE_PLOT_REVERB[] = "Miles/Reverb mix (samples)";
const char PROFILE_PLOT_CONVERSION[] = "Miles/Format conversion (samples)";
const char PROFILE_PLOT_LOW_PASS[] = "Miles/Low-pass (samples)";
const char PROFILE_PLOT_FILTERS[] = "Miles/DSP (samples)";
const char PROFILE_PLOT_OTHER[] = "Miles/Other codec (samples)";
const char PROFILE_PLOT_BINK[] = "Miles/Bink Audio (samples)";
const char PROFILE_PLOT_MP3[] = "Miles/MP3 (samples)";
const char PROFILE_PLOT_OGG[] = "Miles/Ogg (samples)";
const int PROFILE_NO_COUNT = -1;
const int PROFILE_FLAG_WAIT = 2;
const int PROFILE_PLOT_FLAGS = 3;
const int PROFILE_LEAVE_LINE = 0x313b;

const ProfileEntry PROFILE_ENTRIES[] = {
    {0, 0, 0, PROFILE_SITE_BASE + 0, PROFILE_ADPCM_LABEL},
    {1, 1, 0, PROFILE_SITE_BASE + 1, PROFILE_DEINTERLACE_LABEL},
    {2, 2, 0, PROFILE_SITE_BASE + 2, PROFILE_MIX_LABEL},
    {3, 3, 0, PROFILE_SITE_BASE + 3, PROFILE_REVERB_LABEL},
    {4, 4, 0, PROFILE_SITE_BASE + 4, PROFILE_CONVERSION_LABEL},
    {5, 5, 0, PROFILE_SITE_BASE + 5, PROFILE_LOW_PASS_LABEL},
    {6, 6, 0, PROFILE_SITE_BASE + 6, PROFILE_FILTERS_LABEL},
    {7, PROFILE_NO_COUNT, 0, PROFILE_SITE_BASE + 7, PROFILE_COPY_LABEL},
    {8, PROFILE_NO_COUNT, 0, PROFILE_SITE_BASE + 8, PROFILE_START_CALLBACK_LABEL},
    {9, PROFILE_NO_COUNT, 0, PROFILE_SITE_BASE + 9, PROFILE_END_BUFFER_LABEL},
    {10, PROFILE_NO_COUNT, 0, PROFILE_SITE_BASE + 10, PROFILE_END_SAMPLE_LABEL},
    {11, PROFILE_NO_COUNT, PROFILE_FLAG_WAIT, PROFILE_SITE_BASE + 11, PROFILE_SPU_LABEL},
    {12, 7, 0, PROFILE_SITE_BASE + 12, PROFILE_OTHER_LABEL},
    {13, 8, 0, PROFILE_SITE_BASE + 13, PROFILE_BINK_LABEL},
    {14, 9, 0, PROFILE_SITE_BASE + 14, PROFILE_MP3_LABEL},
    {15, 10, 0, PROFILE_SITE_BASE + 15, PROFILE_OGG_LABEL}};
const int PROFILE_ENTRY_COUNT = sizeof(PROFILE_ENTRIES) / sizeof(PROFILE_ENTRIES[0]);

const ProfilePlot PROFILE_PLOTS[] = {
    {0, PROFILE_PLOT_ADPCM},      {1, PROFILE_PLOT_DEINTERLACE}, {2, PROFILE_PLOT_MIX},     {3, PROFILE_PLOT_REVERB},
    {4, PROFILE_PLOT_CONVERSION}, {5, PROFILE_PLOT_LOW_PASS},    {6, PROFILE_PLOT_FILTERS}, {7, PROFILE_PLOT_OTHER},
    {8, PROFILE_PLOT_BINK},       {9, PROFILE_PLOT_MP3},         {10, PROFILE_PLOT_OGG}};
const int PROFILE_PLOT_COUNT = sizeof(PROFILE_PLOTS) / sizeof(PROFILE_PLOTS[0]);

const unsigned int TIME_BASE_RETRY_LIMIT = 500;

MilesProfileSite PROFILE_SITES[DRIVER_PROFILE_TIMER_COUNT];
MilesProfileSite PROFILE_PLOT_SITES[DRIVER_PROFILE_SLOT_COUNT];

#ifdef __ghs__
inline unsigned long long ReadTimeBase()
{
    unsigned int uLow = __MFTB();
    unsigned int uHigh = __MFTBU();
    if (uLow < TIME_BASE_RETRY_LIMIT)
    {
        uLow = __MFTB();
        uHigh = __MFTBU();
    }

    return (static_cast<unsigned long long>(uHigh) << 32) | uLow;
}
#else
inline unsigned long long ReadTimeBase()
{
    return OSGetSystemTime();
}
#endif

inline void StartTimer(MixTimer* pTimer)
{
    if (MSS_profile == NULL)
    {
        return;
    }

    unsigned long long uNow = ReadTimeBase();
    unsigned long long uTotal = (static_cast<unsigned long long>(pTimer->high) << 32) | pTimer->low;
    uTotal = uTotal - uNow;
    pTimer->high = static_cast<unsigned int>(uTotal >> 32);
    pTimer->low = static_cast<unsigned int>(uTotal);
}

inline void StopTimer(MixTimer* pTimer)
{
    if (MSS_profile == NULL)
    {
        return;
    }

    unsigned long long uNow = ReadTimeBase();
    unsigned long long uTotal = (static_cast<unsigned long long>(pTimer->high) << 32) | pTimer->low;
    uTotal = uTotal + uNow;
    pTimer->high = static_cast<unsigned int>(uTotal >> 32);
    pTimer->low = static_cast<unsigned int>(uTotal);
}

inline void MarkProfileSample(_SAMPLE* pSample, int iSlot)
{
    if (MSS_profile == NULL)
    {
        return;
    }

    _DIG_DRIVER* pDriver = pSample->driver;
    if (pDriver->profileSamples[iSlot] != pSample)
    {
        pDriver->profileSamples[iSlot] = pSample;
        ++pDriver->profileCounts[iSlot];
    }
}

#ifdef __ghs__
inline short ReadSample(const unsigned char* pBytes)
{
    short sValue;
    __lhbrx(pBytes, sValue);
    return sValue;
}
#else
inline short ReadSample(const unsigned char* pBytes)
{
    return static_cast<short>(ReadLittleEndian16(pBytes));
}
#endif

inline short ConvertOutputSample(int iValue)
{
    int iScaled = iValue >> OUTPUT_SAMPLE_SHIFT;
    if (iScaled <= OUTPUT_SAMPLE_MINIMUM - 1)
    {
        return static_cast<short>(OUTPUT_SAMPLE_CLIPPED_LOW);
    }

    if (iScaled > OUTPUT_SAMPLE_MAXIMUM)
    {
        iScaled = OUTPUT_SAMPLE_CLIPPED_HIGH;
    }

    return static_cast<short>((static_cast<unsigned short>(iScaled) >> 8) | (static_cast<unsigned char>(iScaled) << 8));
}
}

unsigned int slow_ramp_mix(int iStereoSource, int iStereoDestination, void** ppSource, void* pSourceEnd,
                           void** ppDestination, void* pDestinationEnd, int* piLeft, int* piRight, int iLeftStep,
                           int iRightStep, int iCount)
{
    const unsigned char* pSource = static_cast<const unsigned char*>(*ppSource);
    int* pDestination = static_cast<int*>(*ppDestination);
    unsigned int uFlags = MIX_FLAG_16_BITS;
    if (*piLeft < MIX_LOW_VOLUME_LIMIT || *piRight < MIX_LOW_VOLUME_LIMIT)
    {
        uFlags = MIX_FLAG_16_BITS | MIX_FLAG_LOW_VOLUME;
    }

    int iRight = *piRight << MIX_VOLUME_SHIFT;
    iLeftStep = iLeftStep << MIX_VOLUME_SHIFT;
    int iLeft = *piLeft << MIX_VOLUME_SHIFT;
    iRightStep = iRightStep << MIX_VOLUME_SHIFT;
    int* pDestinationLimit = static_cast<int*>(pDestinationEnd);
    const unsigned char* pSourceLimit = static_cast<const unsigned char*>(pSourceEnd);

    if (iStereoSource == 0)
    {
        if (iStereoDestination == 0)
        {
            if ((reinterpret_cast<uintptr_t>(pDestination + iCount) & MIX_DESTINATION_ALIGNMENT) != 0)
            {
                iCount = iCount
                         + ((MIX_ALIGNMENT_BYTES
                             - (reinterpret_cast<uintptr_t>(pDestination + iCount) & MIX_DESTINATION_ALIGNMENT))
                            >> 2);
                if (pDestinationLimit < pDestination + iCount)
                {
                    iCount = static_cast<int>(reinterpret_cast<intptr_t>(pDestinationLimit) - reinterpret_cast<intptr_t>(pDestination)) / 4;
                }

                int iSourceFrames = (pSourceLimit - pSource) / 2;
                if (iSourceFrames < iCount)
                {
                    iCount = static_cast<unsigned int>(pSourceLimit - pSource) >> 1;
                }
            }

            if (iLeftStep != 0 && 1 < iCount)
            {
                iLeftStep = iLeftStep / iCount;
            }

            for (int iFrame = 0; iFrame < iCount; ++iFrame)
            {
                *pDestination = *pDestination + ReadSample(pSource) * (iLeft >> MIX_VOLUME_SHIFT);
                ++pDestination;
                pSource += 2;
                iLeft = iLeft + iLeftStep;
            }
        }
        else
        {
            uFlags = uFlags | MIX_FLAG_STEREO_DESTINATION;
            if ((reinterpret_cast<uintptr_t>(pDestination + iCount * 2) & MIX_DESTINATION_PAIR_ALIGNMENT) != 0)
            {
                iCount = iCount + 1;
                if (pDestinationLimit < pDestination + 2)
                {
                    iCount = static_cast<int>(reinterpret_cast<intptr_t>(pDestinationLimit) - reinterpret_cast<intptr_t>(pDestination)) / 8;
                }

                int iSourceFrames = (pSourceLimit - pSource) / 2;
                if (iSourceFrames < iCount)
                {
                    iCount = static_cast<unsigned int>(pSourceLimit - pSource) >> 1;
                }
            }

            if (iLeftStep != 0 && 1 < iCount)
            {
                iLeftStep = iLeftStep / iCount;
            }

            if (iRightStep != 0 && 1 < iCount)
            {
                iRightStep = iRightStep / iCount;
            }

            for (int iFrame = 0; iFrame < iCount; ++iFrame)
            {
                short sSample = ReadSample(pSource);
                pDestination[0] = pDestination[0] + sSample * (iLeft >> MIX_VOLUME_SHIFT);
                pDestination[1] = pDestination[1] + sSample * (iRight >> MIX_VOLUME_SHIFT);
                pDestination += 2;
                pSource += 2;
                iLeft = iLeft + iLeftStep;
                iRight = iRight + iRightStep;
            }
        }
    }
    else
    {
        uFlags = uFlags | MIX_FLAG_STEREO_SOURCE;
        if (iStereoDestination == 0)
        {
            if ((reinterpret_cast<uintptr_t>(pDestination + iCount) & MIX_DESTINATION_ALIGNMENT) != 0)
            {
                iCount = iCount
                         + ((MIX_ALIGNMENT_BYTES
                             - (reinterpret_cast<uintptr_t>(pDestination + iCount) & MIX_DESTINATION_ALIGNMENT))
                            >> 2);
                if (pDestinationLimit < pDestination + iCount)
                {
                    iCount = static_cast<int>(reinterpret_cast<intptr_t>(pDestinationLimit) - reinterpret_cast<intptr_t>(pDestination)) / 4;
                }

                int iSourceFrames = (pSourceLimit - pSource) / 4;
                if (iSourceFrames < iCount)
                {
                    iCount = static_cast<unsigned int>(pSourceLimit - pSource) >> 2;
                }
            }

            if (iLeftStep != 0 && 1 < iCount)
            {
                iLeftStep = iLeftStep / iCount;
            }

            if (iRightStep != 0 && 1 < iCount)
            {
                iRightStep = iRightStep / iCount;
            }

            for (int iFrame = 0; iFrame < iCount; ++iFrame)
            {
                *pDestination = *pDestination + ReadSample(pSource) * (iLeft >> MIX_VOLUME_SHIFT)
                                + ReadSample(pSource + 2) * (iRight >> MIX_VOLUME_SHIFT);
                ++pDestination;
                pSource += 4;
                iLeft = iLeft + iLeftStep;
                iRight = iRight + iRightStep;
            }
        }
        else
        {
            uFlags = uFlags | MIX_FLAG_STEREO_DESTINATION;
            if ((reinterpret_cast<uintptr_t>(pDestination + iCount * 2) & MIX_DESTINATION_ALIGNMENT) != 0)
            {
                iCount = iCount + 1;
                if (pDestinationLimit < pDestination + 2)
                {
                    iCount = static_cast<int>(reinterpret_cast<intptr_t>(pDestinationLimit) - reinterpret_cast<intptr_t>(pDestination)) / 8;
                }

                int iSourceFrames = (pSourceLimit - pSource) / 4;
                if (iSourceFrames < iCount)
                {
                    iCount = static_cast<unsigned int>(pSourceLimit - pSource) >> 2;
                }
            }

            if (iLeftStep != 0 && 1 < iCount)
            {
                iLeftStep = iLeftStep / iCount;
            }

            if (iRightStep != 0 && 1 < iCount)
            {
                iRightStep = iRightStep / iCount;
            }

            for (int iFrame = 0; iFrame < iCount; ++iFrame)
            {
                pDestination[0] = pDestination[0] + ReadSample(pSource) * (iLeft >> MIX_VOLUME_SHIFT);
                pDestination[1] = pDestination[1] + ReadSample(pSource + 2) * (iRight >> MIX_VOLUME_SHIFT);
                pDestination += 2;
                pSource += 4;
                iLeft = iLeft + iLeftStep;
                iRight = iRight + iRightStep;
            }
        }
    }

    *ppSource = const_cast<unsigned char*>(pSource);
    *ppDestination = pDestination;
    *piLeft = iLeft >> MIX_VOLUME_SHIFT;
    *piRight = iRight >> MIX_VOLUME_SHIFT;
    return uFlags;
}

extern "C" void* SS_execute_filter_chain(_SAMPLE* pSample, void* pSource, void* pDestination, unsigned int uBytes,
                                         int iChannel, void* pFinal)
{
    int iStages[SAMPLE_FILTER_STAGE_COUNT];
    int iActive = 0;
    for (int iStage = SAMPLE_STAGE_FILTER_FIRST; iStage <= SAMPLE_FILTER_STAGE_COUNT; ++iStage)
    {
        if (pSample->stages[iStage].filter.active != 0)
        {
            iStages[iActive++] = iStage;
        }
    }

    if (iActive == 0)
    {
        return NULL;
    }

    signed char* pMap = pSample->channelMap;
    int iIndex = 0;
    for (int iCandidate = 0; iCandidate < FILTER_CHANNEL_SEARCH_LENGTH; ++iCandidate)
    {
        int iMapped;
        if (pMap == NULL)
        {
            iMapped = iCandidate;
            if (pSample->channelCount <= iCandidate)
            {
                iMapped = -1;
            }
        }
        else
        {
            iMapped = pMap[iCandidate];
        }

        if (iMapped == iChannel)
        {
            iIndex = iCandidate;
            break;
        }
    }

    for (int iFilter = 0; iFilter < iActive; ++iFilter)
    {
        FilterStage& stage = pSample->stages[iStages[iFilter]].filter;
        void* pHandle = stage.handles[iChannel];
        if (pFinal != NULL && iFilter + 1 == iActive)
        {
            stage.instance->sampleProcess(pHandle, pSource, pFinal, uBytes >> 1, iIndex);
            return pFinal;
        }

        stage.instance->sampleProcess(pHandle, pSource, pDestination, uBytes >> 1, iIndex);
        pSource = pDestination;
    }

    return pSource;
}

extern "C" void SS_flush(_DIG_DRIVER* pDriver, int iBus)
{
    for (int iOutput = 0; iOutput < pDriver->outputCount; ++iOutput)
    {
        pDriver->processors[DRIVER_PROCESSOR_FLUSH].function.flush(pDriver->outputs[iOutput].buffer,
                                                                   pDriver->outputs[iOutput].bytes);
    }

    if (iBus <= FINAL_BUS)
    {
        if (iBus < FINAL_BUS)
        {
            DriverBus* pBus = &pDriver->buses[iBus];
            if (pBus->update != 0)
            {
                pDriver->processors[DRIVER_PROCESSOR_FLUSH].function.flush(pBus->buffer + pBus->writeOffset,
                                                                           pBus->chunkBytes);
                if (static_cast<unsigned int>((pDriver->sharedState - pBus->updateValue) * pDriver->fillPeriod)
                    > pBus->tail * BUS_TAIL_SCALE + BUS_TAIL_MARGIN)
                {
                    pBus->update = 0;
                }
            }
        }
        else
        {
            FLT_call_premix_processors(pDriver);
        }
    }
}

unsigned int SS_limiter(_MSS_BB* pBuffers, int iCount, int iSamples, int iPrevious)
{
    int iChunks = 1 - ((1 - iSamples) / LIMITER_CHUNK_FRAMES);
    int iOffset = 0;
    if (iChunks < 1)
    {
        iChunks = 1;
    }

    for (;;)
    {
        int iPeak = 0;
        for (int iBuffer = 0; iBuffer < iCount; ++iBuffer)
        {
            int* pFirst = reinterpret_cast<int*>(pBuffers[iBuffer].buffer) + iOffset * pBuffers[iBuffer].chans;
            int* pEnd = pFirst + pBuffers[iBuffer].chans * LIMITER_CHUNK_FRAMES;
            if (pFirst + iSamples * pBuffers[iBuffer].chans < pEnd)
            {
                pEnd = pFirst + iSamples * pBuffers[iBuffer].chans;
            }

            for (int* pValue = pFirst; pValue != pEnd; ++pValue)
            {
                int iValue = *pValue;
                if (iValue <= 0)
                {
                    iValue = -iValue;
                }

                if (iPeak < iValue)
                {
                    iPeak = iValue;
                }
            }
        }

        int iLevel = LIMITER_UNITY;
        if (iPeak > LIMITER_THRESHOLD)
        {
            iLevel = (((iPeak - LIMITER_THRESHOLD) >> LIMITER_PEAK_SHIFT) * LIMITER_SLOPE) / LIMITER_DIVISOR
                     + LIMITER_UNITY;
            if (iLevel < 0)
            {
                iLevel = 0;
            }
        }

        int iDifference = iPrevious - iLevel;
        if (iDifference < 0)
        {
            iDifference = -iDifference;
        }

        if (iDifference > LIMITER_SMOOTHING_LIMIT - 1)
        {
            iLevel = (static_cast<int>(iLevel << 14) >> 15) + (static_cast<int>(iPrevious << 14) >> 15);
        }

        iPrevious = iLevel;
        if (iPrevious != LIMITER_UNITY && 0 < iCount)
        {
            for (int iBuffer = 0; iBuffer < iCount; ++iBuffer)
            {
                int* pFirst = reinterpret_cast<int*>(pBuffers[iBuffer].buffer) + iOffset * pBuffers[iBuffer].chans;
                int* pEnd = pFirst + pBuffers[iBuffer].chans * LIMITER_CHUNK_FRAMES;
                if (pFirst + iSamples * pBuffers[iBuffer].chans < pEnd)
                {
                    pEnd = pFirst + iSamples * pBuffers[iBuffer].chans;
                }

                for (int* pValue = pFirst; pValue != pEnd; ++pValue)
                {
                    *pValue = (((*pValue >> LIMITER_PEAK_SHIFT) * iPrevious) >> LIMITER_GAIN_SHIFT)
                              << LIMITER_PEAK_SHIFT;
                }
            }
        }

        --iChunks;
        iSamples = iSamples - LIMITER_CHUNK_FRAMES;
        iOffset = iOffset + LIMITER_CHUNK_FRAMES;
        if (iChunks == 0)
        {
            return iPrevious;
        }
    }
}

extern "C" void SS_copy(_DIG_DRIVER* pDriver, int iBus, short* pDestination)
{
    int iReverbBus = iBus;
    if (pDriver->busSamples[0] == NULL)
    {
        iReverbBus = 0;
    }

    DriverBus* pBus = &pDriver->buses[0] + iReverbBus;
    if (pBus->update != 0)
    {
        pBus->writeOffset = pBus->writeOffset + pBus->chunkBytes;
        if (pBus->length <= pBus->writeOffset)
        {
            pBus->writeOffset = 0;
        }

        int iRead = pBus->readOffset;
        unsigned int uChunk = pBus->chunkBytes;
        unsigned int uAvailable = pBus->length - iRead;
        int* pInput = reinterpret_cast<int*>(pBus->buffer + iRead);
        unsigned int uWrapped;
        if (static_cast<int>(uChunk) < static_cast<int>(uAvailable))
        {
            uWrapped = 0;
            pBus->readOffset = iRead + uChunk;
            uAvailable = uChunk;
        }
        else
        {
            uWrapped = uChunk - uAvailable;
            pBus->readOffset = uWrapped;
        }

        int iFrame = 0;
        while (uAvailable != 0)
        {
            unsigned int uSamples = uAvailable >> 2;
            ReverbState reverb = pBus->reverb;
            if (uSamples != 0)
            {
                int iMono = iFrame << 2;
                int iStereo = iFrame << 3;
                iFrame = iFrame + uSamples;
                for (; uSamples != 0; --uSamples)
                {
                    float fInput = static_cast<float>(*pInput);
                    float fFiltered = fInput * reverb.lowPass[0];
                    float fOldState = reverb.state[0];
                    float fSum = fFiltered + reverb.state[0];
                    reverb.state[0] = fFiltered;
                    fFiltered = fSum + fOldState + reverb.state[1];
                    reverb.state[1] = fOldState;
                    float fFeed = -(reverb.lowPass[2] * reverb.state[3]
                                    - -(reverb.lowPass[1] * reverb.state[2] - fFiltered))
                                  + REVERB_DENORMAL_BIAS;
                    reverb.state[3] = reverb.state[2];
                    reverb.state[2] = fFeed;
                    float fFirst = *reverb.lines[0];
                    float fSecond = *reverb.lines[1];
                    float fThird = *reverb.lines[2];
                    float fFourth = *reverb.lines[3];
                    *reverb.lines[0] = reverb.coefficients[0] * fFirst + fFeed;
                    *reverb.lines[1] = reverb.coefficients[1] * *reverb.lines[1] + fFeed;
                    *reverb.lines[2] = reverb.coefficients[2] * *reverb.lines[2] + fFeed;
                    *reverb.lines[3] = reverb.coefficients[3] * *reverb.lines[3] + fFeed;
                    float fComb = fFirst + fSecond + fThird + fFourth;
                    ++pInput;
                    for (int iLine = 0; iLine < 4; ++iLine)
                    {
                        if (reverb.lines[iLine] + 1 < reverb.ends[iLine])
                        {
                            ++reverb.lines[iLine];
                        }
                        else
                        {
                            reverb.lines[iLine] = reverb.starts[iLine];
                        }
                    }

                    float fAllPassIn = *reverb.lines[4];
                    fComb = reverb.coefficients[4] * fAllPassIn + fComb;
                    *reverb.lines[4] = fComb;
                    float fAllPassOut = -(reverb.coefficients[4] * fComb - fAllPassIn);
                    ++reverb.lines[4];
                    if (reverb.ends[4] <= reverb.lines[4])
                    {
                        reverb.lines[4] = reverb.starts[4];
                    }

                    float fFinalIn = *reverb.lines[5];
                    float fLeft = reverb.coefficients[5] * fFinalIn + fAllPassOut * REVERB_LEFT_FEEDBACK;
                    *reverb.lines[5] = fLeft;
                    fLeft = reverb.coefficients[5] * fLeft - fFinalIn;
                    if (reverb.lines[5] + 1 < reverb.ends[5])
                    {
                        ++reverb.lines[5];
                    }
                    else
                    {
                        reverb.lines[5] = reverb.starts[5];
                    }

                    for (int iOutput = 0; iOutput < pDriver->outputCount; ++iOutput)
                    {
                        _MSS_BB& output = pDriver->outputs[iOutput];
                        int iLeftValue = static_cast<int>(-fLeft * pDriver->reverbSendA[output.speaker]);
                        if (output.chans == 2)
                        {
                            int* pStereo = reinterpret_cast<int*>(output.buffer + iStereo);
                            pStereo[0] = pStereo[0] + iLeftValue;
                            pStereo[1] = pStereo[1]
                                         + static_cast<int>(-(reverb.coefficients[5]
                                                                  * (reverb.coefficients[5] * fFinalIn
                                                                     + fAllPassOut * REVERB_RIGHT_FEEDBACK)
                                                              - fFinalIn)
                                                            * pDriver->reverbSendA[output.speaker + 1]);
                        }
                        else
                        {
                            int* pMono = reinterpret_cast<int*>(output.buffer + iMono);
                            *pMono = *pMono + iLeftValue;
                        }
                    }

                    iMono = iMono + 4;
                    iStereo = iStereo + 8;
                }
            }

            for (int iLine = 0; iLine < DRIVER_REVERB_POINTER_COUNT; ++iLine)
            {
                pBus->reverb.lines[iLine] = reverb.lines[iLine];
            }

            for (int iState = 0; iState < 4; ++iState)
            {
                pBus->reverb.state[iState] = reverb.state[iState];
            }

            if (uWrapped == 0)
            {
                break;
            }

            pInput = reinterpret_cast<int*>(pBus->buffer);
            uAvailable = uWrapped;
            uWrapped = 0;
        }
    }

    if (iBus < FINAL_BUS)
    {
        StartTimer(&pDriver->timers[TIMER_FINAL_COPY]);
        _SAMPLE* pSample = pDriver->busSamples[iBus];
        if (pSample->limiterEnabled != 0)
        {
            pSample->limiterLevel = SS_limiter(pDriver->outputs, pDriver->outputCount, pDriver->framesPerBuffer,
                                               pSample->limiterLevel);
        }

        int iOutputs = pDriver->outputCount;
        unsigned int uFrames = pDriver->framesPerBuffer;
        int iStride = pSample->channelCount;
        for (int iOutput = 0; iOutput < iOutputs; ++iOutput)
        {
            _MSS_BB& output = pDriver->outputs[iOutput];
            const int* pValue = reinterpret_cast<const int*>(output.buffer);
            short* pTarget = pDestination + output.speaker;
            if (output.chans == 2)
            {
                for (unsigned int uSample = 0; uSample < uFrames * 2; uSample += 2)
                {
                    pTarget[0] = ConvertOutputSample(pValue[uSample]);
                    pTarget[1] = ConvertOutputSample(pValue[uSample + 1]);
                    pTarget = pTarget + iStride;
                }
            }
            else
            {
                for (unsigned int uSample = 0; uSample < uFrames; ++uSample)
                {
                    *pTarget = ConvertOutputSample(pValue[uSample]);
                    pTarget = pTarget + iStride;
                }
            }
        }

        short* pScan = pDestination;
        short* pScanEnd = pDestination + uFrames * iStride;
        int iPeak = 0;
        for (; pScan != pScanEnd; ++pScan)
        {
            int iValue = *pScan;
            if (iValue <= 0)
            {
                iValue = -iValue;
            }

            if (iPeak < iValue)
            {
                iPeak = iValue;
            }
        }

        unsigned int uSources = pSample->compressorSources;
        pSample->peak = iPeak;
        for (int iSource = 0; iSource < DRIVER_BUS_COUNT && uSources != 0; ++iSource)
        {
            _SAMPLE* pSourceSample = pDriver->busSamples[iSource];
            if ((uSources & 1) != 0 && pSourceSample->compressorLink != 0
                && pSourceSample->busPeak < static_cast<unsigned int>(iPeak))
            {
                pSourceSample->busPeak = iPeak;
            }

            uSources = static_cast<int>(uSources) >> 1;
        }

        StopTimer(&pDriver->timers[TIMER_FINAL_COPY]);
        return;
    }

    if (pDriver->limiterEnabled != 0)
    {
        pDriver->limiterLevel = SS_limiter(pDriver->outputs, pDriver->outputCount, pDriver->framesPerBuffer,
                                           pDriver->limiterLevel);
    }

    StartTimer(&pDriver->timers[TIMER_FILTERS]);
    if (FLT_call_postmix_processors(pDriver, pDestination) != 0)
    {
        return;
    }

    StopTimer(&pDriver->timers[TIMER_FILTERS]);
    if (pDriver->speakerConfig != pDriver->hardwareChannels)
    {
        return;
    }

    StartTimer(&pDriver->timers[TIMER_FINAL_COPY]);
    if (pDriver->speakerConfig < 3)
    {
        pDriver->processors[DRIVER_PROCESSOR_COPY].function.copy(pDriver->outputs[0].buffer, pDriver->outputs[0].bytes,
                                                                 pDestination, pDriver->flags, 1);
    }
    else
    {
        pDriver->processors[DRIVER_PROCESSOR_MULTICHANNEL_COPY].function.multiChannelCopy(
            pDriver->outputs, pDriver->outputCount, pDestination, pDriver->flags, pDriver->framesPerBuffer,
            pDriver->hardwareChannels);
    }

    StopTimer(&pDriver->timers[TIMER_FINAL_COPY]);
}

extern "C" unsigned int SS_stream_to_buffer(_SAMPLE* pSample, short* pDestination, int iSize)
{
    _DIG_DRIVER* pDriver = pSample->driver;
    pSample->sampleEndCounter = 0;
    pSample->endCounter = 0;
    pSample->startCounter = 0;
    if (pDriver->sampleStatus[pSample->index] != SAMPLE_STATUS_PLAYING)
    {
        return 0;
    }

    if (pDriver->speakerConfig > 0 && pSample->is3D != 0)
    {
        float fPrevious = pSample->falloffLowPass;
        SS_calculate_3D_channel_levels(&pDriver->speakers, pDriver->speakerConfig, pSample, pSample->speakerLevelsA);
        if (pSample->falloffVolume != NO_FALLOFF_RESULT)
        {
            pSample->exclusion = pSample->falloffVolume;
        }

        if (fabs(static_cast<float>(fPrevious - pSample->falloffLowPass)) > RATE_FACTOR_EPSILON)
        {
            pSample->cacheState = -1;
        }
    }

    int iPosition = 0;
    int iLastResult = -1;
    if (pSample->playbackDelay != 0)
    {
        unsigned long long uDelay = static_cast<unsigned long long>(static_cast<unsigned int>(iSize))
                                    * static_cast<unsigned int>(pSample->playbackDelay);
        iPosition = static_cast<unsigned int>(uDelay / static_cast<unsigned int>(pDriver->fillPeriod)) & ALIGN_16_MASK;
        pSample->playbackDelay = 0;
    }

    bool bFinished = false;
    while (!bFinished && iSize > iPosition)
    {
        SampleBuffer* pBuffer = &pSample->buffers[pSample->currentBuffer];
        int iRemaining = pBuffer->size - pBuffer->position;
        if (pSample->loopState != 1 && pSample->loopEnd != SAMPLE_LOOP_TO_END
            && pSample->loopEnd < static_cast<unsigned int>(pBuffer->size))
        {
            iRemaining = pSample->loopEnd - pBuffer->position;
        }

        if (iRemaining > 0)
        {
            if (pSample->startOfBuffer != NULL)
            {
                ++pSample->startCounter;
            }

            iPosition = SS_merge(pSample, iRemaining, iPosition, pDestination, iSize);
            if ((iPosition == 0 && iLastResult == 0) || ((iLastResult = iPosition), iSize <= iPosition))
            {
                bFinished = true;
            }

            continue;
        }

        if (pSample->endOfBuffer != NULL)
        {
            ++pSample->endCounter;
        }

        bool bLoop = false;
        if (pBuffer->reset != 0)
        {
            if ((pSample->channelFlags & SAMPLE_FORMAT_ADPCM) == 0)
            {
                reset_asi(pSample, SAMPLE_LOOP_CURRENT, pBuffer->resetPosition);
                pSample->buffers[pSample->currentBuffer].reset = 0;
                bLoop = pSample->loopState != 1;
            }
            else
            {
                pSample->resetB = 0;
                pSample->resetA = 0;
                bLoop = pSample->loopState != 1;
            }
        }
        else
        {
            bLoop = pSample->loopState != 1;
        }

        if (bLoop)
        {
            SampleBuffer* pCurrent = &pSample->buffers[pSample->currentBuffer];
            if (pCurrent->size < pSample->loopStart)
            {
                pCurrent->position = pCurrent->size;
                bFinished = true;
                continue;
            }

            pCurrent->position = pSample->loopStart;
            if ((pSample->channelFlags & SAMPLE_FORMAT_ADPCM) != 0)
            {
                pSample->resetA = 0;
                pSample->resetB = 0;
            }

            reset_asi(pSample, SAMPLE_LOOP_CURRENT, pSample->buffers[pSample->currentBuffer].position);
            if (pSample->loopState != 0)
            {
                --pSample->loopState;
            }

            continue;
        }

        if (pSample->loopMode == LOOP_MODE_STREAMING)
        {
            int iNext = (pSample->currentBuffer + 1) % pSample->bufferCount;
            SampleBuffer* pNext = &pSample->buffers[iNext];
            if (pNext->done == 0)
            {
                if (pNext->size == 0 || pNext->position != 0)
                {
                    pSample->starved = 1;
                    bFinished = true;
                    continue;
                }

                pSample->currentBuffer = iNext;
                continue;
            }
        }

        pDriver->sampleStatus[pSample->index] = SAMPLE_STATUS_DONE;
        pSample->starved = 1;
        if (pSample->endOfSample != NULL)
        {
            ++pSample->sampleEndCounter;
        }

        bFinished = true;
    }

    StartTimer(&pDriver->timers[TIMER_START_CALLBACKS]);
    int iPending = pSample->startCounter--;
    while (iPending != 0)
    {
        pSample->startOfBuffer(pSample);
        iPending = pSample->startCounter--;
    }

    StopTimer(&pDriver->timers[TIMER_START_CALLBACKS]);
    StartTimer(&pDriver->timers[TIMER_END_BUFFER_CALLBACKS]);
    iPending = pSample->endCounter--;
    while (iPending != 0)
    {
        pSample->endOfBuffer(pSample);
        iPending = pSample->endCounter--;
    }

    StopTimer(&pDriver->timers[TIMER_END_BUFFER_CALLBACKS]);
    StartTimer(&pDriver->timers[TIMER_END_SAMPLE_CALLBACKS]);
    iPending = pSample->sampleEndCounter--;
    while (iPending != 0)
    {
        pSample->endOfSample(pSample);
        iPending = pSample->sampleEndCounter--;
    }

    StopTimer(&pDriver->timers[TIMER_END_SAMPLE_CALLBACKS]);
    return iPosition;
}

extern "C" void SS_fill(_DIG_DRIVER* pDriver, short* pDestination)
{
    unsigned long long uStart = rrGetTime();
    unsigned long long uProfileStart = 0;
    if (MSS_profile != NULL)
    {
        reinterpret_cast<ProfileTickFunc>(MSS_profile->pZoneEnter)(MSS_profile);
    }

    if (MSS_profile != NULL)
    {
        uProfileStart = OSGetSystemTime();
    }

    for (int iTimer = 0; iTimer < DRIVER_PROFILE_TIMER_COUNT; ++iTimer)
    {
        pDriver->timers[iTimer].high = 0;
        pDriver->timers[iTimer].low = 0;
    }

    for (int iSlot = 0; iSlot < DRIVER_PROFILE_SLOT_COUNT; ++iSlot)
    {
        pDriver->profileSamples[iSlot] = NULL;
        pDriver->profileCounts[iSlot] = 0;
    }

    if (pDriver->fillCallback != NULL)
    {
        pDriver->fillCallback(pDriver);
    }

    if (pDriver->pendingStarts != 0)
    {
        unsigned int uFrames = pDriver->fillBytes / (pDriver->hardwareChannels << 1);
        unsigned long long uFrameStart =
            static_cast<unsigned long long>(static_cast<unsigned int>(pDriver->sharedState)) * uFrames;
        unsigned long long uFrameEnd = uFrameStart + uFrames;
        for (int iIndex = 0; iIndex < pDriver->sampleCount; ++iIndex)
        {
            _SAMPLE* pSample = &pDriver->samples[iIndex];
            int iStatus = pDriver->sampleStatus[iIndex];
            if ((iStatus == SAMPLE_STATUS_DONE || iStatus == SAMPLE_STATUS_STOPPED)
                && (pSample->reverbBus == SAMPLE_NO_REVERB_BUS
                    || (pSample->phaseMask & (1 << (pDriver->phase & SAMPLE_PHASE_MASK))) != 0))
            {
                unsigned long long uTarget = (static_cast<unsigned long long>(pSample->startTimeHigh) << 32)
                                             | pSample->startTimeLow;
                if (uTarget != 0 && uTarget < uFrameEnd)
                {
                    unsigned int uDelta = 0;
                    if (uTarget >= uFrameStart)
                    {
                        uDelta = pSample->startTimeLow - static_cast<unsigned int>(uFrameStart);
                    }

                    pSample->playbackDelay = pSample->playbackDelay
                                             + static_cast<int>(
                                                 (static_cast<unsigned long long>(uDelta) * SAMPLE_START_DELAY_UNITS)
                                                 / static_cast<long long>(pDriver->outputRate));
                    --pDriver->pendingStarts;
                    unsigned char ucPhase = pSample->phaseMask;
                    pSample->startTimeHigh = 0;
                    pSample->startTimeLow = 0;
                    AIL_API_start_sample(pSample);
                    pSample->phaseMask = ucPhase;
                }
            }
        }
    }

    for (int iIndex = 0; iIndex < pDriver->sampleCount; ++iIndex)
    {
        _SAMPLE* pSample = &pDriver->samples[iIndex];
        DecoderStage& decoder = pSample->stages[SAMPLE_STAGE_DECODER].decoder;
        if (pDriver->sampleStatus[iIndex] == SAMPLE_STATUS_PLAYING && pSample->buffers[0].position == 0
            && pSample->loopMode == LOOP_MODE_ASI
            && (pSample->reverbBus == SAMPLE_NO_REVERB_BUS || pSample->fadeSamples != 0
                || (pSample->phaseMask & (1 << (pDriver->phase & SAMPLE_PHASE_MASK))) != 0)
            && decoder.active != 0)
        {
            decoder.process(decoder.stream, NULL, 0);
            SampleBuffer* pBuffer = &pSample->buffers[pSample->currentBuffer];
            if (pBuffer->position == pBuffer->size && decoder.outputReservoir != RIB_ENTRY_NOT_FOUND)
            {
                int iReservoir = -1;
                decoder.property(decoder.stream, decoder.outputReservoir, &iReservoir, 0, 0);
                if (iReservoir > 0)
                {
                    pBuffer->position = pBuffer->size - 1;
                    pSample->bufferRewind = 1;
                }
            }
        }
    }

    bool bHasBuses = pDriver->busSamples[0] != NULL;
    for (int iBus = 0; iBus < DRIVER_BUS_COUNT; ++iBus)
    {
        if (pDriver->busSamples[iBus] != NULL)
        {
            pDriver->busSamples[iBus]->busPeak = 0;
        }
    }

    int iActive = 0;
    int iCurrentBus = 0;
    if (!bHasBuses)
    {
        iCurrentBus = FINAL_BUS;
    }
    else
    {
        pDriver->busActive[0] = 0;
    }

    SS_flush(pDriver, 0);

    for (;;)
    {
        for (int iIndex = 0; iIndex < pDriver->sampleCount; ++iIndex)
        {
            _SAMPLE* pSample = &pDriver->samples[iIndex];
            if (pDriver->sampleStatus[iIndex] != SAMPLE_STATUS_PLAYING
                || !(pSample->reverbBus == SAMPLE_NO_REVERB_BUS || pSample->fadeSamples != 0
                     || (pSample->phaseMask & (1 << (pDriver->phase & SAMPLE_PHASE_MASK))) != 0))
            {
                continue;
            }

            if (bHasBuses)
            {
                if (pSample->reverbBus != iCurrentBus)
                {
                    continue;
                }

                if (iCurrentBus < DRIVER_BUS_COUNT)
                {
                    ++iActive;
                    ++pDriver->busActive[iCurrentBus];
                }
            }
            else
            {
                ++iActive;
            }

            unsigned int uBytes = pDriver->fillBytes;
            unsigned long long uPlayed = (static_cast<unsigned long long>(pSample->playedHigh) << 32)
                                         | pSample->playedLow;
            uPlayed = uPlayed + static_cast<long long>(static_cast<int>(uBytes));
            pSample->playedHigh = static_cast<unsigned int>(uPlayed >> 32);
            pSample->playedLow = static_cast<unsigned int>(uPlayed);
            unsigned int uPeriod = pDriver->fillPeriod;
            unsigned int uDelay = pSample->playbackDelay;
            if (uPeriod <= uDelay)
            {
                pSample->playbackDelay = uDelay - uPeriod;
                continue;
            }

            if (pSample->endMode == 2)
            {
                pSample->endMode = 0;
                AIL_API_end_sample(pSample);
                continue;
            }

            if (pSample->fadeSamples != 0)
            {
                pSample->fadePosition = pSample->fadePosition - static_cast<short>(uPeriod >> 8);
                if (pSample->fadePosition < 0)
                {
                    pSample->fadePosition = 0;
                }
            }

            if (iCurrentBus == FINAL_BUS && pSample->compressorLink != 0)
            {
                int iStage = pSample->compressorLink >> 16;
                FilterStage& filter = pSample->stages[iStage].filter;
                if (filter.active != 0 && filter.instance != NULL
                    && (filter.instance->flags & FILTER_SAMPLE_PROPERTY_FLAG) != 0)
                {
                    for (int iChannel = 0; iChannel < pSample->channelCount; ++iChannel)
                    {
                        filter.instance->sampleProperty(
                            filter.handles[iChannel], pSample->compressorLink & WORD_MASK_16, 0, &pSample->busPeak, 0);
                    }
                }
            }

            SS_stream_to_buffer(pSample, NULL, pSample->driver->outputs[0].bytes);
            if (pSample->fadeSamples != 0 && pSample->fadePosition == 0)
            {
                if (pSample->fadeEnds != 0)
                {
                    _STREAM* pStream = pSample->stream;
                    if (pStream != NULL)
                    {
                        AIL_pause_stream(pStream, 1);
                        pStream->paused = 1;
                        pSample->fadeEnds = 0;
                        pSample->fadeSamples = 0;
                        continue;
                    }

                    AIL_end_sample(pSample);
                }

                pSample->fadeSamples = 0;
                pSample->fadeEnds = 0;
            }
        }

        if (iCurrentBus < DRIVER_BUS_COUNT && pDriver->busActive[iCurrentBus] == 0
            && pDriver->buses[iCurrentBus].update == 0)
        {
            _SAMPLE* pBusSample = pDriver->busSamples[iCurrentBus];
            if (pDriver->sampleStatus[pBusSample->index] == SAMPLE_STATUS_PLAYING)
            {
                AIL_API_stop_sample(pBusSample);
            }
        }
        else if (pDriver->busSamples[0] == NULL || iCurrentBus > DRIVER_BUS_COUNT - 1)
        {
            SS_copy(pDriver, FINAL_BUS, pDestination);
        }
        else
        {
            SS_copy(pDriver, iCurrentBus, reinterpret_cast<short*>(pDriver->busBuffers[iCurrentBus]));
            _SAMPLE* pBusSample = pDriver->busSamples[iCurrentBus];
            AIL_API_load_sample_buffer(pBusSample, SAMPLE_BUFFER_CURRENT, pDriver->busBuffers[iCurrentBus],
                                       pDriver->framesPerBuffer * pBusSample->channelCount * 2);
            if (pDriver->busActive[iCurrentBus] != 0
                && pDriver->sampleStatus[pBusSample->index] != SAMPLE_STATUS_PLAYING)
            {
                AIL_API_resume_sample(pBusSample);
            }
        }

        do
        {
            ++iCurrentBus;
            if (iCurrentBus > DRIVER_BUS_COUNT - 1)
            {
                break;
            }
        } while (pDriver->busSamples[iCurrentBus] == NULL);

        if (iCurrentBus > FINAL_BUS)
        {
            break;
        }

        SS_flush(pDriver, bHasBuses ? iCurrentBus : 0);
        if (iCurrentBus < DRIVER_BUS_COUNT)
        {
            pDriver->busActive[iCurrentBus] = 0;
        }
    }

    pDriver->activeSamples = iActive;
    ++pDriver->sharedState;
    if (MSS_profile != NULL)
    {
        for (int iEntry = 0; iEntry < PROFILE_ENTRY_COUNT; ++iEntry)
        {
            const ProfileEntry& entry = PROFILE_ENTRIES[iEntry];
            const MixTimer& timer = pDriver->timers[entry.timer];
            int iCount = 0;
            bool bReport;
            if (entry.count == PROFILE_NO_COUNT)
            {
                bReport = timer.high != 0 || timer.low != 0;
            }
            else
            {
                iCount = pDriver->profileCounts[entry.count];
                bReport = iCount != 0;
            }

            if (bReport && MSS_profile != NULL)
            {
                reinterpret_cast<ProfileZoneFunc>(MSS_profile->pFunctionBC)(
                    MSS_profile, &uProfileStart, timer.high, timer.low, iCount, entry.flags, MILES_WAVEFILE_FILE,
                    entry.line, &PROFILE_SITES[entry.timer], entry.label);
            }
        }

        for (int iPlot = 0; iPlot < PROFILE_PLOT_COUNT && MSS_profile != NULL; ++iPlot)
        {
            MSS_profile->pPlot(
                MSS_profile, 0, PROFILE_PLOT_FLAGS, 0,
                static_cast<float>(static_cast<unsigned int>(pDriver->profileCounts[PROFILE_PLOTS[iPlot].count])),
                &PROFILE_PLOT_SITES[PROFILE_PLOTS[iPlot].count], PROFILE_PLOTS[iPlot].label);
        }

        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_WAVEFILE_FILE, PROFILE_LEAVE_LINE);
        }
    }

    unsigned long long uEnd = rrGetTime();
    unsigned long long uElapsed = uEnd - uStart;
    unsigned long long uTotal = (static_cast<unsigned long long>(pDriver->fillTimeHigh) << 32) | pDriver->fillTimeLow;
    uTotal = uTotal + uElapsed;
    pDriver->fillTimeHigh = static_cast<unsigned int>(uTotal >> 32);
    pDriver->fillTimeLow = static_cast<unsigned int>(uTotal);
}

namespace
{
const int MERGE_UNITY = 0x10000;
const int MERGE_MINIMUM_STEP = 0x20;
const unsigned int MERGE_SNAP_FRACTION = 0x20000000;
const unsigned int MERGE_FRACTION_FLAGS = 0xc0000000;
const unsigned int MERGE_FRACTION_MASK = 0xffff;
const unsigned int MERGE_FRACTION_INTEGER_MASK = 0x3fff;
const int MERGE_FRACTION_SHIFT = 16;
const unsigned int MERGE_FLAG_RESAMPLE = 0x10;
const unsigned int MERGE_FLAG_FILTERED = 4;
const unsigned int MERGE_FLAG_CONVERT = 0x40;
const int MERGE_SHIFT_PCM = 1;
const int MERGE_SHIFT_MONO = 2;
const int MERGE_SHIFT_STEREO = 3;
const int MERGE_CHUNK_BYTES = 0x800;
const int MERGE_HISTORY_BYTES = 16;
const int MERGE_SCRATCH_SHORTS = (MERGE_HISTORY_BYTES + MERGE_CHUNK_BYTES) / 2;
const int MERGE_HISTORY_SHORTS = MERGE_HISTORY_BYTES / 2;
const int MERGE_BYTE_BIAS = 0x80;
const unsigned int MERGE_ADPCM_MONO = 2;
const unsigned int MERGE_ADPCM_STEREO = 10;
const int MERGE_PROFILE_ADPCM = 0;
const int MERGE_PROFILE_DEINTERLEAVE = 1;
const int MERGE_PROFILE_MIX = 2;
const int MERGE_PROFILE_REVERB = 3;
const int MERGE_PROFILE_CONVERSION = 4;
const int MERGE_PROFILE_LOW_PASS = 5;
const int MERGE_PROFILE_FILTERS = 6;
const int MERGE_PROFILE_DECODER_BASE = 6;
const int MERGE_DECODER_TIMER_BASE = 11;
const int MERGE_CHANNEL_MASK_BITS = 8;

const unsigned int LOW_PASS_COUNT_MASK = 0xffff;
const unsigned int LOW_PASS_PHASE_MASK = 0xffff0000;
const int LOW_PASS_PHASE_SHIFT = 16;
const int LOW_PASS_BLEND_STEPS = 16;
const int LOW_PASS_PENDING_BLEND = 0xf;
const int LOW_PASS_PHASE_CROSSFADE = 1;
const int LOW_PASS_PHASE_FADE_IN = 2;
const int LOW_PASS_PHASE_FADE_OUT = 3;
const int LOW_PASS_MODE_STEADY = 0x10000;
const int LOW_PASS_MODE_FADE_OUT = 0x3000f;
const int LOW_PASS_NO_PENDING = 0x7fff;
const int LOW_PASS_PENDING_DISABLE = 0x30000;
const int LOW_PASS_BLEND_SHIFT = 4;
const int LOW_PASS_SAMPLE_MINIMUM = -0x8000;
const int LOW_PASS_SAMPLE_MAXIMUM = 0x7fff;
const float LOW_PASS_DISABLE_RATIO = 0.999f;
const float LOW_PASS_ENABLE_RATIO = 0.001f;
const float LOW_PASS_MAXIMUM_RATIO = 0.98f;
const float LOW_PASS_COEFFICIENT_SCALE = 8192.0f;
const float LOW_PASS_COEFFICIENT_UNIT = 0.0001220703125f;
const float LOW_PASS_SQRT_TWO = 1.4142135f;
const float LOW_PASS_FOUR = 4.0f;
const float LOW_PASS_TWO = 2.0f;
const float LOW_PASS_PI = 3.1415927f;

const float MERGE_UNITY_FLOAT = 65536.0f;
const float MERGE_VOLUME_SCALE = 2048.0f;
const float MERGE_VOLUME_UNIT = 0.00048828125f;
const float MERGE_THREE = 3.0f;
const int MERGE_VOLUME_MAXIMUM = 0x800;
const int MERGE_OUTPUT_DRC_CONFIG = 8;
const int MERGE_DRC_DOWNMIX_ROW = 5;

const int MIX_LEVEL_PAN_A = 0;
const int MIX_LEVEL_PAN_B = 1;
const int MIX_LEVEL_C = 2;
const int MIX_LEVEL_D = 3;
const int MIX_LEVEL_E = 4;
const int MIX_LEVEL_F = 5;
const int MIX_LEVEL_AVERAGE = 6;
const int MIX_LEVEL_COUNT = 7;
const int OUTPUT_LEVEL_COLUMNS = 10;

const int OUTPUT_LEVEL_SOURCES[SPEAKER_CONFIG_COUNT][OUTPUT_LEVEL_COLUMNS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 6, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 4, 5, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 2, 3, 4, 5, 0, 0, 0, 0}, {0, 1, 2, 3, 4, 5, 6, 0, 0, 0}, {0, 1, 2, 3, 4, 5, 4, 5, 0, 0},
    {0, 1, 2, 3, 4, 5, 6, 4, 5, 0}};

inline unsigned short EncodeSample(int iValue)
{
    return static_cast<unsigned short>((static_cast<unsigned short>(iValue) >> 8)
                                       | (static_cast<unsigned char>(iValue) << 8));
}

inline int ClampSample(int iValue)
{
    if (iValue < LOW_PASS_SAMPLE_MINIMUM)
    {
        return LOW_PASS_SAMPLE_MINIMUM;
    }

    if (iValue > LOW_PASS_SAMPLE_MAXIMUM)
    {
        return LOW_PASS_SAMPLE_MAXIMUM;
    }

    return iValue;
}
}

int SS_merge(_SAMPLE* pSample, int iAvailable, int iOffset, short* pDestination, int iSize)
{
    _DIG_DRIVER* pDriver = pSample->driver;
    unsigned char* pDestinationBytes = reinterpret_cast<unsigned char*>(pDestination);

    int iFilterActive = 0;
    if (pSample->filterCount > 0)
    {
        iFilterActive = 1;
    }

    if (pSample->lowPassActive != 0)
    {
        iFilterActive = 1;
    }

    unsigned int uFormat = pSample->channelFlags;
    int iSource16 = (uFormat & (SAMPLE_FORMAT_16_BITS | SAMPLE_FORMAT_ADPCM)) != 0
                    || pSample->stages[SAMPLE_STAGE_DECODER].decoder.active != 0;
    int iPerChannel = (uFormat & SAMPLE_FORMAT_MULTICHANNEL) != 0
                      || ((uFormat & SAMPLE_FORMAT_STEREO) != 0
                          && (pSample->is3D != 0 || iFilterActive != 0 || pSample->userLevels != 0)
                          && pDestination == NULL);

    unsigned int uFlags = 0;
    int iFrameBytes = 1;
    if (iPerChannel)
    {
        uFlags = MIX_FLAG_16_BITS;
        iFrameBytes = 2;
        if (iSource16)
        {
            iFrameBytes = pSample->channelCount << 1;
        }
    }
    else
    {
        if (pDestination == NULL && (pDriver->flags & DRIVER_FLAG_STEREO_OR_MULTICHANNEL) != 0
            && (uFormat & SAMPLE_FORMAT_STEREO) != 0 && iFilterActive == 0)
        {
            iFrameBytes = 2;
            uFlags = MIX_FLAG_STEREO_SOURCE | MIX_FLAG_STEREO_DESTINATION;
        }
        else if ((uFormat & SAMPLE_FORMAT_STEREO) != 0)
        {
            iFrameBytes = 2;
            uFlags |= MIX_FLAG_STEREO_SOURCE;
        }

        if (iSource16)
        {
            iFrameBytes <<= 1;
            uFlags |= MIX_FLAG_16_BITS;
        }
    }

    int iStep = MERGE_UNITY;
    unsigned int uFraction;
    if (pDestination != NULL)
    {
        pDriver->outputRate = static_cast<int>(static_cast<float>(pSample->playbackRate) * pSample->playbackRateFactor
                                               * pSample->levelC);
        uFraction = 0;
        pSample->resampleFraction = 0;
    }
    else
    {
        float fRate = pSample->playbackRateFactor * pSample->levelC;
        if (pSample->is3D != 0 && pSample->dopplerActive != 0)
        {
            fRate = fRate * pSample->dopplerScale;
        }

        iStep = static_cast<int>(static_cast<float>(pSample->playbackRate) * fRate * MERGE_UNITY_FLOAT
                                 / static_cast<float>(pDriver->outputRate));
        int iTolerance = pSample->resampleTolerance;
        if (iStep <= iTolerance + MERGE_UNITY && MERGE_UNITY - iTolerance <= iStep)
        {
            uFraction = MERGE_SNAP_FRACTION;
            pSample->resampleFraction = MERGE_SNAP_FRACTION;
            iStep = MERGE_UNITY;
        }
        else
        {
            uFlags |= MERGE_FLAG_RESAMPLE;
            if (pSample->resampleFilter != 0)
            {
                uFlags |= MERGE_FLAG_FILTERED;
            }

            uFraction = pSample->resampleFraction;
            if (iStep < MERGE_MINIMUM_STEP)
            {
                iStep = MERGE_MINIMUM_STEP;
            }
        }
    }

    int iDestBytes = iSize - iOffset;
    int iShift = MERGE_SHIFT_PCM;
    if (pDestination == NULL)
    {
        iShift = MERGE_SHIFT_MONO;
        if (pDriver->outputs[0].chans > 1)
        {
            iShift = MERGE_SHIFT_STEREO;
        }
    }

    unsigned int uFrames = static_cast<unsigned int>(iDestBytes >> iShift);
    unsigned int uRemainder = MERGE_UNITY - (uFraction & MERGE_FRACTION_MASK);
    if (pDestination == NULL && (uFlags & MERGE_FLAG_RESAMPLE) != 0 && iPerChannel)
    {
        unsigned long long uDividend = (static_cast<unsigned long long>(pDriver->framesPerBuffer)
                                        << MERGE_FRACTION_SHIFT)
                                       + uRemainder - (iStep >> 1);
        int iMaximumFrames = static_cast<int>(static_cast<long long>(uDividend) / iStep);
        if (static_cast<int>(uFrames) > iMaximumFrames)
        {
            iDestBytes = iMaximumFrames << iShift;
            uFrames = static_cast<unsigned int>(iDestBytes >> iShift);
        }
    }

    unsigned long long uSpan = static_cast<unsigned long long>(uFrames) * static_cast<unsigned int>(iStep)
                               + MERGE_FRACTION_MASK - uRemainder;
    int iSourceBytes = static_cast<int>(static_cast<unsigned int>(uSpan >> MERGE_FRACTION_SHIFT)) * iFrameBytes;
    if ((uFraction & MERGE_FRACTION_FLAGS) != 0)
    {
        iSourceBytes -= ((iStep >> MERGE_FRACTION_SHIFT)
                         - ((uFraction >> MERGE_FRACTION_SHIFT) & MERGE_FRACTION_INTEGER_MASK))
                        * iFrameBytes;
    }
    else
    {
        iSourceBytes += iFrameBytes;
    }

    if (iSourceBytes < 1)
    {
        iSourceBytes = iFrameBytes;
    }

    bool bHistory = false;
    const unsigned char* pLastFrame = NULL;
    unsigned short aScratch[MERGE_SCRATCH_SHORTS];
    unsigned char* pScratch = reinterpret_cast<unsigned char*>(aScratch + MERGE_HISTORY_SHORTS);
    int iChunkBytes = (MERGE_CHUNK_BYTES / iFrameBytes) * iFrameBytes;
    unsigned char* pSource;
    unsigned char* pSourceEnd;

    if (pSample->stages[SAMPLE_STAGE_DECODER].decoder.active == 0)
    {
        SampleBuffer* pBuffer = &pSample->buffers[pSample->currentBuffer];
        pSource = const_cast<unsigned char*>(pBuffer->data) + pBuffer->position;
        pSourceEnd = pSource + iAvailable;
        if ((pSample->channelFlags & SAMPLE_FORMAT_ADPCM) == 0)
        {
            if (iSourceBytes > 0
                || (iSourceBytes = iFrameBytes, (uFraction & MERGE_FRACTION_MASK) + iStep >= MERGE_UNITY))
            {
                if (iAvailable < iSourceBytes)
                {
                    iSourceBytes = iAvailable;
                }
            }

            pSourceEnd = pSource + iSourceBytes;
        }
        else
        {
            int iHistoryBytes = 0;
            if (*pSample->channelHeader != CHANNEL_HEADER_UNITY)
            {
                pScratch -= iFrameBytes;
                iHistoryBytes = iFrameBytes;
                short* pHistory = reinterpret_cast<short*>(pScratch);
                for (int iHistory = 0; iHistory < pSample->channelCount; ++iHistory)
                {
                    pHistory[iHistory] = static_cast<short>(pSample->channelHeader[iHistory]);
                }
            }

            int iDecodeBytes = iSourceBytes - iHistoryBytes;
            if (iDecodeBytes > 0)
            {
                unsigned char* pDecoded = pScratch + iHistoryBytes;
                MarkProfileSample(pSample, MERGE_PROFILE_ADPCM);
                StartTimer(&pDriver->timers[TIMER_ADPCM]);
                unsigned int uDecodeFlags = MERGE_ADPCM_MONO;
                if ((pSample->channelFlags & SAMPLE_FORMAT_STEREO) != 0)
                {
                    uDecodeFlags = MERGE_ADPCM_STEREO;
                }

                if (iDecodeBytes > iChunkBytes)
                {
                    iDecodeBytes = iChunkBytes;
                }

                unsigned char* pEncoded = pSource;
                pDriver->processors[DRIVER_PROCESSOR_ADPCM_DECODE].function.adpcmDecode(
                    reinterpret_cast<void**>(&pDecoded), reinterpret_cast<void**>(&pEncoded), iDecodeBytes, iAvailable,
                    uDecodeFlags, &pSample->adpcmBlockSize);
                StopTimer(&pDriver->timers[TIMER_ADPCM]);
                iSourceBytes = static_cast<int>(pDecoded - pScratch);
                pBuffer = &pSample->buffers[pSample->currentBuffer];
                pBuffer->position = static_cast<int>(pEncoded - pBuffer->data);
                pSourceEnd = pDecoded;
                pLastFrame = pScratch + (iSourceBytes - iFrameBytes);
                bHistory = true;
                pSource = pScratch;
            }
        }
    }
    else
    {
        int iHistoryBytes = 0;
        if (*pSample->channelHeader != CHANNEL_HEADER_UNITY)
        {
            pScratch -= iFrameBytes;
            iHistoryBytes = iFrameBytes;
            short* pHistory = reinterpret_cast<short*>(pScratch);
            for (int iHistory = 0; iHistory < pSample->channelCount; ++iHistory)
            {
                pHistory[iHistory] = static_cast<short>(pSample->channelHeader[iHistory]);
            }
        }

        int iDecodeBytes = iSourceBytes - iHistoryBytes;
        if (iDecodeBytes > 0)
        {
            int iType = pSample->stages[SAMPLE_STAGE_DECODER].decoder.active;
            MarkProfileSample(pSample, MERGE_PROFILE_DECODER_BASE + iType);
            StartTimer(&pDriver->timers[MERGE_DECODER_TIMER_BASE + iType]);
            if (iDecodeBytes > iChunkBytes)
            {
                iDecodeBytes = iChunkBytes;
            }

            iSourceBytes = pSample->stages[SAMPLE_STAGE_DECODER].decoder.process(
                pSample->stages[SAMPLE_STAGE_DECODER].decoder.stream, pScratch + iHistoryBytes, iDecodeBytes);
            StopTimer(&pDriver->timers[MERGE_DECODER_TIMER_BASE + iType]);
            iSourceBytes += iHistoryBytes;
        }

        SampleBuffer* pBuffer = &pSample->buffers[pSample->currentBuffer];
        if (iSourceBytes == 0)
        {
            if (pBuffer->reset != 0)
            {
                return 0;
            }
        }
        else if (pBuffer->position == pBuffer->size
                 && pSample->stages[SAMPLE_STAGE_DECODER].decoder.outputReservoir != RIB_ENTRY_NOT_FOUND)
        {
            int iReservoir = -1;
            pSample->stages[SAMPLE_STAGE_DECODER].decoder.property(
                pSample->stages[SAMPLE_STAGE_DECODER].decoder.stream,
                pSample->stages[SAMPLE_STAGE_DECODER].decoder.outputReservoir, &iReservoir, NULL, NULL);
            if (iReservoir > 0)
            {
                pBuffer->position = pBuffer->size - 1;
                pSample->bufferRewind = 1;
            }
        }

        pSourceEnd = pScratch + iSourceBytes;
        pLastFrame = pScratch + (iSourceBytes - iFrameBytes);
        bHistory = true;
        pSource = pScratch;
    }

    if (iSourceBytes < 1)
    {
        return 0;
    }

    if (iDestBytes < 1)
    {
        return iSize;
    }

    unsigned int uNextRemainder = MERGE_UNITY - (pSample->resampleFraction & MERGE_FRACTION_MASK);
    unsigned int uSourceFrames = iSourceBytes / iFrameBytes + 1;
    unsigned long long uSourceSpan = (static_cast<unsigned long long>(uSourceFrames) << MERGE_FRACTION_SHIFT)
                                     + uNextRemainder - (iStep >> 1);
    int iFramesForSource = static_cast<int>(static_cast<long long>(uSourceSpan) / iStep);
    int iDestinationLimit = (iFramesForSource + 1) << iShift;
    if (iDestinationLimit < iDestBytes)
    {
        if (iDestinationLimit != 0)
        {
            iDestBytes = iDestinationLimit;
        }
        else
        {
            iDestBytes = 1 << iShift;
        }
    }

    pSample->volume = ZERO;
    for (int iChannel = 0; iChannel < MERGE_CHANNEL_MASK_BITS; ++iChannel)
    {
        if ((pSample->cacheState & 1) != 0 && iChannel < pSample->channelCount)
        {
            ChannelState* pChannelState = &pSample->channelStates[iChannel];
            float fCutOff = pSample->falloffLowPass;
            float fLimit = pSample->rate;
            int iPlaybackRate = pSample->playbackRate;
            if (fCutOff != NO_FALLOFF_RESULT && fCutOff < fLimit)
            {
                fLimit = fCutOff;
            }

            fLimit = fLimit / static_cast<float>(iPlaybackRate / 2);
            if (ONE < fLimit)
            {
                fLimit = ONE;
            }

            if (pChannelState->lowPass < fLimit)
            {
                fLimit = pChannelState->lowPass;
            }

            unsigned int uMode = pChannelState->lowPassInfo.mode & LOW_PASS_COUNT_MASK;
            if (uMode == LOW_PASS_PENDING_BLEND)
            {
                int iPhase = pChannelState->lowPassInfo.mode >> LOW_PASS_PHASE_SHIFT;
                if (iPhase == LOW_PASS_PHASE_CROSSFADE)
                {
                    pChannelState->lowPassInfo.coefficientA >>= LOW_PASS_PHASE_SHIFT;
                    pChannelState->lowPassInfo.coefficientB >>= LOW_PASS_PHASE_SHIFT;
                    pChannelState->lowPassInfo.coefficientC >>= LOW_PASS_PHASE_SHIFT;
                    pChannelState->lowPassInfo.historyA >>= LOW_PASS_PHASE_SHIFT;
                    pChannelState->lowPassInfo.historyB >>= LOW_PASS_PHASE_SHIFT;
                    pChannelState->lowPassInfo.mode = LOW_PASS_MODE_STEADY;
                    pChannelState->appliedLowPass = NO_FALLOFF_RESULT;
                }
                else if (iPhase == LOW_PASS_PHASE_FADE_IN)
                {
                    pChannelState->lowPassInfo.mode = 0;
                    pChannelState->appliedLowPass = NO_FALLOFF_RESULT;
                    pChannelState->lowPassInfo.coefficientA = LOW_PASS_NO_PENDING;
                }

                uMode = 0;
            }

            if (fLimit < ZERO || LOW_PASS_DISABLE_RATIO <= fLimit)
            {
                pChannelState->appliedLowPass = NO_FALLOFF_RESULT;
                if (uMode == 0)
                {
                    if (pChannelState->lowPassInfo.mode != 0)
                    {
                        pChannelState->lowPassInfo.mode = LOW_PASS_MODE_FADE_OUT;
                    }
                }
                else
                {
                    pChannelState->limitB = LOW_PASS_PENDING_DISABLE;
                }
            }
            else
            {
                float fRatio = (fLimit * static_cast<float>(pSample->playbackRate) * pSample->playbackRateFactor)
                               / static_cast<float>(pDriver->outputRate);
                float fApplied = LOW_PASS_MAXIMUM_RATIO;
                if (fRatio <= LOW_PASS_MAXIMUM_RATIO)
                {
                    fApplied = fRatio;
                    if (fRatio - LEVEL_EPSILON < ZERO)
                    {
                        fApplied = ZERO;
                    }
                }

                if (fApplied != pChannelState->appliedLowPass)
                {
                    pChannelState->appliedLowPass = fApplied;
                    pSample->lowPassActive = 1;
                    if (fApplied < LOW_PASS_ENABLE_RATIO)
                    {
                        if (uMode == 0)
                        {
                            set_lp_info(&pChannelState->lowPassInfo, 0, 0, 0);
                        }
                        else
                        {
                            pChannelState->limitB = 0;
                            pChannelState->pendingCoefficients = 0;
                        }
                    }
                    else
                    {
                        float fTangent = ONE
                                         / static_cast<float>(tan(static_cast<double>(LOW_PASS_PI * fApplied * HALF)));
                        float fNumerator = LOW_PASS_COEFFICIENT_SCALE
                                           / (fTangent * fTangent + (LOW_PASS_SQRT_TWO * fTangent + ONE));
                        int iFirst = static_cast<int>(fNumerator);
                        if (fNumerator != ZERO)
                        {
                            float fScale = static_cast<float>(iFirst) * LOW_PASS_COEFFICIENT_UNIT;
                            float fRoot = static_cast<float>(
                                sqrt(static_cast<double>(LOW_PASS_FOUR / fScale - LOW_PASS_TWO)));
                            float fOffset = (fRoot - LOW_PASS_SQRT_TWO) * HALF;
                            float fComplement = ONE - fOffset * fOffset;
                            int iSecond = static_cast<int>((fComplement + fComplement) * fScale
                                                           * LOW_PASS_COEFFICIENT_SCALE);
                            int iThird = static_cast<int>((-(LOW_PASS_SQRT_TWO * fOffset - ONE) + fOffset * fOffset)
                                                          * fScale * LOW_PASS_COEFFICIENT_SCALE);
                            if (uMode != 0)
                            {
                                pChannelState->limitB = iFirst;
                                pChannelState->pendingCoefficients = (iThird << LOW_PASS_PHASE_SHIFT)
                                                                     | (iSecond & LOW_PASS_COUNT_MASK);
                            }
                            else if (iFirst != static_cast<short>(pChannelState->lowPassInfo.coefficientA))
                            {
                                set_lp_info(&pChannelState->lowPassInfo, iFirst, iSecond, iThird);
                            }
                        }
                        else if (uMode == 0)
                        {
                            set_lp_info(&pChannelState->lowPassInfo, 0, 0, 0);
                        }
                        else
                        {
                            pChannelState->limitB = 0;
                            pChannelState->pendingCoefficients = 0;
                        }
                    }
                }
            }
        }


        pSample->cacheState >>= 1;
    }

    pSample->cacheState = 0;

    float fFade = ONE;
    if (pSample->fadeSamples != 0)
    {
        fFade = static_cast<float>(pSample->fadePosition) / static_cast<float>(pSample->fadeSamples);
        if (fFade < ZERO)
        {
            fFade = fFade + ONE;
        }
    }

    int iReverbBus = pSample->reverbBus;
    float fReverbSecond = pSample->reverbSecond;
    unsigned char* pReverbDestination = NULL;
    int iReverbLevel = 0;
    float fBusSend;
    if (iReverbBus < DRIVER_BUS_COUNT)
    {
        _SAMPLE* pBusSample = pDriver->busSamples[iReverbBus];
        if (pBusSample != NULL && pBusSample->reverbDirty != 0)
        {
            fReverbSecond = pBusSample->reverbSecond;
        }

        fBusSend = pDriver->buses[iReverbBus].sendLevel;
    }
    else if (iReverbBus == SAMPLE_NO_REVERB_BUS)
    {
        fBusSend = ONE;
    }
    else
    {
        fBusSend = pDriver->buses[iReverbBus].sendLevel;
    }

    float fWet = pDriver->masterVolume * fBusSend * pSample->volumeCurve * fReverbSecond * pSample->levelD
                 * pSample->levelB * pSample->levelA * fFade * (ONE - pSample->exclusion);
    if (pSample->automaticWetAttenuation != 0 && pSample->falloffPointCounts[1] == 0)
    {
        fWet = fWet * pSample->distanceVolume;
    }

    if (pSample->volume < fWet)
    {
        pSample->volume = fWet;
    }

    if (pDestination == NULL && !(fWet < LEVEL_EPSILON) && iReverbBus < DRIVER_BUS_COUNT)
    {
        DriverBus* pBus = &pDriver->buses[iReverbBus];
        if (pBus->update != 0)
        {
            iReverbLevel = static_cast<int>(MERGE_VOLUME_SCALE * fWet + HALF);
            if (iReverbLevel >= MERGE_VOLUME_MAXIMUM)
            {
                iReverbLevel = MERGE_VOLUME_MAXIMUM;
            }

            pReverbDestination = pBus->buffer + pBus->writeOffset;
            pBus->updateValue = pDriver->sharedState;
        }
    }

    float fReturn = ONE;
    if (iReverbBus != SAMPLE_NO_REVERB_BUS)
    {
        fReturn = pDriver->buses[iReverbBus].returnGain;
    }

    float fDry = pDriver->masterVolume * fReturn * pSample->levelD * pSample->reverbFirst * (ONE - pSample->obstruction)
                 * pSample->levelA * fFade;
    float aMixLevels[MIX_LEVEL_COUNT];
    aMixLevels[MIX_LEVEL_PAN_A] = pSample->panLevelA * fDry;
    aMixLevels[MIX_LEVEL_PAN_B] = pSample->panLevelB * fDry;
    int iSpeakerConfig = pDriver->speakerConfig;
    if (iSpeakerConfig > 2)
    {
        aMixLevels[MIX_LEVEL_E] = pSample->mixLevel0 * fDry;
        aMixLevels[MIX_LEVEL_F] = pSample->mixLevel1 * fDry;
        aMixLevels[MIX_LEVEL_C] = pSample->mixLevel2 * fDry;
        aMixLevels[MIX_LEVEL_D] = pSample->mixLevel3 * fDry;
        aMixLevels[MIX_LEVEL_AVERAGE] = (aMixLevels[MIX_LEVEL_E] + aMixLevels[MIX_LEVEL_F]) * HALF;
    }

    unsigned int uInitialFraction = pSample->resampleFraction;
    int iSourceFrames = iSourceBytes / iFrameBytes;
    int iDummyState = 0;
    int iMixedOffset = 0;
    int iEndOffset = 0;
    int* piLeft = pSample->channelPositionA;
    int* piRight = pSample->channelPositionB;
    unsigned int uOutputFraction = 0;
    int iChannel = 0;
    unsigned char* pConsumed = NULL;
    unsigned char* pInterleaved = pSource;
    unsigned char* pBlockStart = pSource;
    int iStride = iFrameBytes;
    int iConsumedBytes = iSourceBytes;

    for (;;)
    {
        if (iPerChannel)
        {
            pSource = pDriver->interleaveBuffer;
            iStride = pSample->channelCount;
            MarkProfileSample(pSample, MERGE_PROFILE_DEINTERLEAVE);
            StartTimer(&pDriver->timers[TIMER_DEINTERLEAVE]);
            unsigned short* pPacked = reinterpret_cast<unsigned short*>(pSource);
            if (iSource16)
            {
                const unsigned short* pFrame = reinterpret_cast<const unsigned short*>(pInterleaved) + iChannel;
                for (int iFrame = 0; iFrame < iSourceFrames; ++iFrame)
                {
                    *pPacked++ = *pFrame;
                    pFrame += iStride;
                }
            }
            else
            {
                const unsigned char* pFrame = pInterleaved + iChannel;
                for (int iFrame = 0; iFrame < iSourceFrames; ++iFrame)
                {
                    *pPacked++ = static_cast<unsigned short>((*pFrame + MERGE_BYTE_BIAS) & BYTE_MASK_8);
                    pFrame += iStride;
                }
            }

            pSourceEnd = reinterpret_cast<unsigned char*>(pPacked);
            StopTimer(&pDriver->timers[TIMER_DEINTERLEAVE]);
            uFlags = (uFlags & ~MIX_FLAG_STEREO_SOURCE) | MIX_FLAG_16_BITS;
            iStride = 2;
        }

        pBlockStart = pSource;
        bool bSilent = false;
        if (pDestination == NULL)
        {
            int iOutputCount = pDriver->outputCount;
            int iSilent = 0;
            const int* pSources = OUTPUT_LEVEL_SOURCES[iSpeakerConfig];
            int iRightChannel = iChannel;
            if ((uFlags & MIX_FLAG_STEREO_SOURCE) != 0)
            {
                iRightChannel = iChannel + 1;
            }

            for (int iOutput = 0; iOutput < iOutputCount; ++iOutput)
            {
                int iLeftRow = iOutput * 2;
                int iRightRow = iLeftRow;
                if (pDriver->outputs[iOutput].chans > 1)
                {
                    iRightRow = iLeftRow + 1;
                }

                float fRight = aMixLevels[pSources[iRightRow]] * pDriver->reverbSendB[iRightRow];
                float fLeft = aMixLevels[pSources[iLeftRow]] * pDriver->reverbSendB[iLeftRow];
                if (pSample->reverbBus == SAMPLE_NO_REVERB_BUS)
                {
                    fRight = pSample->levelA;
                    fLeft = fRight;
                }

                pSample->levelsC[iLeftRow][iChannel] = pSample->levelsB[iLeftRow][iChannel];
                pSample->levelsC[iLeftRow + 1][iChannel] = pSample->levelsB[iLeftRow + 1][iChannel];
                float fLevelLeft;
                float fLevelRight;
                if (pSample->is3D == 0)
                {
                    fLevelRight = pSample->levels[iRightRow][iRightChannel];
                    fLevelLeft = pSample->levels[iLeftRow][iChannel];
                }
                else
                {
                    fLevelLeft = pSample->speakerLevelsA[iLeftRow];
                    fLevelRight = pSample->speakerLevelsA[iRightRow];
                }

                if (pSample->endMode != 0)
                {
                    fLevelRight = ZERO;
                    fLevelLeft = ZERO;
                }

                bool bDrcOutput = pSample->outputToDRC && pDriver->speakerConfig == MERGE_OUTPUT_DRC_CONFIG;
                if (bDrcOutput && iLeftRow <= MERGE_DRC_DOWNMIX_ROW)
                {
                    pSample->levelsB[iLeftRow][iChannel] = static_cast<int>(
                        MERGE_VOLUME_SCALE * fLeft * pSample->speakerScales[iLeftRow] * ZERO + HALF);
                    pSample->levelsB[iLeftRow + 1][iChannel] = static_cast<int>(
                        MERGE_VOLUME_SCALE * fRight * pSample->speakerScales[iRightRow] * ZERO + HALF);
                }
                else
                {
                    if (bDrcOutput)
                    {
                        fLevelRight = ONE;
                        fLevelLeft = ONE;
                        if (pSample->is3D != 0)
                        {
                            const float* pLevels = pSample->speakerLevelsA;
                            fLevelLeft = (pLevels[0] + pLevels[4] + pLevels[2] * HALF) / MERGE_THREE;
                            fLevelRight = (pLevels[1] + pLevels[5] + pLevels[2] * HALF) / MERGE_THREE;
                        }
                    }

                    pSample->levelsB[iLeftRow][iChannel] = static_cast<int>(
                        MERGE_VOLUME_SCALE * fLeft * pSample->speakerScales[iLeftRow] * fLevelLeft + HALF);
                    pSample->levelsB[iLeftRow + 1][iChannel] = static_cast<int>(
                        MERGE_VOLUME_SCALE * fRight * pSample->speakerScales[iRightRow] * fLevelRight + HALF);
                }

                if (pSample->levelsB[iLeftRow][iChannel] >= MERGE_VOLUME_MAXIMUM)
                {
                    pSample->levelsB[iLeftRow][iChannel] = MERGE_VOLUME_MAXIMUM;
                }

                if (pSample->levelsB[iLeftRow + 1][iChannel] >= MERGE_VOLUME_MAXIMUM)
                {
                    pSample->levelsB[iLeftRow + 1][iChannel] = MERGE_VOLUME_MAXIMUM;
                }

                if (pSample->levelsB[iLeftRow][iChannel] == 0 && pSample->levelsB[iLeftRow + 1][iChannel] == 0
                    && pSample->levelsC[iLeftRow][iChannel] == 0 && pSample->levelsC[iLeftRow + 1][iChannel] == 0
                    && pSample->reverbSecond * pDriver->buses[pSample->reverbBus].sendLevel * pSample->levelD
                           < LEVEL_EPSILON)
                {
                    ++iSilent;
                }
            }

            bSilent = iSilent == iOutputCount;
            const LOWPASS_INFO& silentInfo = pSample->channelStates[iChannel].lowPassInfo;
            if (silentInfo.mode == LOW_PASS_MODE_STEADY && silentInfo.coefficientA == 0)
            {
                bSilent = true;
            }
        }

        if (bSilent)
        {
            *piLeft = 0;
            *piRight = 0;
            iEndOffset = iOffset + iDestBytes;
            *pSample->channelHeader = CHANNEL_HEADER_UNITY;
            pSample->resampleFraction = 0;
            pSource = pSourceEnd;
        }
        else
        {
            if (pSample->resampleFraction == MERGE_SNAP_FRACTION && (uFlags & MERGE_FLAG_FILTERED) != 0)
            {
                pSample->resampleFraction = 0;
                if ((uFlags & MIX_FLAG_16_BITS) == 0)
                {
                    *piLeft = pSource[0] - MERGE_BYTE_BIAS;
                    *piRight = pSource[0] - MERGE_BYTE_BIAS;
                    if ((uFlags & MIX_FLAG_STEREO_SOURCE) != 0)
                    {
                        *piRight = pSource[1] - MERGE_BYTE_BIAS;
                    }
                }
                else
                {
                    *piLeft = ReadSample(pSource);
                    *piRight = *piLeft;
                    if ((uFlags & MIX_FLAG_STEREO_SOURCE) != 0)
                    {
                        *piRight = ReadSample(pSource + 2);
                    }
                }
            }

            if (iFilterActive != 0 || pDestination == NULL)
            {
                unsigned int uStereo = uFlags & MIX_FLAG_STEREO_SOURCE;
                int iFrames = iDestBytes >> iShift;
                unsigned char* pData;
                unsigned int uBytes;
                if ((uFlags & MIX_FLAG_16_BITS) == 0 || (uFlags & MERGE_FLAG_RESAMPLE) != 0
                    || (uStereo != 0 && (iFilterActive != 0)))
                {
                    pData = pDriver->mix.buffer;
                    int iConvertBytes = iFrames << 1;
                    uStereo = uFlags & MIX_FLAG_STEREO_DESTINATION;
                    if (uStereo != 0)
                    {
                        iConvertBytes = iFrames << 2;
                    }

                    MarkProfileSample(pSample, MERGE_PROFILE_CONVERSION);
                    StartTimer(&pDriver->timers[TIMER_CONVERSION]);
                    unsigned char* pConverted = pData;
                    pSample->stages[SAMPLE_STAGE_MERGE].merge.merge(
                        &pSource, &pSample->resampleFraction, pSourceEnd, reinterpret_cast<void**>(&pConverted),
                        pData + iConvertBytes, piLeft, piRight, iStep, 0, 0, uFlags | MERGE_FLAG_CONVERT);
                    StopTimer(&pDriver->timers[TIMER_CONVERSION]);
                    uBytes = static_cast<unsigned int>(pConverted - pData);
                }
                else
                {
                    int iFrameLimit = iFrames * iStride;
                    uBytes = static_cast<unsigned int>(pSourceEnd - pSource);
                    pData = pBlockStart;
                    if (iFrameLimit < static_cast<int>(uBytes))
                    {
                        pSource += iFrameLimit;
                        uBytes = iFrameLimit;
                    }
                    else
                    {
                        pSource = pSourceEnd;
                    }
                }

                unsigned char* pFiltered = pData;
                ChannelState* pChannelState = &pSample->channelStates[iChannel];
                if (pChannelState->lowPassInfo.mode != 0)
                {
                    if (pDestination == NULL || pSample->filterCount != 0)
                    {
                        pFiltered = pDriver->mix.buffer;
                    }
                    else
                    {
                        pFiltered = pDestinationBytes + iOffset;
                    }

                    MarkProfileSample(pSample, MERGE_PROFILE_LOW_PASS);
                    StartTimer(&pDriver->timers[TIMER_LOW_PASS]);
                    ChannelState sState = *pChannelState;
                    int iPhase = sState.lowPassInfo.mode >> LOW_PASS_PHASE_SHIFT;
                    unsigned int uSamples = uBytes >> 1;
                    if (uSamples != 0)
                    {
                        const unsigned char* pInput = pData;
                        unsigned short* pOutput = reinterpret_cast<unsigned short*>(pFiltered);
                        bool bDone = false;
                        while (!bDone)
                        {
                            int iBlend = sState.lowPassInfo.mode & LOW_PASS_COUNT_MASK;
                            if (iBlend != 0)
                            {
                                int iNewWeight = LOW_PASS_BLEND_STEPS - iBlend;
                                for (;;)
                                {
                                    LOWPASS_INFO& info = sState.lowPassInfo;
                                    int iPreviousSample = info.sampleA;
                                    int iPreviousHistory = info.historyA;
                                    int iSample = ReadSample(pInput);
                                    int iSum = iSample + info.sampleA * 2 + info.sampleB;
                                    int iNew =
                                        ((iSum * static_cast<short>(info.coefficientA)
                                          - static_cast<short>(info.coefficientB) * static_cast<short>(info.historyA))
                                         - static_cast<short>(info.coefficientC) * static_cast<short>(info.historyB))
                                        >> LOW_PASS_COEFFICIENT_SHIFT;
                                    int iOld;
                                    if (iPhase < LOW_PASS_PHASE_FADE_IN)
                                    {
                                        iOld = ((iSum * (info.coefficientA >> LOW_PASS_PHASE_SHIFT)
                                                 - (info.coefficientB >> LOW_PASS_PHASE_SHIFT)
                                                       * (info.historyA >> LOW_PASS_PHASE_SHIFT))
                                                - (info.coefficientC >> LOW_PASS_PHASE_SHIFT)
                                                      * (info.historyB >> LOW_PASS_PHASE_SHIFT))
                                               >> LOW_PASS_COEFFICIENT_SHIFT;
                                        iOld = ClampSample(iOld);
                                    }
                                    else
                                    {
                                        iOld = iSample;
                                    }

                                    iNew = ClampSample(iNew);
                                    info.sampleB = iPreviousSample;
                                    info.historyB = iPreviousHistory;
                                    info.sampleA = iSample;
                                    info.historyA = static_cast<int>(
                                        (static_cast<unsigned int>(iOld) << LOW_PASS_PHASE_SHIFT)
                                        | (static_cast<unsigned int>(iNew) & LOW_PASS_COUNT_MASK));
                                    int iBlended;
                                    if (iPhase == LOW_PASS_PHASE_FADE_OUT)
                                    {
                                        iBlended = (iOld * iNewWeight + iNew * iBlend) >> LOW_PASS_BLEND_SHIFT;
                                    }
                                    else
                                    {
                                        iBlended = (iNew * iNewWeight + iOld * iBlend) >> LOW_PASS_BLEND_SHIFT;
                                    }

                                    *pOutput = EncodeSample(iBlended);
                                    --uSamples;
                                    ++pOutput;
                                    pInput += 2;
                                    if (iBlend == 1)
                                    {
                                        break;
                                    }

                                    ++iNewWeight;
                                    --iBlend;
                                    if (uSamples == 0)
                                    {
                                        info.mode = (info.mode & LOW_PASS_PHASE_MASK) | iBlend;
                                        bDone = true;
                                        break;
                                    }
                                }

                                if (bDone)
                                {
                                    break;
                                }

                                LOWPASS_INFO& finished = sState.lowPassInfo;
                                if (iPhase == LOW_PASS_PHASE_FADE_OUT)
                                {
                                    finished.coefficientA = LOW_PASS_NO_PENDING;
                                    finished.mode = 0;
                                    finished.historyA = 0;
                                    sState.appliedLowPass = NO_FALLOFF_RESULT;
                                    finished.historyB = 0;
                                    finished.sampleA = 0;
                                    finished.sampleB = 0;
                                }
                                else
                                {
                                    finished.mode = LOW_PASS_MODE_STEADY;
                                }

                                if (sState.limitB != LOW_PASS_NO_PENDING)
                                {
                                    if (sState.limitB == LOW_PASS_PENDING_DISABLE)
                                    {
                                        finished.mode = LOW_PASS_MODE_FADE_OUT;
                                    }
                                    else
                                    {
                                        set_lp_info(&finished, sState.limitB,
                                                    static_cast<short>(sState.pendingCoefficients),
                                                    sState.pendingCoefficients >> LOW_PASS_PHASE_SHIFT);
                                    }

                                    sState.limitB = LOW_PASS_NO_PENDING;
                                    if (uSamples == 0)
                                    {
                                        finished.mode = finished.mode & LOW_PASS_PHASE_MASK;
                                        break;
                                    }

                                    iPhase = finished.mode >> LOW_PASS_PHASE_SHIFT;
                                    continue;
                                }

                                if (uSamples == 0)
                                {
                                    break;
                                }
                            }

                            if (iPhase == LOW_PASS_PHASE_FADE_OUT)
                            {
                                for (; uSamples != 0; --uSamples)
                                {
                                    *pOutput = *reinterpret_cast<const unsigned short*>(pInput);
                                    ++pOutput;
                                    pInput += 2;
                                }
                            }
                            else
                            {
                                LOWPASS_INFO& info = sState.lowPassInfo;
                                info.coefficientA = static_cast<short>(info.coefficientA);
                                info.coefficientB = static_cast<short>(info.coefficientB);
                                info.coefficientC = static_cast<short>(info.coefficientC);
                                info.historyA = static_cast<short>(info.historyA);
                                info.historyB = static_cast<short>(info.historyB);
                                for (; uSamples != 0; --uSamples)
                                {
                                    int iSample = ReadSample(pInput);
                                    int iPreviousSample = info.sampleA;
                                    int iFiltered = ClampSample(
                                        ((iSample + info.sampleA * 2 + info.sampleB) * info.coefficientA
                                         - info.coefficientB * info.historyA - info.coefficientC * info.historyB)
                                        >> LOW_PASS_COEFFICIENT_SHIFT);
                                    info.sampleB = iPreviousSample;
                                    info.sampleA = iSample;
                                    info.historyB = info.historyA;
                                    info.historyA = iFiltered;
                                    *pOutput = EncodeSample(iFiltered);
                                    ++pOutput;
                                    pInput += 2;
                                }
                            }

                            bDone = true;
                        }

                        *pChannelState = sState;
                    }

                    if (iPhase == LOW_PASS_PHASE_FADE_OUT)
                    {
                        pSample->lowPassActive = 0;
                        for (int iActive = 0; iActive < pSample->channelCount; ++iActive)
                        {
                            if (pSample->channelStates[iActive].lowPassInfo.mode != 0)
                            {
                                pSample->lowPassActive = 1;
                            }
                        }
                    }

                    StopTimer(&pDriver->timers[TIMER_LOW_PASS]);
                }

                if (pSample->filterCount > 0)
                {
                    MarkProfileSample(pSample, MERGE_PROFILE_FILTERS);
                    StartTimer(&pDriver->timers[TIMER_FILTERS]);
                    unsigned char* pFinal = NULL;
                    if (pDestination != NULL)
                    {
                        pFinal = pDestinationBytes + iOffset;
                    }

                    pFiltered = static_cast<unsigned char*>(
                        SS_execute_filter_chain(pSample, pFiltered, pDriver->mix.buffer, uBytes, iChannel, pFinal));
                    StopTimer(&pDriver->timers[TIMER_FILTERS]);
                }

                if (pDestination == NULL)
                {
                    int iOutputCount = pDriver->outputCount;
                    unsigned char* pFilteredEnd = pFiltered + uBytes;
                    unsigned char* pMixSource = pFiltered;
                    if (pReverbDestination != NULL && iReverbLevel != 0)
                    {
                        int iReverbLeft = iReverbLevel;
                        int iReverbRight = iReverbLevel;
                        unsigned char* pReverbCursor;
                        unsigned char* pReverbEnd = pReverbDestination + pDriver->buses[0].chunkBytes;
                        if (pDriver->outputs[0].chans < 2)
                        {
                            pReverbCursor = pReverbDestination + iOffset;
                        }
                        else
                        {
                            pReverbCursor = pReverbDestination + (iOffset >> 1);
                        }

                        unsigned int uReverbFlags = slow_ramp_mix(
                            uStereo != 0, 0, reinterpret_cast<void**>(&pMixSource), pFilteredEnd,
                            reinterpret_cast<void**>(&pReverbCursor), pReverbEnd, &iReverbLeft, &iReverbRight, 0, 0, 0);
                        MarkProfileSample(pSample, MERGE_PROFILE_REVERB);
                        StartTimer(&pDriver->timers[TIMER_REVERB]);
                        unsigned int uReverbFraction = 0;
                        int iReverbState = 0;
                        pSample->stages[SAMPLE_STAGE_MERGE].merge.merge(
                            &pMixSource, &uReverbFraction, pFilteredEnd, reinterpret_cast<void**>(&pReverbCursor),
                            pReverbEnd, &iReverbState, &iReverbState, MERGE_UNITY, iReverbLevel, iReverbLevel,
                            uReverbFlags);
                        StopTimer(&pDriver->timers[TIMER_REVERB]);
                    }

                    int iSourceShift = 1;
                    if (uStereo != 0)
                    {
                        iSourceShift = 2;
                    }

                    int iFirstOffset = 0;
                    for (int iOutput = 0; iOutput < iOutputCount; ++iOutput)
                    {
                        _MSS_BB& output = pDriver->outputs[iOutput];
                        int iLeftRow = iOutput * 2;
                        unsigned char* pCursor;
                        if (output.chans < pDriver->outputs[0].chans)
                        {
                            pCursor = output.buffer + (iOffset >> 1);
                        }
                        else
                        {
                            pCursor = output.buffer + iOffset;
                        }

                        pMixSource = pFiltered;
                        unsigned char* pEnd = output.buffer + output.bytes;
                        int iLeftTarget = pSample->levelsB[iLeftRow][iChannel];
                        int iLeftCurrent = pSample->levelsC[iLeftRow][iChannel];
                        unsigned int uDestinationStereo = output.chans >= 2 ? 1 : 0;
                        int iDestinationShift = uDestinationStereo + MERGE_SHIFT_MONO;
                        int iRightCurrent = pSample->levelsC[iLeftRow + 1][iChannel];
                        int iDestinationFrames = static_cast<int>(pEnd - pCursor) >> iDestinationShift;
                        int iRightTarget = pSample->levelsB[iLeftRow + 1][iChannel];
                        if ((iLeftTarget != iLeftCurrent || iRightTarget != iRightCurrent)
                            && pSample->levelsD[iOutput][iChannel] == 0)
                        {
                            pSample->levelsD[iOutput][iChannel] = pSample->rampLength;
                        }

                        int iCount = pSample->levelsD[iOutput][iChannel];
                        int iSourceFramesLeft = static_cast<int>(uBytes) >> iSourceShift;
                        if (iSourceFramesLeft < iCount)
                        {
                            iCount = iSourceFramesLeft;
                        }

                        if (iDestinationFrames < iCount)
                        {
                            iCount = iDestinationFrames;
                        }

                        unsigned int uMixFlags = slow_ramp_mix(
                            uStereo != 0, uDestinationStereo, reinterpret_cast<void**>(&pMixSource), pFilteredEnd,
                            reinterpret_cast<void**>(&pCursor), pEnd, &iLeftCurrent, &iRightCurrent,
                            iLeftTarget - iLeftCurrent, iRightTarget - iRightCurrent, iCount);
                        pSample->levelsD[iOutput][iChannel] -= iCount;
                        if (pSample->levelsD[iOutput][iChannel] == 0)
                        {
                            if (pSample->endMode != 0)
                            {
                                pSample->endMode = 2;
                            }
                        }
                        else
                        {
                            pSample->levelsB[iLeftRow][iChannel] = iLeftCurrent;
                            pSample->levelsB[iLeftRow + 1][iChannel] = iRightCurrent;
                        }

                        if (pMixSource != pFilteredEnd && pCursor != pEnd)
                        {
                            if (iLeftTarget == 0 && iRightTarget == 0)
                            {
                                int iRemainingDestination = static_cast<int>(pEnd - pCursor) >> iDestinationShift;
                                int iRemainingSource = static_cast<int>(pFilteredEnd - pMixSource) >> iSourceShift;
                                if (iRemainingSource < iRemainingDestination)
                                {
                                    iRemainingDestination = iRemainingSource;
                                }

                                pCursor += iRemainingDestination << iDestinationShift;
                            }
                            else
                            {
                                float fLeftPeak = static_cast<float>(iLeftTarget) * MERGE_VOLUME_UNIT;
                                float fPeak = pSample->volume;
                                if (pSample->volume < fLeftPeak)
                                {
                                    pSample->volume = fLeftPeak;
                                    fPeak = fLeftPeak;
                                }

                                float fRightPeak = static_cast<float>(iRightTarget) * MERGE_VOLUME_UNIT;
                                if (fPeak < fRightPeak)
                                {
                                    pSample->volume = fRightPeak;
                                }

                                MarkProfileSample(pSample, MERGE_PROFILE_MIX);
                                StartTimer(&pDriver->timers[TIMER_MIX]);
                                unsigned int uRampFraction = 0;
                                int iRampState = 0;
                                pSample->stages[SAMPLE_STAGE_MERGE].merge.merge(
                                    &pMixSource, &uRampFraction, pFilteredEnd, reinterpret_cast<void**>(&pCursor), pEnd,
                                    &iRampState, &iRampState, MERGE_UNITY, iLeftTarget, iRightTarget, uMixFlags);
                                StopTimer(&pDriver->timers[TIMER_MIX]);
                                if (pMixSource != pFilteredEnd && pCursor != pEnd)
                                {
                                    unsigned int uSecondFraction = 0;
                                    int iSecondState = 0;
                                    StartTimer(&pDriver->timers[TIMER_MIX]);
                                    pSample->stages[SAMPLE_STAGE_MERGE].merge.merge(
                                        &pMixSource, &uSecondFraction, pFilteredEnd, reinterpret_cast<void**>(&pCursor),
                                        pEnd, &iSecondState, &iSecondState, MERGE_UNITY, iLeftTarget, iRightTarget,
                                        uMixFlags | MIX_FLAG_LOW_VOLUME);
                                    StopTimer(&pDriver->timers[TIMER_MIX]);
                                }
                            }
                        }

                        if (iOutput == 0)
                        {
                            iFirstOffset = static_cast<int>(pCursor - pDriver->outputs[0].buffer);
                        }
                    }

                    iEndOffset = iFirstOffset;
                }
                else
                {
                    iEndOffset = iOffset + static_cast<int>(uBytes);
                }
            }
            else
            {
                unsigned char* pCursor = pDestinationBytes + iOffset;
                MarkProfileSample(pSample, MERGE_PROFILE_MIX);
                StartTimer(&pDriver->timers[TIMER_MIX]);
                pSample->stages[SAMPLE_STAGE_MERGE].merge.merge(
                    &pSource, &pSample->resampleFraction, pSourceEnd, reinterpret_cast<void**>(&pCursor),
                    pDestinationBytes + iOffset + iDestBytes, piLeft, piRight, iStep, 0, 0,
                    (uFlags & ~MIX_FLAG_STEREO_DESTINATION) | MERGE_FLAG_CONVERT);
                StopTimer(&pDriver->timers[TIMER_MIX]);
                iEndOffset = static_cast<int>(pCursor - pDestinationBytes);
            }

            uOutputFraction = pSample->resampleFraction;
            pConsumed = pSource;
            iMixedOffset = iEndOffset;
        }

        if (!iPerChannel)
        {
            iConsumedBytes = static_cast<int>(pSource - pBlockStart);
            break;
        }

        ++iChannel;
        if (iChannel >= pSample->channelCount)
        {
            if (iMixedOffset != 0)
            {
                iConsumedBytes = (static_cast<int>(pConsumed - pBlockStart) >> 1) * iFrameBytes;
                pSample->resampleFraction = uOutputFraction;
                pSource = pConsumed;
                iEndOffset = iMixedOffset;
            }

            break;
        }

        pSample->resampleFraction = uInitialFraction;
        if ((iChannel & 1) == 0)
        {
            piLeft = pSample->channelPositionA + (iChannel >> 1);
        }
        else
        {
            piLeft = pSample->channelPositionB + (iChannel >> 1);
        }

        piRight = &iDummyState;
    }

    if (bHistory)
    {
        if (pSource < pSourceEnd)
        {
            for (int iFrame = 0; iFrame < pSample->channelCount; ++iFrame)
            {
                pSample->channelHeader[iFrame] = reinterpret_cast<const short*>(pLastFrame)[iFrame];
            }
        }
        else
        {
            *pSample->channelHeader = CHANNEL_HEADER_UNITY;
        }
    }
    else
    {
        pSample->buffers[pSample->currentBuffer].position += iConsumedBytes;
    }

    return iEndOffset;
}
