#ifndef GAME_ITEM_LOOTITEMRANDOMCHANCEWITHSPECIALMODIFIERCONDITION_HPP
#define GAME_ITEM_LOOTITEMRANDOMCHANCEWITHSPECIALMODIFIERCONDITION_HPP

#include "game/core/LootItemCondition.hpp"

class LootTableContext;
class Random;

class LootItemRandomChanceWithSpecialModifierCondition : public LootItemCondition
{
public:
    LootItemRandomChanceWithSpecialModifierCondition(float chance);

    virtual bool applies(Random* random, LootTableContext& context);

private:
    float chance;
};

#endif
