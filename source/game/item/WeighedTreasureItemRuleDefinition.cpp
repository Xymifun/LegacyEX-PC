#include "game/item/WeighedTreasureItemRuleDefinition.hpp"

#include "game/core/WeighedTreasure.hpp"
#include "game/item/AddEnchantmentRuleDefinition.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

WeighedTreasureItemRuleDefinition::WeighedTreasureItemRuleDefinition()
{
    mMinCount = 0;
    mMaxCount = 0;
    mWeight = 0;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_minCount, &mMinCount);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_maxCount, &mMaxCount);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_weight, &mWeight);
}

WeighedTreasure* WeighedTreasureItemRuleDefinition::getItem()
{
    WeighedTreasure* treasure = new WeighedTreasure(Item::byId(mItemId), mData, mMinCount, mMaxCount, mWeight);
    not_null_ptr<ItemInstance> item = treasure->getItem();
    item->set4JData(m4JData);

    for (std::vector<AddEnchantmentRuleDefinition*>::iterator it = mEnchantments.begin(); it != mEnchantments.end();
         ++it)
    {
        (*it)->enchantItem(item);
    }

    return treasure;
}

ConsoleGameRules::EGameRuleType WeighedTreasureItemRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_WeighedTreasureItem;
}
