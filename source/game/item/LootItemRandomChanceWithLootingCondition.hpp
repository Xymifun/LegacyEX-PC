#ifndef GAME_ITEM_LOOTITEMRANDOMCHANCEWITHLOOTINGCONDITION_HPP
#define GAME_ITEM_LOOTITEMRANDOMCHANCEWITHLOOTINGCONDITION_HPP

#include "game/core/LootItemCondition.hpp"

class LootTableContext;
class Random;

class LootItemRandomChanceWithLootingCondition : public LootItemCondition
{
public:
    LootItemRandomChanceWithLootingCondition(float chance, float lootingMultiplier);

    virtual bool applies(Random* random, LootTableContext& context);

private:
    float chance;
    float lootingMultiplier;
};

#endif
