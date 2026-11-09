#include "game/entity/projectile/ArrowDamageEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

ArrowDamageEnchantment::ArrowDamageEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_ARROW_DAMAGE);
}

int ArrowDamageEnchantment::getMinCost(int level)
{
    return level * 10 - 9;
}

int ArrowDamageEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

int ArrowDamageEnchantment::getMaxLevel()
{
    return 5;
}

ArrowDamageEnchantment::~ArrowDamageEnchantment()
{
}
