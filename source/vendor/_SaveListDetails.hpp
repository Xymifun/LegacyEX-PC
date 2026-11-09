#ifndef SOURCE_VENDOR_SAVELISTDETAILS_HPP
#define SOURCE_VENDOR_SAVELISTDETAILS_HPP

#include <stdint.h>
#include <wchar.h>

class _SaveListDetails
{
public:
    _SaveListDetails();
    ~_SaveListDetails() {}
    uint32_t m_saveIndex;
    unsigned char* m_thumbnail;
    uint32_t m_thumbnailSize;
    wchar_t m_title[128];
    char m_filename[32];
};

#endif
