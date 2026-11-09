#include "game/item/DamageEnchantment.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/AxeItem.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

static const int MIN_COST[3] = {1, 5, 5};
static const int LEVEL_COST[3] = {11, 8, 8};
static const int LEVEL_COST_SPAN[3] = {20, 20, 20};
static const int DESCRIPTION_IDS[3] = {(int)IDS_ENCHANTMENT_DAMAGE_ALL, IDS_ENCHANTMENT_DAMAGE_UNDEAD,
                                       IDS_ENCHANTMENT_DAMAGE_ARTHROPODS};

DamageEnchantment::DamageEnchantment(const Rarity* rarity, int type, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::WEAPON, slots),
      mType(type)
{
}

int DamageEnchantment::getMaxLevel()
{
    return 5;
}

int DamageEnchantment::getMinCost(int level)
{
    return MIN_COST[mType] + (level - 1) * LEVEL_COST[mType];
}

int DamageEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + LEVEL_COST_SPAN[mType];
}

float DamageEnchantment::getDamageBonus(int level, const boost::shared_ptr<LivingEntity>& target)
{
    EMobType mobType = MOB_TYPE_UNDEFINED;
    if (target != NULL)
    {
        mobType = target->getMobType();
    }

    if (mType == 0)
    {
        return 1.0f + std::max(0.0f, level - 1.0f) * 0.5f;
    }

    if ((mType == 1 && mobType == MOB_TYPE_UNDEAD) || (mType == 2 && mobType == MOB_TYPE_ARTHROPOD))
    {
        return level * 2.5f;
    }

    return 0.0f;
}

bool DamageEnchantment::checkCompatibility(const Enchantment* other) const
{
    return dynamic_cast<const DamageEnchantment*>(other) == NULL;
}

int DamageEnchantment::getDescriptionId()
{
    return DESCRIPTION_IDS[mType];
}

bool DamageEnchantment::canEnchant(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    if (dynamic_cast<AxeItem*>(item) != NULL)
    {
        return true;
    }

    return Enchantment::canEnchant(itemInstance);
}

void DamageEnchantment::doPostAttack(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity> target,
                                     int level)
{
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
    {
        boost::shared_ptr<LivingEntity> livingTarget = boost::static_pointer_cast<LivingEntity>(target);

        if (mType == 2 && livingTarget->getMobType() == MOB_TYPE_ARTHROPOD)
        {
            int duration = attacker->getRandom()->nextInt(level * 10) + 20;
            livingTarget->addEffect(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, duration, 3),
                                    boost::shared_ptr<Entity>());
        }
    }
}

DamageEnchantment::~DamageEnchantment()
{
}
