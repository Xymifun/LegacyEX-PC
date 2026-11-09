#include "game/core/MushroomCow.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

MushroomCow::MushroomCow(Level* level) : Cow(level, false)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 1.4f);
    mSpawnableBlock = Blocks::mycelium;
}

Entity* MushroomCow::create(Level* level)
{
    return new MushroomCow(level);
}

void MushroomCow::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::MUSHROOM_COW);
}

bool MushroomCow::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (itemInstance->getItem() == Items::bowl && getAge() >= 0)
    {
        itemInstance->shrink(1);
        if (itemInstance->isEmpty())
        {
            player->setItemInHand(hand, not_null_ptr<ItemInstance>(new ItemInstance(Items::mushroomStew)));
            return true;
        }
        if (!player->mInventory->add(not_null_ptr<ItemInstance>(new ItemInstance(Items::mushroomStew))))
        {
            player->drop(not_null_ptr<ItemInstance>(new ItemInstance(Items::mushroomStew)), false);
            return true;
        }
    }

    if (itemInstance->getItem() == Items::shears && getAge() >= 0)
    {
        if (mLevel->canCreateMore(EntityType::COW, Level::eSpawnType_Breed))
        {
            mLevel->addParticle(PARTICLE_LARGE_EXPLODE, mX, mY + mBbHeight * 0.5f, mZ, 0.0, 0.0, 0.0,
                                arrayWithLength<int>());

            if (!mLevel->mIsClientSide)
            {
                remove();

                boost::shared_ptr<Cow> cow(new Cow(mLevel, true));
                if (isPassenger())
                {
                    cow->startRiding(getVehicle(), true);
                }
                if (isLeashed())
                {
                    cow->setLeashedTo(getLeashHolder(), true);
                }
                cow->moveTo(mX, mY, mZ, mYRot, mXRot);
                cow->setHealth(getHealth());
                cow->mYBodyRot = mYBodyRot;
                if (hasCustomName())
                {
                    cow->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());
                }
                mLevel->addEntity(cow);

                for (int i = 0; i < 5; ++i)
                {
                    mLevel->addEntity(boost::shared_ptr<Entity>(boost::shared_ptr<ItemEntity>(
                        new ItemEntity(mLevel, mX, mY + mBbHeight, mZ,
                                       not_null_ptr<ItemInstance>(new ItemInstance(Blocks::red_mushroom))))));
                }

                itemInstance->hurtAndBreak(1, player);
                playSound(SoundEvents::SHEEP_SHEAR, 1.0f, 1.0f);
                return true;
            }
            return true;
        }
        else
        {
            player->displayClientMessage(IDS_CANT_SHEAR_MOOSHROOM, true);
            return true;
        }
    }

    return Cow::mobInteract(player, hand);
}

bool MushroomCow::checkSpawnRules()
{
    BlockPos pos(shared_from_this());
    Block* block = mLevel->getBlockState(pos.below())->getBlock();
    return (block == Blocks::grass || block == Blocks::mycelium) && mLevel->getDaytimeRawBrightness(pos) > 8
           && PathfinderMob::checkSpawnRules();
}

boost::shared_ptr<AgableMob> MushroomCow::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<MushroomCow>(new MushroomCow(mLevel));
}

unsigned int MushroomCow::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case ItemId::BOWL:
        if (!isBaby())
            return IDS_TOOLTIPS_MILK;
        break;
    case ItemId::SHEARS:
        if (!isBaby())
            return IDS_TOOLTIPS_SHEAR;
        break;
    }
    return Cow::GetInteractTooltip(tooltipData);
}

unsigned long long MushroomCow::GetType()
{
    return EntityType::MUSHROOM_COW;
}
