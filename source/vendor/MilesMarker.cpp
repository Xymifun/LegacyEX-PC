#include "vendor/MilesMarker.hpp"

#include <stddef.h>
#include <stdint.h>

#include "vendor/MilesMem.hpp"

static const unsigned int MILES_MARKER_NOT_FOUND = 0xFFFFFFFF;

extern "C" int AIL_stricmp(const char* pFirst, const char* pSecond);

extern "C" unsigned int AIL_find_marker_in_list(const MilesMarkerList* pList, const char* pName, unsigned int* puType)
{
    const MilesMarkerEntry* pEntry;
    int iRemaining;

    if (pList != NULL && pName != NULL)
    {
        pEntry = pList->entries;

        for (iRemaining = pList->iCount; iRemaining != 0; iRemaining--)
        {
            if (AIL_stricmp(reinterpret_cast<const char*>(pList) + pEntry->uNameOffset, pName) == 0)
            {
                *puType = pEntry->uType;
                return pEntry->uIndex;
            }

            pEntry++;
        }
    }

    return MILES_MARKER_NOT_FOUND;
}

extern "C" const MilesMarkerList* AIL_get_marker_list(SoundBank* pBank, const char* pName)
{
    SoundBankHeader* pHeader;
    SoundBank::AssetEntry* pEntry;

    pHeader = reinterpret_cast<SoundBankHeader*>(pBank);

    if (pHeader == NULL || pName == NULL)
    {
        return NULL;
    }

    pEntry = reinterpret_cast<SoundBank::AssetEntry*>(
#if SOUND_BANK_OFFSETS
        static_cast<uintptr_t>(FindAsset(pBank, pName, SoundBankAt<SoundBank::AssetEntry>(pHeader, pHeader->sounds),
                                         pHeader->soundCount)));
#else
        static_cast<uintptr_t>(FindAsset(pBank, pName, pHeader->sounds, pHeader->soundCount)));
#endif

    if (pEntry == NULL)
    {
        return NULL;
    }

    return reinterpret_cast<const MilesMarkerList*>(reinterpret_cast<char*>(pHeader) + pEntry->value
                                                    + offsetof(SoundBankHeader, name));
}
