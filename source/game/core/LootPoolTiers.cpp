#include "game/core/LootPoolTiers.hpp"

LootPoolTiers::LootPoolTiers(int tierCount, int bonusTierRolls, float bonusTierChance)
    : tierCount(tierCount),
      bonusTierRolls(bonusTierRolls),
      bonusTierChance(bonusTierChance)
{
}
