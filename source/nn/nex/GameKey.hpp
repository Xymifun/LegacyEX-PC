#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_GAMEKEY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_GAMEKEY_HPP

#include <stdint.h>

#include "nn/nex/_DDL_GameKey.hpp"

namespace nn
{
namespace nex
{

class GameKey : public _DDL_GameKey
{
public:
    GameKey() : _DDL_GameKey()
    {
        m_uiTitleID = 0;
        m_uiTitleVersion = 0;
    }

    GameKey(unsigned long long uiTitleID, unsigned short uiTitleVersion);

    virtual ~GameKey() {}

    bool operator==(const GameKey& rOther) const;
    bool operator!=(const GameKey& rOther) const;
};

}
}

#endif
