#include "game/entity/projectile/ArrowInfiniteEnchantment.hpp"
#include "game/core/StringIds.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/MendingEnchantment.hpp"

static const int ARROW_INFINITY_DESCRIPTION_ID = static_cast<int>(IDS_ENCHANTMENT_ARROW_INFINITE);

ArrowInfiniteEnchantment::ArrowInfiniteEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(ARROW_INFINITY_DESCRIPTION_ID);
}

int ArrowInfiniteEnchantment::getMinCost(int)
{
    return 20;
}

int ArrowInfiniteEnchantment::getMaxCost(int)
{
    return 50;
}

int ArrowInfiniteEnchantment::getMaxLevel()
{
    return 1;
}

bool ArrowInfiniteEnchantment::checkCompatibility(const Enchantment* other) const
{
    if (other != NULL && dynamic_cast<const MendingEnchantment*>(other) != NULL)
    {
        return false;
    }
    return Enchantment::checkCompatibility(other);
}

ArrowInfiniteEnchantment::~ArrowInfiniteEnchantment()
{
}
