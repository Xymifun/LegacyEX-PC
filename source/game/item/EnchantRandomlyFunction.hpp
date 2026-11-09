#ifndef GAME_ITEM_ENCHANTRANDOMLYFUNCTION_HPP
#define GAME_ITEM_ENCHANTRANDOMLYFUNCTION_HPP

#include <vector>

#include "game/item/LootItemFunction.hpp"

class Enchantment;
class LootItemCondition;
class LootTableContext;
class Random;
class ItemInstance;

class EnchantRandomlyFunction : public LootItemFunction
{
public:
    EnchantRandomlyFunction(std::vector<LootItemCondition*>* conditions, const std::vector<Enchantment*>& enchantments);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

private:
    std::vector<Enchantment*> enchantments;
    std::vector<Enchantment*> possibleEnchantments;
};

#endif
