#include "game/item/ThornsEnchantment.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/DamageSource.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

ThornsEnchantment::ThornsEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR_CHEST, slots)
{
    setDescriptionId((int)IDS_ENCHANTMENT_THORNS);
}

int ThornsEnchantment::getMinCost(int level)
{
    return level * 20 - 10;
}

int ThornsEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int ThornsEnchantment::getMaxLevel()
{
    return 3;
}

bool ThornsEnchantment::canEnchant(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    if (dynamic_cast<ArmorItem*>(item) != NULL)
    {
        return true;
    }

    return Enchantment::canEnchant(itemInstance);
}

bool ThornsEnchantment::shouldHit(int level, Random* random)
{
    if (level > 0)
    {
        return random->nextFloat() < 0.15f * (float)level;
    }

    return false;
}

int ThornsEnchantment::getDamage(int level, Random* random)
{
    if (level > 10)
    {
        return level - 10;
    }

    return random->nextInt(4) + 1;
}

void ThornsEnchantment::doPostHurt(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<Entity> attacker,
                                   int level)
{
    Random* random = target->getRandom();
    boost::shared_ptr<ItemInstance> item = EnchantmentHelper::getRandomItemWith(Enchantments::THORNS, target);

    if (!item->isEmpty())
    {
        if (shouldHit(level, random))
        {
            if (attacker)
            {
                EntityDamageSource* damageSource = DamageSource::CreateThorns(target);
                attacker->hurt(damageSource, (float)getDamage(level, random));
                delete damageSource;
                attacker->playSound(SoundEvents::ENCHANT_THORNS_HIT, 0.5f, 1.0f);
            }
            item->hurtAndBreak(3, target);
        }
        else
        {
            item->hurtAndBreak(1, target);
        }
    }
}

ThornsEnchantment::~ThornsEnchantment()
{
}
