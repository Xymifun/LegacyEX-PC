#ifndef VENDOR__XUSER_SIGNIN_INFO_HPP
#define VENDOR__XUSER_SIGNIN_INFO_HPP

#include <stdint.h>

#include "game/world/generation/PlayerUID.hpp"

struct _XUSER_SIGNIN_INFO : PlayerUID
{
    _XUSER_SIGNIN_INFO();

    uint32_t dwInfoFlags;
};

#endif
