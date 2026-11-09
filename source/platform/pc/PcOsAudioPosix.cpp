#ifndef _WIN32

#include "platform/pc/PcOs.hpp"

bool PcOsAudioOpen(unsigned int, unsigned int, PcOsAudioFillFn)
{
    return false;
}

#endif
