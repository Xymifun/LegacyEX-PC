#include "game/item/EnchantmentInstance.hpp"

#include "game/core/Rarity.hpp"
#include "game/item/Enchantment.hpp"

EnchantmentInstance::EnchantmentInstance(Enchantment* enchantment, int level)
    : WeighedRandomItem(enchantment->getRarity()->getWeight()),
      enchantment(enchantment),
      level(level)
{
}

EnchantmentInstance::~EnchantmentInstance()
{
}

EnchantmentInstance* EnchantmentInstance::copy()
{
    return new EnchantmentInstance(enchantment, level);
}
