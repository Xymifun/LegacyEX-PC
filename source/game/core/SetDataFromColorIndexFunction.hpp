#ifndef GAME_CORE_SETDATAFROMCOLORINDEXFUNCTION_HPP
#define GAME_CORE_SETDATAFROMCOLORINDEXFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

class SetDataFromColorIndexFunction : public LootItemFunction
{
public:
    SetDataFromColorIndexFunction(std::vector<LootItemCondition*>* conditions);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);
};

#endif
