#include "game/item/DiggingEnchantment.hpp"
#include "game/core/StringIds.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

DiggingEnchantment::DiggingEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::DIGGER, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_DIGGING);
}

DiggingEnchantment::~DiggingEnchantment()
{
}

int DiggingEnchantment::getMinCost(int level)
{
    return level * 10 - 9;
}

int DiggingEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int DiggingEnchantment::getMaxLevel()
{
    return 5;
}

bool DiggingEnchantment::canEnchant(not_null_ptr<ItemInstance> item)
{
    if (item->getItem() == Items::shears)
        return true;
    return Enchantment::canEnchant(item);
}
