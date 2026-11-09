#include "game/item/TridentImpalerEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentCategory.hpp"

TridentImpalerEnchantment::TridentImpalerEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::TRIDENT, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_IMPALER);
}

TridentImpalerEnchantment::~TridentImpalerEnchantment()
{
}

int TridentImpalerEnchantment::getMinCost(int level)
{
    return level * 7 + 10;
}

int TridentImpalerEnchantment::getMaxCost(int level)
{
    return 50;
}

int TridentImpalerEnchantment::getMaxLevel()
{
    return 5;
}

float TridentImpalerEnchantment::getDamageBonus(int level, const boost::shared_ptr<LivingEntity>& target)
{
    if (target && target->isInWaterOrRain())
    {
        return (float)level * 2.5F;
    }

    return 0.0F;
}
