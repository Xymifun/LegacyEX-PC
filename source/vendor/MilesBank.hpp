#ifndef SOURCE_VENDOR_MILES_BANK_HPP
#define SOURCE_VENDOR_MILES_BANK_HPP

#include "game/client/audio/SoundBank.hpp"
#include "vendor/MilesMem.hpp"

struct _DIG_DRIVER;
struct EventSettings;

const int SOUND_BANK_VERSION = 8;
const int ENVIRONMENT_PRESET_MAGIC = 0x454e5620;
const int BANK_HEADER_READ_SIZE = 0x1000;

enum EnvironmentSettingKind
{
    ENVIRONMENT_ROOM_TYPE = 0,
    ENVIRONMENT_MASTER_REVERB = 1,
    ENVIRONMENT_MASTER_REVERB_LEVELS = 2,
    ENVIRONMENT_MASTER_VOLUME = 3,
    ENVIRONMENT_ROLLOFF_FACTOR = 4,
    ENVIRONMENT_DOPPLER_FACTOR = 5,
    ENVIRONMENT_SPEAKER_RADIUS = 6,
    ENVIRONMENT_KIND_COUNT = 7
};

union EnvironmentValue
{
    int integer;
    float real;
};

struct EnvironmentPair
{
    EnvironmentValue value;
    unsigned int expression;
};

struct EnvironmentRecord
{
    unsigned int kind;
    EnvironmentPair pairs[1];
};

struct EnvironmentPreset
{
    int magic;
    int reserved0[2];
    int size;
    EnvironmentRecord records[1];
};

struct SoundBankImage
{
    int magic;
    int version;
    int memoryUsage;
    int pathLength;
#if SOUND_BANK_OFFSETS
    unsigned int path;
    unsigned int events;
    unsigned int environments;
    unsigned int presets;
    unsigned int sounds;
    unsigned int settings;
#else
    char* path;
    SoundBank::AssetEntry* events;
    SoundBank::AssetEntry* environments;
    SoundBank::AssetEntry* presets;
    SoundBank::AssetEntry* sounds;
    EventSettings* settings;
#endif
    int eventCount;
    int environmentCount;
    int presetCount;
    int soundCount;
    char name[1];
};

extern "C" SoundBank* AIL_open_soundbank(const char* fileName, const char* bankName);
extern "C" void AIL_close_soundbank(SoundBank* bank);
extern "C" int AIL_get_event_contents(SoundBank* bank, const char* name, const unsigned char** contents);
extern "C" void* AIL_find_environment_preset(SoundBank* bank, const char* name);
extern "C" int AIL_resolve_raw_environment_preset(void* preset, PresetEvalFunc evaluate);
extern "C" int AIL_apply_raw_environment_preset(_DIG_DRIVER* driver, void* preset);
extern "C" int AIL_unapply_raw_environment_preset(_DIG_DRIVER* driver, void* preset);
extern "C" int AIL_get_soundbank_mem_usage(SoundBank* bank);

#endif
