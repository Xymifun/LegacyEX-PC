#include "game/world/block/BlockPresentMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlockPresentMedal::BlockPresentMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op)
{
}

float BlockPresentMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, getBlocksPresent);
}
