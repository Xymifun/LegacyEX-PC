#ifndef GAME_ITEM_LOOTINGENCHANTFUNCTION_HPP
#define GAME_ITEM_LOOTINGENCHANTFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class LootingEnchantFunction : public LootItemFunction
{
public:
    LootingEnchantFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds count, int limit);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    RandomValueBounds count;
    int limit;
};

#endif
