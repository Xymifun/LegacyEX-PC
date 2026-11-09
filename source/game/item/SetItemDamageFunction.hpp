#ifndef GAME_ITEM_SETITEMDAMAGEFUNCTION_HPP
#define GAME_ITEM_SETITEMDAMAGEFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class SetItemDamageFunction : public LootItemFunction
{
public:
    SetItemDamageFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds damage);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    RandomValueBounds damage;
};

#endif
