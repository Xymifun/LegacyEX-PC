#ifndef GAME_ITEM_ENCHANTRANDOMEQUIPMENTFUNCTION_HPP
#define GAME_ITEM_ENCHANTRANDOMEQUIPMENTFUNCTION_HPP

#include "game/item/LootItemFunction.hpp"

class ItemInstance;
class LootItemCondition;
class LootTableContext;
class Random;

template <class T>
class not_null_ptr;

class EnchantRandomEquipmentFunction : public LootItemFunction
{
public:
    EnchantRandomEquipmentFunction(std::vector<LootItemCondition*>* conditions, float chance)
        : LootItemFunction(conditions),
          chance(chance)
    {
    }

    virtual ~EnchantRandomEquipmentFunction();

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    float chance;
};

#endif
