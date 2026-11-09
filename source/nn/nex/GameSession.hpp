#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_GAMESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_GAMESESSION_HPP

#include "nn/nex/_DDL_GameSession.hpp"

namespace nn
{
namespace nex
{

class GameSession : public _DDL_GameSession
{
public:
    GameSession() : _DDL_GameSession() {}

    virtual ~GameSession() {}
};

}
}

#endif
