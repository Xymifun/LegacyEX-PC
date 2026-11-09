#ifndef GAME_ITEM_ENCHANTWITHLEVELSFUNCTION_HPP
#define GAME_ITEM_ENCHANTWITHLEVELSFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class EnchantWithLevelsFunction : public LootItemFunction
{
public:
    EnchantWithLevelsFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds levels,
                              bool allowTreasure);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    RandomValueBounds levels;
    bool allowTreasure;
};

#endif
