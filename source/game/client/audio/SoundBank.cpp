#include "game/client/audio/SoundBank.hpp"

#include <stdint.h>

static char ourtoupper(char value)
{
    if (value >= 'a' && value <= 'z')
    {
        return value - ('a' - 'A');
    }
    return value;
}

intptr_t FindAsset(SoundBank* soundBank, const char* name, SoundBank::AssetEntry* entries, int32_t count)
{
    for (;;)
    {
        if (count == 0)
        {
            return 0;
        }

        const int32_t midpoint = count / 2;
        SoundBank::AssetEntry* entry = entries + midpoint;
        int32_t comparison;
        const char* entryName = reinterpret_cast<const char*>(soundBank) + entry->nameOffset;
        const char* searchName = name;
        for (;;)
        {
            int32_t entryCharacter = *entryName;
            entryCharacter = ourtoupper(entryCharacter);
            int32_t searchCharacter = *searchName;
            searchCharacter = ourtoupper(searchCharacter);
            if (entryCharacter < searchCharacter)
            {
                comparison = -1;
                break;
            }
            if (entryCharacter > searchCharacter)
            {
                comparison = 1;
                break;
            }
            if (entryCharacter == 0)
            {
                comparison = 0;
                break;
            }
            ++entryName;
            ++searchName;
        }

        if (comparison == 0)
        {
            return reinterpret_cast<intptr_t>(entry);
        }

        if (count == 1)
        {
            return 0;
        }

        if (comparison > 0)
        {
            count = midpoint;
        }
        else
        {
            entries = entry + 1;
            count = count - midpoint - 1;
        }
    }
}
