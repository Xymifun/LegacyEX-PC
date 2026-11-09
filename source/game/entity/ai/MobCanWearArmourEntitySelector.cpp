#include "game/entity/ai/MobCanWearArmourEntitySelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"

MobCanWearArmourEntitySelector::MobCanWearArmourEntitySelector(not_null_ptr<ItemInstance> itemInstance)
{
    mItemInstance = itemInstance;
}

bool MobCanWearArmourEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity->isAlive())
    {
        return false;
    }

    if (!eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);
    if (!livingEntity->getItemSlot(Mob::getEquipmentSlotForItem(mItemInstance))->isEmpty())
    {
        return false;
    }

    const bool isVillager = eTYPE_DERIVED_FROM(EntityType::VILLAGER, livingEntity->GetType());
    if (isVillager)
    {
        return false;
    }

    if (eTYPE_DERIVED_FROM(EntityType::ZOMBIE, livingEntity->GetType())
        || eTYPE_DERIVED_FROM(EntityType::ABSTRACT_SKELETON, livingEntity->GetType()))
    {
        return true;
    }

    if (eTYPE_DERIVED_FROM(EntityType::MOB, livingEntity->GetType()))
    {
        return boost::static_pointer_cast<Mob>(livingEntity)->canPickUpLoot();
    }

    const bool isArmorStand = eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, livingEntity->GetType());
    if (isArmorStand)
    {
        return true;
    }

    const bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, livingEntity->GetType());
    if (isPlayer)
    {
        return true;
    }

    return false;
}
