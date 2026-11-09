#ifndef GAME_CORE_TRIALMODE_HPP
#define GAME_CORE_TRIALMODE_HPP

#include <stdint.h>

#include "game/world/generation/MultiPlayerGameMode.hpp"

class ClientPacketListener;

class TrialMode : public MultiPlayerGameMode
{
public:
    TrialMode(int playerId, ClientPacketListener* connection, int userIndex);

    virtual uint32_t isImplemented();
};

#endif
