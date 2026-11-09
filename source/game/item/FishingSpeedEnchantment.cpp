#include "game/item/FishingSpeedEnchantment.hpp"
#include "game/core/StringIds.hpp"

FishingSpeedEnchantment::FishingSpeedEnchantment(const Rarity* rarity, const EnchantmentCategory* category,
                                                 arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, category, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_FISHING_SPEED);
}

FishingSpeedEnchantment::~FishingSpeedEnchantment()
{
}

int FishingSpeedEnchantment::getMinCost(int level)
{
    return level * 9 + 6;
}

int FishingSpeedEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int FishingSpeedEnchantment::getMaxLevel()
{
    return 3;
}
