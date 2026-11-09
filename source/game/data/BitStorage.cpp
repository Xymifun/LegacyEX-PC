#include "game/data/BitStorage.hpp"

#include <stdint.h>

int32_t getMaskInt(uint32_t offset, uint32_t width)
{
    return static_cast<int32_t>((0xffffffffU >> (32 - width)) << offset);
}
