#include "game/world/block/BlocksDestroyedFromBelowMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float BlocksDestroyedFromBelowMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->mBlocksDestroyedFromBelow)));
}
