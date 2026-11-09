#include "game/item/TridentLoyaltyEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/Enchantments.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/TridentRiptideEnchantment.hpp"

TridentLoyaltyEnchantment::TridentLoyaltyEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::TRIDENT, slots)
{
    setDescriptionId((int)IDS_ENCHANTMENT_LOYALTY);
}

TridentLoyaltyEnchantment::~TridentLoyaltyEnchantment()
{
}

int TridentLoyaltyEnchantment::getMinCost(int level)
{
    return level * 7 + 5;
}

int TridentLoyaltyEnchantment::getMaxCost(int level)
{
    return 50;
}

int TridentLoyaltyEnchantment::getMaxLevel()
{
    return 3;
}

bool TridentLoyaltyEnchantment::checkCompatibility(const Enchantment* other) const
{
    if (other != NULL && dynamic_cast<const TridentRiptideEnchantment*>(other) != NULL)
    {
        return false;
    }

    return Enchantment::checkCompatibility(other);
}
