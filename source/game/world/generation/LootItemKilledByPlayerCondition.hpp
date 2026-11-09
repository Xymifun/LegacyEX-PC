#ifndef GAME_WORLD_GENERATION_LOOTITEMKILLEDBYPLAYERCONDITION_HPP
#define GAME_WORLD_GENERATION_LOOTITEMKILLEDBYPLAYERCONDITION_HPP

#include "game/core/LootItemCondition.hpp"

class LootTableContext;
class Random;

class LootItemKilledByPlayerCondition : public LootItemCondition
{
public:
    LootItemKilledByPlayerCondition(bool inverse);

    virtual bool applies(Random* random, LootTableContext& context);

private:
    bool inverse;
};

#endif
