#include "game/item/AddEnchantmentRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

AddEnchantmentRuleDefinition::AddEnchantmentRuleDefinition() : GameRuleDefinition()
{
    mLevel = 0;
    mEnchantmentId = 0;

    GameRules_IntAttribute* enchantmentAttribute = addIntAttribute(ConsoleGameRules::eGameRuleAttr_enchantmentId,
                                                                   &mEnchantmentId);
    enchantmentAttribute->setMin(0)->setMax(0xff);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_enchantmentLevel, &mLevel)->setMin(0);
}

unsigned int AddEnchantmentRuleDefinition::enchantItem(not_null_ptr<ItemInstance> item)
{
    unsigned int enchanted = 0;
    if (!item->isEmpty())
    {
        bool isEnchantedBook = item->getItem() == Items::enchantedBook;
        if (isEnchantedBook)
        {
            EnchantedBookItem::addEnchantment(item, new EnchantmentInstance(Enchantment::byId(mEnchantmentId), mLevel));
        }
        else if (item->getItem()->isEnchantable(item))
        {
            Enchantment* enchantment = Enchantment::byId(mEnchantmentId);
            if (enchantment != NULL && enchantment->mCategory->canEnchant(item->getItem()))
            {
                item->enchant(enchantment, mLevel);
                enchanted = 1;
            }
        }
    }
    return enchanted;
}

ConsoleGameRules::EGameRuleType AddEnchantmentRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_AddEnchantment;
}
