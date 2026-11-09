#include "game/core/TrialMode.hpp"

#include "game/minigame/FullTutorial.hpp"

TrialMode::TrialMode(int playerId, ClientPacketListener* connection, int userIndex)
    : MultiPlayerGameMode(playerId, connection, userIndex, false, false)
{
    mTutorial = new FullTutorial(playerId, mGameModeConstraints, true);
}

uint32_t TrialMode::isImplemented()
{
    return 1;
}
