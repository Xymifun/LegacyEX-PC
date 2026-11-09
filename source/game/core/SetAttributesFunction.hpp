#ifndef GAME_CORE_SETATTRIBUTESFUNCTION_HPP
#define GAME_CORE_SETATTRIBUTESFUNCTION_HPP

#include <string>
#include <vector>

#include "game/core/AttributeModifier.hpp"
#include "game/item/LootItemFunction.hpp"
#include "game/util/RandomValueBounds.hpp"

class EquipmentSlot;

class SetAttributesFunction : public LootItemFunction
{
public:
    class Modifier
    {
    public:
        Modifier(const std::wstring& attributeName, const std::wstring& name, const int& operation,
                 RandomValueBounds amount, std::vector<EquipmentSlot*> slots, eMODIFIER_ID id);
    };

    SetAttributesFunction(std::vector<LootItemCondition*>* conditions, std::vector<Modifier*>* modifiers);

    virtual not_null_ptr<ItemInstance> apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                             LootTableContext& context);

protected:
    std::vector<Modifier*>* modifiers;
};

#endif
