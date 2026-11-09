#ifndef GAME_ITEM_SETITEMDATAFUNCTION_HPP
#define GAME_ITEM_SETITEMDATAFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class SetItemDataFunction : public LootItemFunction
{
public:
    SetItemDataFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds value);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    RandomValueBounds value;
};

#endif
