#ifndef SOURCE_VENDOR_MILES_MARKER_HPP
#define SOURCE_VENDOR_MILES_MARKER_HPP

struct SoundBank;

struct MilesMarkerEntry
{
    unsigned int uNameOffset;
    unsigned int uIndex;
    unsigned int uType;
};

struct MilesMarkerList
{
    int iCount;
    MilesMarkerEntry entries[1];
};

extern "C" unsigned int AIL_find_marker_in_list(const MilesMarkerList* pList, const char* pName, unsigned int* puType);
extern "C" const MilesMarkerList* AIL_get_marker_list(SoundBank* pBank, const char* pName);

#endif
