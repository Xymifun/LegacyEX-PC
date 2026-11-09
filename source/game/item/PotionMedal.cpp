#include "game/item/PotionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float PotionMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->getTotalPotionsUsed())));
}
