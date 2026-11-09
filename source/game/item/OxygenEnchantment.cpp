#include "game/item/OxygenEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

OxygenEnchantment::OxygenEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR_HEAD, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_OXYGEN);
}

OxygenEnchantment::~OxygenEnchantment()
{
}

int OxygenEnchantment::getMinCost(int level)
{
    return level * 10;
}

int OxygenEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 30;
}

int OxygenEnchantment::getMaxLevel()
{
    return 3;
}
