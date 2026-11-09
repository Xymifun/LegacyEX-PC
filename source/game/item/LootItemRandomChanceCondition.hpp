#ifndef GAME_ITEM_LOOTITEMRANDOMCHANCECONDITION_HPP
#define GAME_ITEM_LOOTITEMRANDOMCHANCECONDITION_HPP

#include "game/core/LootItemCondition.hpp"

class LootTableContext;
class Random;

class LootItemRandomChanceCondition : public LootItemCondition
{
public:
    LootItemRandomChanceCondition(float chance);

    virtual bool applies(Random* random, LootTableContext& context);

private:
    float chance;
};

#endif
