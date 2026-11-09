#include "game/world/block/BlockDestroyedMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlockDestroyedMedal::BlockDestroyedMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false)
{
}

float BlockDestroyedMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, getBlocksDestroyed);
}
