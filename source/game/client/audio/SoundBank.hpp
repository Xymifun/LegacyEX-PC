#ifndef GAME_CLIENT_AUDIO_SOUNDBANK_HPP
#define GAME_CLIENT_AUDIO_SOUNDBANK_HPP

#include <stddef.h>
#include <stdint.h>

struct SoundBank
{
    struct AssetEntry
    {
        int32_t nameOffset;
        int32_t value;
    };
};

intptr_t FindAsset(SoundBank* soundBank, const char* name, SoundBank::AssetEntry* entries, int32_t count);

#if __SIZEOF_POINTER__ == 8
#define SOUND_BANK_OFFSETS 1
template <typename T>
inline T* SoundBankAt(const void* bank, unsigned int offset)
{
    return offset == 0 ? NULL : reinterpret_cast<T*>(const_cast<char*>(static_cast<const char*>(bank)) + offset);
}
#else
#define SOUND_BANK_OFFSETS 0
#endif

#endif
