#include "game/item/DigDurabilityEnchantment.hpp"

#include "game/core/StringIds.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

DigDurabilityEnchantment::DigDurabilityEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BREAKABLE, slots)
{
    setDescriptionId(IDS_ENCHANTMENT_DURABILITY);
}

DigDurabilityEnchantment::~DigDurabilityEnchantment()
{
}

int DigDurabilityEnchantment::getMinCost(int level)
{
    return level * 8 - 3;
}

int DigDurabilityEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int DigDurabilityEnchantment::getMaxLevel()
{
    return 3;
}

bool DigDurabilityEnchantment::canEnchant(not_null_ptr<ItemInstance> item)
{
    if (item->isDamageableItem())
        return true;
    return Enchantment::canEnchant(item);
}

bool DigDurabilityEnchantment::shouldIgnoreDurabilityDrop(not_null_ptr<ItemInstance> item, int level, Random* random)
{
    Item* held = item->getItem();
    if (held != 0 && dynamic_cast<ArmorItem*>(held) != 0 && random->nextFloat() < 0.6f)
        return false;
    return random->nextInt(level + 1) > 0;
}
