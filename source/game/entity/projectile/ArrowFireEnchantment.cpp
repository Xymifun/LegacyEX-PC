#include "game/entity/projectile/ArrowFireEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

namespace
{
const int ARROW_FIRE_DESCRIPTION_ID = IDS_ENCHANTMENT_ARROW_FIRE;
}

ArrowFireEnchantment::ArrowFireEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(ARROW_FIRE_DESCRIPTION_ID);
}

int ArrowFireEnchantment::getMinCost(int level)
{
    return 20;
}

int ArrowFireEnchantment::getMaxCost(int level)
{
    return 50;
}

int ArrowFireEnchantment::getMaxLevel()
{
    return 1;
}

ArrowFireEnchantment::~ArrowFireEnchantment()
{
}
