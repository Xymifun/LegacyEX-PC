#include "game/item/MendingEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

MendingEnchantment::MendingEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BREAKABLE, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_MENDING);
}

MendingEnchantment::~MendingEnchantment()
{
}

int MendingEnchantment::getMinCost(int level)
{
    return level * 25;
}

int MendingEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 50;
}

bool MendingEnchantment::isTreasureOnly()
{
    return true;
}

int MendingEnchantment::getMaxLevel()
{
    return 1;
}
