#ifndef VENDOR_DISKSAVEBUFFER_HPP
#define VENDOR_DISKSAVEBUFFER_HPP

#include <string>

struct DiskSaveBuffer
{
    unsigned char* data;
    unsigned int length;
    std::wstring name;
};

#endif
