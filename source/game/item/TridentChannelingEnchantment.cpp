#include "game/item/TridentChannelingEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/TridentRiptideEnchantment.hpp"

TridentChannelingEnchantment::TridentChannelingEnchantment(const Rarity* rarity,
                                                           arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::TRIDENT, slots)
{
    setDescriptionId((int)IDS_ENCHANTMENT_CHANNELING);
}

int TridentChannelingEnchantment::getMinCost(int level)
{
    return 25;
}

int TridentChannelingEnchantment::getMaxCost(int level)
{
    return 50;
}

int TridentChannelingEnchantment::getMaxLevel()
{
    return 1;
}

bool TridentChannelingEnchantment::checkCompatibility(const Enchantment* other) const
{
    if (other != NULL && dynamic_cast<const TridentRiptideEnchantment*>(other) != NULL)
    {
        return false;
    }

    return Enchantment::checkCompatibility(other);
}

TridentChannelingEnchantment::~TridentChannelingEnchantment()
{
}
