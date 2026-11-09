#include "game/item/SweepingEdgeEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"

SweepingEdgeEnchantment::SweepingEdgeEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::WEAPON, slots)
{
}

float SweepingEdgeEnchantment::getSweepingDamageRatio(int level)
{
    return 1.0f - 1.0f / static_cast<float>(level + 1);
}

int SweepingEdgeEnchantment::getMaxLevel()
{
    return 3;
}

int SweepingEdgeEnchantment::getMinCost(int level)
{
    return level * 9 - 4;
}

int SweepingEdgeEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

int SweepingEdgeEnchantment::getDescriptionId()
{
    return IDS_ENCHANTMENT_DIGGING;
}

SweepingEdgeEnchantment::~SweepingEdgeEnchantment()
{
}
