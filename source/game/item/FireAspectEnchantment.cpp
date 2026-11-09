#include "game/item/FireAspectEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

FireAspectEnchantment::FireAspectEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::WEAPON, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_FIRE);
}

int FireAspectEnchantment::getMinCost(int level)
{
    return level * 20 - 10;
}

int FireAspectEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int FireAspectEnchantment::getMaxLevel()
{
    return 2;
}

FireAspectEnchantment::~FireAspectEnchantment()
{
}
