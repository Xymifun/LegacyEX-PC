#include "game/entity/mob/LookAtEntityHint.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Boat.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/Tutorial.hpp"

LookAtEntityHint::LookAtEntityHint(eTutorial_Hint hint, Tutorial* tutorialValue, int description, int id,
                                   unsigned long long typeValue)
    : TutorialHint(hint, tutorialValue, description, HINT_LOOK_AT_ENTITY, true, true, -1),
      entityType(typeValue),
      entityId(id),
      entityPredicate(NULL)
{
}

LookAtEntityHint::~LookAtEntityHint()
{
}

not_null_ptr<ItemInstance> LookAtEntityHint::GetEntityAsItem(boost::shared_ptr<Entity> entity)
{
    switch (entity->GetType())
    {
    case EntityType::PAINTING:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::painting));
    case EntityType::ITEM_FRAME:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::itemFrame));
    case EntityType::LEASH_FENCE_KNOT_ENTITY:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::lead));
    case EntityType::MINECART_RIDEABLE:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::minecart));
    case EntityType::MINECART_CHEST:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::chestMinecart));
    case EntityType::MINECART_FURNACE:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::furnaceMinecart));
    case EntityType::MINECART_HOPPER:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::hopperMinecart));
    case EntityType::MINECART_TNT:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::tntMinecart));
    case EntityType::BOAT:
        return not_null_ptr<ItemInstance>(new ItemInstance(boost::static_pointer_cast<Boat>(entity)->getDropItem()));
    case EntityType::ARMOR_STAND:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::armorStand));
    case EntityType::ENDER_CRYSTAL:
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::endCrystal));
    }

    return ItemInstance::EMPTY;
}

unsigned int LookAtEntityHint::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (entityType == entity->GetType() && (entityPredicate == NULL || entityPredicate(entity)))
    {
        Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
        if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::ENDER_CRYSTAL, entity->GetType()))
        {
            not_null_ptr<ItemInstance> item = GetEntityAsItem(entity);
            if (item->isEmpty())
            {
                return 0;
            }

            details->titleId = item->getUseDescriptionId();
            if (item->getItem() != NULL)
            {
                details->itemDescription = item->getHoverName();
            }
            details->itemDescriptionId = item->getDescriptionId(-1);
            details->itemInstance = item;
            details->itemId = Item::VALIDgetId(item->getItem());
            details->itemAuxValue = item->getAuxValue();
            details->isItem = true;
            return tutorial->setMessage(this, details);
        }
        else
        {
            details->titleId = descriptionId;
            details->promptId = entityId;
            details->isItem = true;
            return tutorial->setMessage(this, details);
        }
    }
    return 0;
}
