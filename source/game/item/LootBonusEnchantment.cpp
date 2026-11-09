#include "game/item/LootBonusEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/Enchantments.hpp"

LootBonusEnchantment::LootBonusEnchantment(const Rarity* rarity, const EnchantmentCategory* category,
                                           arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, category, slots)
{
    if (category == EnchantmentCategory::DIGGER)
    {
        setDescriptionId(IDS_ENCHANTMENT_LOOT_BONUS_DIGGER);
    }
    else if (category == EnchantmentCategory::FISHING_ROD)
    {
        setDescriptionId(IDS_ENCHANTMENT_LOOT_BONUS_FISHING_ROD);
    }
    else
    {
        setDescriptionId(IDS_ENCHANTMENT_LOOT_BONUS);
    }
}

LootBonusEnchantment::~LootBonusEnchantment()
{
}

int LootBonusEnchantment::getMinCost(int level)
{
    return level * 9 + 6;
}

int LootBonusEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int LootBonusEnchantment::getMaxLevel()
{
    return 3;
}

bool LootBonusEnchantment::checkCompatibility(const Enchantment* other) const
{
    return Enchantment::checkCompatibility(other) && other != Enchantments::SILK_TOUCH;
}
