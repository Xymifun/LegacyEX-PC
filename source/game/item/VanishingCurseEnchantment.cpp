#include "game/item/VanishingCurseEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

VanishingCurseEnchantment::VanishingCurseEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ALL, slots)
{
    setDescriptionId((int)IDS_ENCHANTMENT_CURSE_OF_VANISHING);
}

VanishingCurseEnchantment::~VanishingCurseEnchantment()
{
}

int VanishingCurseEnchantment::getMinCost(int level)
{
    return 25;
}

int VanishingCurseEnchantment::getMaxCost(int level)
{
    return 50;
}

int VanishingCurseEnchantment::getMaxLevel()
{
    return 1;
}

bool VanishingCurseEnchantment::isTreasureOnly()
{
    return true;
}

bool VanishingCurseEnchantment::isCurse()
{
    return true;
}
