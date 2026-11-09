#ifndef GAME_CORE_SETNBTFUNCTION_HPP
#define GAME_CORE_SETNBTFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"

class CompoundTag;

class SetNBTFunction : public LootItemFunction
{
public:
    SetNBTFunction(std::vector<LootItemCondition*>* conditions, CompoundTag* tag);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

private:
    CompoundTag* tag;
};

#endif
