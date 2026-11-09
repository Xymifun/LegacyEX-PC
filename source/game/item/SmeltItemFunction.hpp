#ifndef GAME_ITEM_SMELTITEMFUNCTION_HPP
#define GAME_ITEM_SMELTITEMFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class SmeltItemFunction : public LootItemFunction
{
public:
    SmeltItemFunction(std::vector<LootItemCondition*>* conditions);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);
};

#endif
