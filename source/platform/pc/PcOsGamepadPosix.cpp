#ifndef _WIN32

#include "platform/pc/PcOs.hpp"

size_t PcOsGamepadPoll(PcOsGamepadState*, size_t)
{
    return 0;
}

void PcOsGamepadSetPlayer(unsigned int, int) {}

#endif
