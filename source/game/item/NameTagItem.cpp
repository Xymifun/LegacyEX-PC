#include "game/item/NameTagItem.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"

NameTagItem::NameTagItem()
{
}

bool NameTagItem::interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand)
{
    if (!itemInstance->hasCustomHoverName() || Entity::instanceof(target, EntityType::PLAYER)
        || itemInstance->getHoverName() == target->getCustomName())
    {
        return false;
    }

    target->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                          itemInstance->wasCreatedByRestrictedPlayer());

    if (Entity::instanceof(target, EntityType::MOB))
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(target);
        mob->setPersistenceRequired(true);

        if (mob->isPassenger() && eTYPE_DERIVED_FROM(EntityType::MOB, mob->getVehicle()->GetType()))
        {
            boost::shared_ptr<Mob> vehicle = boost::static_pointer_cast<Mob>(mob->getVehicle());
            vehicle->setPersistenceRequired(true);
        }
    }

    itemInstance->shrink(1);
    return true;
}
