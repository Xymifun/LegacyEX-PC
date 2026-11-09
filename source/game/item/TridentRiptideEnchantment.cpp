#include "game/item/TridentRiptideEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/TridentChannelingEnchantment.hpp"
#include "game/item/TridentLoyaltyEnchantment.hpp"

TridentRiptideEnchantment::TridentRiptideEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::TRIDENT, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_RIPTIDE);
}

TridentRiptideEnchantment::~TridentRiptideEnchantment()
{
}

int TridentRiptideEnchantment::getMinCost(int level)
{
    return level * 7 + 10;
}

int TridentRiptideEnchantment::getMaxCost(int)
{
    return 50;
}

int TridentRiptideEnchantment::getMaxLevel()
{
    return 3;
}

bool TridentRiptideEnchantment::checkCompatibility(const Enchantment* other) const
{
    if (other != NULL
        && (dynamic_cast<const TridentChannelingEnchantment*>(other) != NULL
            || dynamic_cast<const TridentLoyaltyEnchantment*>(other) != NULL))
    {
        return false;
    }

    return Enchantment::checkCompatibility(other);
}
