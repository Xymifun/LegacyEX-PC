#include "game/item/SplashPotionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float SplashPotionMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->getTotalSplashPotionsUsed())));
}
