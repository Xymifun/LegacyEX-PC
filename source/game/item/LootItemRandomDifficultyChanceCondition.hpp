#ifndef GAME_ITEM_LOOTITEMRANDOMDIFFICULTYCHANCECONDITION_HPP
#define GAME_ITEM_LOOTITEMRANDOMDIFFICULTYCHANCECONDITION_HPP

#include "game/core/LootItemCondition.hpp"

class LootTableContext;
class Random;

class LootItemRandomDifficultyChanceCondition : public LootItemCondition
{
public:
    LootItemRandomDifficultyChanceCondition(float peacefulChance, float easyChance, float normalChance,
                                            float hardChance);

    virtual bool applies(Random* random, LootTableContext& context);

private:
    float chances[4];
};

#endif
