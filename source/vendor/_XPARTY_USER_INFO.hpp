#ifndef SOURCE_VENDOR_XPARTY_USER_INFO_HPP
#define SOURCE_VENDOR_XPARTY_USER_INFO_HPP

#include "game/world/generation/PlayerUID.hpp"

class _XPARTY_USER_INFO : public PlayerUID
{
public:
    _XPARTY_USER_INFO();

private:
    unsigned char m_userData[44];
};

#endif
