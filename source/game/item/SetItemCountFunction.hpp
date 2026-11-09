#ifndef GAME_ITEM_SETITEMCOUNTFUNCTION_HPP
#define GAME_ITEM_SETITEMCOUNTFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class SetItemCountFunction : public LootItemFunction
{
public:
    SetItemCountFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds count);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    RandomValueBounds count;
};

#endif
