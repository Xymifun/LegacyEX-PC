#include "vendor/MilesBank.hpp"

#include "game/core/BigEndian.hpp"
#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesApi.hpp"
#include "vendor/MilesAsync.hpp"
#include "vendor/MilesEventExec.hpp"
#include "vendor/MilesIO.hpp"

#include <math.h>
#include <stddef.h>
#include <string.h>


namespace
{
const unsigned int BANK_WAIT_FOREVER = 0xFFFFFFFF;
const unsigned int BANK_READ_CLOSE_WHEN_DONE = 1;
const unsigned int BANK_READ_APPEND = 4;
const int BANK_READ_COMPLETE = 1;
const int BANK_HEADER_READ_LINE = 0x11e;
const int BANK_IMAGE_ALLOC_LINE = 0x147;
const int BANK_BODY_READ_LINE = 0x156;
const int BANK_PATH_ALLOC_LINE = 0x182;
const int ENVIRONMENT_ROOM_BUS = 0;
const int ENVIRONMENT_SPEAKER_COUNT = 0;
const float ENVIRONMENT_DECIBELS_PER_DECADE = 20.0f;
const double ENVIRONMENT_DECADE_BASE = 10.0;
const float DEFAULT_REVERB_TIME = 1.493f;
const float DEFAULT_REVERB_PREDELAY = 0.011f;
const float DEFAULT_REVERB_DAMPING = 0.5f;
const float DEFAULT_REVERB_DRY = 1.0f;
const float DEFAULT_REVERB_WET = 0.0f;
const float DEFAULT_MASTER_VOLUME = 1.0f;
const float DEFAULT_ROLLOFF_FACTOR = 1.0f;
const float DEFAULT_DOPPLER_FACTOR = 1.0f;
const float DEFAULT_SPEAKER_RADIUS = 3.0f;
const char HLBANK_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\hlbank.cpp";
const char EVENT_NOT_FOUND_ERROR[] = "Event Not Found.";
const char ENVIRONMENT_NOT_FOUND_ERROR[] = "Environment Not Found.";
const char OPEN_FAILED_ERROR[] = "Failed to open sound bank.";
const char NOT_A_BANK_ERROR[] = "Sound file isn't a bank.";
const char BANK_VERSION_ERROR[] = "Bank file version doesn't match this runtime.";
const char AUDITION_NAME_ERROR[] =
    "Audition bank name provided doesn't match the bank name in the loaded file. Are you loading the "
    "bank you think you are?";

union BankHeaderBuffer
{
    SoundBankImage image;
    unsigned char bytes[BANK_HEADER_READ_SIZE];
};

inline EnvironmentRecord* NextRecord(EnvironmentRecord* record, int dataWords)
{
    return reinterpret_cast<EnvironmentRecord*>(reinterpret_cast<char*>(record)
                                                + (dataWords + 1) * sizeof(unsigned int));
}

inline void EvaluatePair(EnvironmentPreset* environment, EnvironmentPair* pair, PresetEvalFunc evaluate)
{
    if (pair->expression != 0)
    {
        evaluate(NULL, reinterpret_cast<const char*>(environment) + pair->expression, -1, &pair->value, 1);
    }
}

#if !SOUND_BANK_OFFSETS
inline SoundBank::AssetEntry* Relocate(SoundBankImage* bank, SoundBank::AssetEntry* offset)
{
    return reinterpret_cast<SoundBank::AssetEntry*>(reinterpret_cast<char*>(bank) + reinterpret_cast<int>(offset));
}
#endif

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
const unsigned int BANK_HEADER_WORDS = 14;
const unsigned int BANK_EVENTS_WORD = 5;
const unsigned int BANK_SETTINGS_WORD = 9;
const unsigned int BANK_EVENT_COUNT_WORD = 10;
const unsigned int BANK_TABLE_COUNT = 4;
const unsigned int ASSET_ENTRY_WORDS = 2;
const unsigned int SOUND_RECORD_NAME_WORDS = 2;
const unsigned int SOUND_RECORD_INFO_OFFSET = 0xc;
const unsigned int SOUND_RECORD_INFO_WORDS = 11;
const unsigned int SOUND_RECORD_MARKERS_OFFSET = 0x38;
const unsigned int MARKER_ENTRY_WORDS = 3;
const unsigned int PRESET_HEADER_WORDS = 4;
const unsigned int PRESET_SIZE_OFFSET = 0xc;
const unsigned int SETTINGS_HEADER_WORDS = 8;
const unsigned int SETTINGS_STEP_COUNT_WORDS = 4;
const unsigned int SETTINGS_VARIABLE_COUNT_WORD = 5;
const unsigned int SETTINGS_BUS_COUNT_WORD = 6;
const unsigned int SETTINGS_CURVE_COUNT_WORD = 7;
const unsigned int SETTINGS_BUS_WORDS = 3;
const unsigned int SETTINGS_CURVE_HEADER_WORDS = 2;
const unsigned int SETTINGS_VARIABLE_WORDS = 2;
const unsigned int WORD_SIZE = 4;

struct BankImageView
{
    unsigned char* base;
    unsigned int size;
};

bool SwapWords(const BankImageView& image, unsigned int offset, unsigned int count)
{
    if (offset > image.size || count > (image.size - offset) / WORD_SIZE)
    {
        return false;
    }

    for (unsigned int i = 0; i < count; ++i)
    {
        unsigned int word;
        memcpy(&word, image.base + offset + i * WORD_SIZE, WORD_SIZE);
        word = BigEndian::ToHost(word);
        memcpy(image.base + offset + i * WORD_SIZE, &word, WORD_SIZE);
    }

    return true;
}

unsigned int ReadWord(const BankImageView& image, unsigned int offset)
{
    unsigned int word;
    memcpy(&word, image.base + offset, WORD_SIZE);
    return word;
}

bool SwapWordAndRead(const BankImageView& image, unsigned int offset, unsigned int* value)
{
    if (!SwapWords(image, offset, 1))
    {
        return false;
    }

    *value = ReadWord(image, offset);
    return true;
}

bool SwapPresetToHostOrder(const BankImageView& image, unsigned int offset)
{
    unsigned int size;
    if (!SwapWords(image, offset, PRESET_HEADER_WORDS - 1) || !SwapWordAndRead(image, offset + PRESET_SIZE_OFFSET, &size)
        || size < PRESET_HEADER_WORDS * WORD_SIZE)
    {
        return false;
    }

    return SwapWords(image, offset + PRESET_HEADER_WORDS * WORD_SIZE, size / WORD_SIZE - PRESET_HEADER_WORDS);
}

bool SwapSoundRecordToHostOrder(const BankImageView& image, unsigned int offset)
{
    unsigned int markerCount;
    if (!SwapWords(image, offset, SOUND_RECORD_NAME_WORDS)
        || !SwapWords(image, offset + SOUND_RECORD_INFO_OFFSET, SOUND_RECORD_INFO_WORDS)
        || !SwapWordAndRead(image, offset + SOUND_RECORD_MARKERS_OFFSET, &markerCount))
    {
        return false;
    }

    return SwapWords(image, offset + SOUND_RECORD_MARKERS_OFFSET + WORD_SIZE, markerCount * MARKER_ENTRY_WORDS);
}

bool SwapSettingsToHostOrder(const BankImageView& image, unsigned int offset)
{
    if (!SwapWords(image, offset, SETTINGS_HEADER_WORDS))
    {
        return false;
    }

    unsigned int stepCount = 0;
    for (unsigned int i = 1; i <= SETTINGS_STEP_COUNT_WORDS; ++i)
    {
        stepCount += ReadWord(image, offset + i * WORD_SIZE);
    }

    const unsigned int busCount = ReadWord(image, offset + SETTINGS_BUS_COUNT_WORD * WORD_SIZE);
    const unsigned int curveCount = ReadWord(image, offset + SETTINGS_CURVE_COUNT_WORD * WORD_SIZE);
    const unsigned int variableCount = ReadWord(image, offset + SETTINGS_VARIABLE_COUNT_WORD * WORD_SIZE);
    unsigned int cursor = offset + SETTINGS_HEADER_WORDS * WORD_SIZE;
    if (!SwapWords(image, cursor, stepCount))
    {
        return false;
    }

    cursor += stepCount * WORD_SIZE;
    if (!SwapWords(image, cursor, busCount * SETTINGS_BUS_WORDS))
    {
        return false;
    }

    cursor += busCount * SETTINGS_BUS_WORDS * WORD_SIZE;
    for (unsigned int i = 0; i < curveCount; ++i)
    {
        if (!SwapWords(image, cursor, SETTINGS_CURVE_HEADER_WORDS))
        {
            return false;
        }

        const unsigned int curveSize = ReadWord(image, cursor + WORD_SIZE);
        cursor += SETTINGS_CURVE_HEADER_WORDS * WORD_SIZE;
        if (!SwapWords(image, cursor, curveSize / WORD_SIZE))
        {
            return false;
        }

        cursor += curveSize / WORD_SIZE * WORD_SIZE;
    }

    return SwapWords(image, cursor, variableCount * SETTINGS_VARIABLE_WORDS);
}

bool IsBigEndianBank(const SoundBankImage& header)
{
    return header.magic == BigEndian::ToHost(SOUND_BANK_MAGIC);
}

void ConvertBankHeaderToHostOrder(SoundBankImage* header)
{
    const BankImageView image = {reinterpret_cast<unsigned char*>(header), BANK_HEADER_WORDS * WORD_SIZE};
    SwapWords(image, 0, BANK_HEADER_WORDS);
}

bool ConvertBankBodyToHostOrder(SoundBankImage* bank)
{
    const BankImageView image = {reinterpret_cast<unsigned char*>(bank), static_cast<unsigned int>(bank->memoryUsage)};
    for (unsigned int table = 0; table < BANK_TABLE_COUNT; ++table)
    {
        const unsigned int tableOffset = ReadWord(image, (BANK_EVENTS_WORD + table) * WORD_SIZE);
        const unsigned int count = ReadWord(image, (BANK_EVENT_COUNT_WORD + table) * WORD_SIZE);
        if (!SwapWords(image, tableOffset, count * ASSET_ENTRY_WORDS))
        {
            return false;
        }

        if (table == 0)
        {
            continue;
        }

        for (unsigned int i = 0; i < count; ++i)
        {
            const unsigned int value = ReadWord(image, tableOffset + (i * ASSET_ENTRY_WORDS + 1) * WORD_SIZE);
            const bool converted = table == BANK_TABLE_COUNT - 1 ? SwapSoundRecordToHostOrder(image, value)
                                                                 : SwapPresetToHostOrder(image, value);
            if (!converted)
            {
                return false;
            }
        }
    }

    const unsigned int settings = ReadWord(image, BANK_SETTINGS_WORD * WORD_SIZE);
    return settings == 0 || SwapSettingsToHostOrder(image, settings);
}
#endif
}

extern "C" void AIL_close_soundbank(SoundBank* bank)
{
    if (bank != NULL)
    {
#if !SOUND_BANK_OFFSETS
        AIL_mem_free_lock(reinterpret_cast<SoundBankImage*>(bank)->path);
#endif
        AIL_mem_free_lock(bank);
    }
}

extern "C" SoundBank* AIL_open_soundbank(const char* fileName, const char* bankName)
{
    if (fileName == NULL)
    {
        return NULL;
    }

    MilesAsyncRead request;
    BankHeaderBuffer header;

    memset(&request, 0, sizeof(request));
    request.pBuffer = &header;
    strcpy(request.szFileName, fileName);
    request.iLengthHigh = 0;
    request.iLength = BANK_HEADER_READ_SIZE;
    request.iOwnerLine = BANK_HEADER_READ_LINE;
    request.pOwnerName = HLBANK_FILE;
    request.uFlags = BANK_READ_CLOSE_WHEN_DONE;
    MilesAsyncFileRead(&request);
    if (MilesAsyncFileStatus(&request, BANK_WAIT_FOREVER) != BANK_READ_COMPLETE)
    {
        CloseAsync(&request);
        AIL_set_error(OPEN_FAILED_ERROR);
        return NULL;
    }

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    const bool bigEndianBank = IsBigEndianBank(header.image);
    if (bigEndianBank)
    {
        ConvertBankHeaderToHostOrder(&header.image);
    }
#endif

    if (header.image.magic != SOUND_BANK_MAGIC)
    {
        CloseAsync(&request);
        AIL_set_error(NOT_A_BANK_ERROR);
        return NULL;
    }

    if (header.image.version != SOUND_BANK_VERSION)
    {
        CloseAsync(&request);
        AIL_set_error(BANK_VERSION_ERROR);
        return NULL;
    }

    if (bankName != NULL && AIL_stricmp(header.image.name, bankName) != 0)
    {
        CloseAsync(&request);
        AIL_set_error(AUDITION_NAME_ERROR);
        return NULL;
    }

#if SOUND_BANK_OFFSETS
    const size_t pathLength = strlen(fileName);
    SoundBankImage* bank = static_cast<SoundBankImage*>(
        AIL_mem_alloc_lock_info(header.image.memoryUsage + pathLength + 1, HLBANK_FILE, BANK_IMAGE_ALLOC_LINE));
#else
    SoundBankImage* bank = static_cast<SoundBankImage*>(
        AIL_mem_alloc_lock_info(header.image.memoryUsage, HLBANK_FILE, BANK_IMAGE_ALLOC_LINE));
#endif
    int copySize = request.iBytesRead;
    if (copySize > header.image.memoryUsage)
    {
        copySize = header.image.memoryUsage;
    }

    memcpy(bank, &header, copySize);
    if (header.image.memoryUsage > request.iBytesRead)
    {
        request.iLength = header.image.memoryUsage - request.iBytesRead;
        request.pOwnerName = HLBANK_FILE;
        request.uFlags = BANK_READ_APPEND;
        request.pBuffer = reinterpret_cast<char*>(bank) + request.iBytesRead;
        request.iOwnerLine = BANK_BODY_READ_LINE;
        request.iLengthHigh = request.iLength >> 31;
        request.iBytesRead = 0;
        MilesAsyncFileRead(&request);
        if (MilesAsyncFileStatus(&request, BANK_WAIT_FOREVER) != BANK_READ_COMPLETE)
        {
            AIL_mem_free_lock(bank);
            return NULL;
        }
    }
    else
    {
        CloseAsync(&request);
    }

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    if (bigEndianBank && !ConvertBankBodyToHostOrder(bank))
    {
        AIL_mem_free_lock(bank);
        AIL_set_error(NOT_A_BANK_ERROR);
        return NULL;
    }
#endif

#if SOUND_BANK_OFFSETS
    if (bank->settings != 0)
    {
        AIL_set_event_settings(SoundBankAt<EventSettings>(bank, bank->settings));
    }

    bank->path = header.image.memoryUsage;
    memcpy(SoundBankAt<char>(bank, bank->path), fileName, pathLength + 1);
    bank->pathLength = pathLength;
    return reinterpret_cast<SoundBank*>(bank);
#else
    bank->events = Relocate(bank, bank->events);
    bank->environments = Relocate(bank, bank->environments);
    bank->sounds = Relocate(bank, bank->sounds);
    bank->presets = Relocate(bank, bank->presets);
    if (bank->settings != NULL)
    {
        bank->settings = reinterpret_cast<EventSettings*>(reinterpret_cast<char*>(bank)
                                                          + reinterpret_cast<int>(bank->settings));
        AIL_set_event_settings(bank->settings);
    }

    const size_t pathLength = strlen(fileName);
    bank->path = static_cast<char*>(AIL_mem_alloc_lock_info(pathLength + 1, HLBANK_FILE, BANK_PATH_ALLOC_LINE));
    memcpy(bank->path, fileName, pathLength + 1);
    bank->pathLength = pathLength;
    return reinterpret_cast<SoundBank*>(bank);
#endif
}

extern "C" int AIL_get_event_contents(SoundBank* bank, const char* name, const unsigned char** contents)
{
    SoundBankImage* image = reinterpret_cast<SoundBankImage*>(bank);
    if (image == NULL || contents == NULL || name == NULL || image->magic != SOUND_BANK_MAGIC)
    {
        return 0;
    }

    SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
#if SOUND_BANK_OFFSETS
        FindAsset(bank, name, SoundBankAt<SoundBank::AssetEntry>(image, image->events), image->eventCount));
#else
        FindAsset(bank, name, image->events, image->eventCount));
#endif
    if (entry == NULL)
    {
        AIL_set_error(EVENT_NOT_FOUND_ERROR);
        return 0;
    }

    *contents = reinterpret_cast<const unsigned char*>(image) + entry->value;
    return 1;
}

extern "C" int AIL_resolve_raw_environment_preset(void* preset, PresetEvalFunc evaluate)
{
    EnvironmentPreset* environment = static_cast<EnvironmentPreset*>(preset);
    if (environment == NULL || evaluate == NULL || environment->magic != ENVIRONMENT_PRESET_MAGIC)
    {
        return 0;
    }

    EnvironmentRecord* end = reinterpret_cast<EnvironmentRecord*>(reinterpret_cast<char*>(environment)
                                                                  + environment->size);
    int dataWords;
    EnvironmentRecord* record = environment->records;
    while (record != end)
    {
        dataWords = 2;
        switch (record->kind)
        {
        case ENVIRONMENT_ROOM_TYPE:
            break;
        case ENVIRONMENT_MASTER_REVERB:
            EvaluatePair(environment, &record->pairs[0], evaluate);
            EvaluatePair(environment, &record->pairs[1], evaluate);
            EvaluatePair(environment, &record->pairs[2], evaluate);
            dataWords = 6;
            break;
        case ENVIRONMENT_MASTER_REVERB_LEVELS:
            EvaluatePair(environment, &record->pairs[0], evaluate);
            EvaluatePair(environment, &record->pairs[1], evaluate);
            dataWords = 4;
            break;
        case ENVIRONMENT_MASTER_VOLUME:
        case ENVIRONMENT_ROLLOFF_FACTOR:
        case ENVIRONMENT_DOPPLER_FACTOR:
        case ENVIRONMENT_SPEAKER_RADIUS:
            EvaluatePair(environment, &record->pairs[0], evaluate);
            break;
        default:
            break;
        }

        record = NextRecord(record, dataWords);
    }

    return 1;
}

extern "C" int AIL_apply_raw_environment_preset(_DIG_DRIVER* driver, void* preset)
{
    EnvironmentPreset* environment = static_cast<EnvironmentPreset*>(preset);
    if (driver == NULL || environment == NULL || environment->magic != ENVIRONMENT_PRESET_MAGIC)
    {
        return 0;
    }

    EnvironmentRecord* end = reinterpret_cast<EnvironmentRecord*>(reinterpret_cast<char*>(environment)
                                                                  + environment->size);
    int dataWords;
    EnvironmentRecord* record = environment->records;
    while (record != end)
    {
        dataWords = 2;
        switch (record->kind)
        {
        case ENVIRONMENT_ROOM_TYPE:
            AIL_set_room_type(driver, ENVIRONMENT_ROOM_BUS, record->pairs[0].value.integer);
            break;
        case ENVIRONMENT_MASTER_REVERB:
            AIL_set_digital_master_reverb(driver, ENVIRONMENT_ROOM_BUS, record->pairs[0].value.real,
                                          record->pairs[1].value.real, record->pairs[2].value.real);
            dataWords = 6;
            break;
        case ENVIRONMENT_MASTER_REVERB_LEVELS:
        {
            const float dry = static_cast<float>(
                pow(ENVIRONMENT_DECADE_BASE,
                    static_cast<double>(record->pairs[1].value.real / ENVIRONMENT_DECIBELS_PER_DECADE)));
            const float wet = static_cast<float>(
                pow(ENVIRONMENT_DECADE_BASE,
                    static_cast<double>(record->pairs[0].value.real / ENVIRONMENT_DECIBELS_PER_DECADE)));
            AIL_set_digital_master_reverb_levels(driver, ENVIRONMENT_ROOM_BUS, dry, wet);
            dataWords = 4;
            break;
        }
        case ENVIRONMENT_MASTER_VOLUME:
        {
            const float volume = static_cast<float>(
                pow(ENVIRONMENT_DECADE_BASE,
                    static_cast<double>(record->pairs[0].value.real / ENVIRONMENT_DECIBELS_PER_DECADE)));
            AIL_set_digital_master_volume_level(driver, volume);
            break;
        }
        case ENVIRONMENT_ROLLOFF_FACTOR:
            AIL_set_3D_rolloff_factor(driver, record->pairs[0].value.real);
            break;
        case ENVIRONMENT_DOPPLER_FACTOR:
            AIL_set_3D_doppler_factor(driver, record->pairs[0].value.real);
            break;
        case ENVIRONMENT_SPEAKER_RADIUS:
            AIL_set_speaker_configuration(driver, NULL, ENVIRONMENT_SPEAKER_COUNT, record->pairs[0].value.real);
            break;
        default:
            break;
        }

        record = NextRecord(record, dataWords);
    }

    return 1;
}

extern "C" void* AIL_find_environment_preset(SoundBank* bank, const char* name)
{
    SoundBankImage* image = reinterpret_cast<SoundBankImage*>(bank);
    if (name == NULL || bank == NULL || image->magic != SOUND_BANK_MAGIC)
    {
        return NULL;
    }

    SoundBank::AssetEntry* entry = reinterpret_cast<SoundBank::AssetEntry*>(
#if SOUND_BANK_OFFSETS
        FindAsset(bank, name, SoundBankAt<SoundBank::AssetEntry>(image, image->environments), image->environmentCount));
#else
        FindAsset(bank, name, image->environments, image->environmentCount));
#endif
    if (entry == NULL)
    {
        AIL_set_error(ENVIRONMENT_NOT_FOUND_ERROR);
        return NULL;
    }

    return reinterpret_cast<char*>(image) + entry->value;
}

extern "C" int AIL_unapply_raw_environment_preset(_DIG_DRIVER* driver, void* preset)
{
    EnvironmentPreset* environment = static_cast<EnvironmentPreset*>(preset);
    if (driver == NULL || environment == NULL || environment->magic != ENVIRONMENT_PRESET_MAGIC)
    {
        return 0;
    }

    EnvironmentRecord* record = environment->records;
    EnvironmentRecord* end = reinterpret_cast<EnvironmentRecord*>(reinterpret_cast<char*>(environment)
                                                                  + environment->size);
    while (record != end)
    {
        int dataWords = 2;
        switch (record->kind)
        {
        case ENVIRONMENT_ROOM_TYPE:
            AIL_set_room_type(driver, ENVIRONMENT_ROOM_BUS, 0);
            break;
        case ENVIRONMENT_MASTER_REVERB:
            AIL_set_digital_master_reverb(driver, ENVIRONMENT_ROOM_BUS, DEFAULT_REVERB_TIME, DEFAULT_REVERB_PREDELAY,
                                          DEFAULT_REVERB_DAMPING);
            dataWords = 6;
            break;
        case ENVIRONMENT_MASTER_REVERB_LEVELS:
            AIL_set_digital_master_reverb_levels(driver, ENVIRONMENT_ROOM_BUS, DEFAULT_REVERB_DRY, DEFAULT_REVERB_WET);
            dataWords = 4;
            break;
        case ENVIRONMENT_MASTER_VOLUME:
            AIL_set_digital_master_volume_level(driver, DEFAULT_MASTER_VOLUME);
            break;
        case ENVIRONMENT_ROLLOFF_FACTOR:
            AIL_set_3D_rolloff_factor(driver, DEFAULT_ROLLOFF_FACTOR);
            break;
        case ENVIRONMENT_DOPPLER_FACTOR:
            AIL_set_3D_doppler_factor(driver, DEFAULT_DOPPLER_FACTOR);
            break;
        case ENVIRONMENT_SPEAKER_RADIUS:
            AIL_set_speaker_configuration(driver, NULL, ENVIRONMENT_SPEAKER_COUNT, DEFAULT_SPEAKER_RADIUS);
            break;
        default:
            break;
        }

        record = NextRecord(record, dataWords);
    }

    return 1;
}

extern "C" int AIL_get_soundbank_mem_usage(SoundBank* bank)
{
    SoundBankImage* image = reinterpret_cast<SoundBankImage*>(bank);
    int usage = 0;
    if (image->magic == SOUND_BANK_MAGIC)
    {
        usage = image->memoryUsage;
    }

    return usage;
}
