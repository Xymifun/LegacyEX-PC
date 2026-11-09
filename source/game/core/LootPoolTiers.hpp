#ifndef GAME_CORE_LOOTPOOLTIERS_HPP
#define GAME_CORE_LOOTPOOLTIERS_HPP

class LootPoolTiers
{
public:
    int tierCount;
    int bonusTierRolls;
    float bonusTierChance;

    LootPoolTiers(int tierCount, int bonusTierRolls, float bonusTierChance);
};

#endif
