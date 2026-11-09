#include "game/world/generation/VillagerInteractGoal.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"

VillagerInteractGoal::VillagerInteractGoal(Villager* villager)
    : InteractGoal(villager, typeid(Villager), 3.0f, 0.02f),
      mInteractionDelay(0),
      mVillager(villager)
{
}

void VillagerInteractGoal::start()
{
    InteractGoal::start();

    if (mVillager->hasExcessFood() && mLookAt.lock()
        && eTYPE_DERIVED_FROM(EntityType::VILLAGER, mLookAt.lock()->GetType())
        && boost::dynamic_pointer_cast<Villager>(mLookAt.lock())->wantsMoreFood())
        mInteractionDelay = 10;
    else
        mInteractionDelay = 0;
}

void VillagerInteractGoal::tick()
{
    InteractGoal::tick();

    if (mInteractionDelay <= 0 || --mInteractionDelay != 0)
        return;

    SimpleContainer* inventory = mVillager->getInventory();
    for (unsigned int slot = 0; slot < inventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = inventory->getItem(slot);
        not_null_ptr<ItemInstance> itemToThrow = ItemInstance::EMPTY;

        if (!item->isEmpty())
        {
            Item* itemType = item->getItem();
            if ((itemType == Items::bread || itemType == Items::potato || itemType == Items::carrot
                 || itemType == Items::beetroot)
                && item->getCount() > 3)
            {
                int amount = item->getCount() / 2;
                item->shrink(amount);
                itemToThrow = not_null_ptr<ItemInstance>(new ItemInstance(itemType, amount, item->getAuxValue()));
            }
            else if (itemType == Items::wheat && item->getCount() > 5)
            {
                int amount = (item->getCount() / 2) / 3 * 3;
                int loaves = amount / 3;
                item->shrink(amount);
                itemToThrow = not_null_ptr<ItemInstance>(new ItemInstance(Items::bread, loaves, 0));
            }
        }

        if (item->isEmpty())
            inventory->setItem(slot, ItemInstance::EMPTY);

        if (!itemToThrow->isEmpty())
        {
            const double throwY = mVillager->mY - 0.3f + mVillager->getEyeHeight();
            boost::shared_ptr<ItemEntity> entity(
                new ItemEntity(mVillager->mLevel, mVillager->mX, throwY, mVillager->mZ, itemToThrow));

            float speed = 0.3f;
            float yaw = mVillager->mYHeadRot;
            float pitch = mVillager->mXRot;
            entity->mXd = -Mth::sin(yaw * Mth::DEG_TO_RAD) * Mth::cos(pitch * Mth::DEG_TO_RAD) * speed;
            entity->mZd = Mth::cos(yaw * Mth::DEG_TO_RAD) * Mth::cos(pitch * Mth::DEG_TO_RAD) * speed;
            entity->mYd = -Mth::sin(pitch * Mth::DEG_TO_RAD) * speed + 0.1f;
            entity->setDefaultPickUpDelay();
            mVillager->mLevel->addEntity(entity);
            break;
        }
    }
}
