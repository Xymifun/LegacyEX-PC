#include "game/entity/projectile/ArrowKnockbackEnchantment.hpp"

namespace
{
const int kArrowKnockbackDescriptionId = static_cast<int>(0x86d14a8dU);
}

#include "game/item/EnchantmentCategory.hpp"

ArrowKnockbackEnchantment::ArrowKnockbackEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(kArrowKnockbackDescriptionId);
}

int ArrowKnockbackEnchantment::getMinCost(int level)
{
    return level * 20 - 8;
}

int ArrowKnockbackEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 25;
}

int ArrowKnockbackEnchantment::getMaxLevel()
{
    return 2;
}

ArrowKnockbackEnchantment::~ArrowKnockbackEnchantment()
{
}
