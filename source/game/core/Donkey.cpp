#include "game/core/Donkey.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mule.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Blocks.hpp"

Donkey::Donkey(Level* level) : AbstractChestedHorse(level)
{
    fjDerivedCtorCalls();
}

void Donkey::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractChestedHorse::addDataWalkers(fixer, EntityType::DONKEY);
}

ResourceLocation* Donkey::getDefaultLootTable()
{
    return &BuiltInLootTables::DONKEY;
}

const SoundEvent* Donkey::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::DONKEY_AMBIENT;
}

const SoundEvent* Donkey::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::DONKEY_DEATH;
}

const SoundEvent* Donkey::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::DONKEY_HURT;
}

bool Donkey::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
    {
        return false;
    }

    if (eTYPE_DERIVED_FROM(EntityType::DONKEY, other->GetType())
        || eTYPE_DERIVED_FROM(EntityType::HORSE, other->GetType()))
    {
        return canParent() && boost::static_pointer_cast<AbstractHorse>(other)->canParent();
    }

    return false;
}

boost::shared_ptr<AgableMob> Donkey::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<AbstractHorse> child;
    if (eTYPE_DERIVED_FROM(EntityType::HORSE, partner->GetType()))
    {
        child = boost::shared_ptr<AbstractHorse>(new Mule(mLevel));
    }
    else
    {
        child = boost::shared_ptr<AbstractHorse>(new Donkey(mLevel));
    }
    setOffspringAttributes(partner, child);
    return child;
}

unsigned int Donkey::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bAllowedToInteract)
    {
        int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
        bool horseFood = itemId == BlockId::HAY_BLOCK || itemId == ItemId::APPLE || itemId == ItemId::WHEAT
                         || itemId == ItemId::GOLDEN_APPLE || itemId == ItemId::SUGAR
                         || itemId == ItemId::GOLDEN_CARROT;
        bool loveFood = itemId == ItemId::GOLDEN_CARROT || itemId == ItemId::GOLDEN_APPLE;

        if (itemId == ItemId::SPAWN_EGG)
        {
            if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::DONKEY)
                return IDS_TOOLTIPS_PLACE;
            if (isLeashedToPlayer(tooltipData.m_pPlayer))
                return IDS_TOOLTIPS_UNLEASH;
        }
        else
        {
            if (isLeashedToPlayer(tooltipData.m_pPlayer))
                return IDS_TOOLTIPS_UNLEASH;

            if (itemId == ItemId::LEAD)
            {
                if (!isLeashed())
                    return IDS_TOOLTIPS_LEASH;
                if (!getPassengers()->empty() || !isTamed())
                    return IDS_NONE;
                return IDS_TOOLTIPS_MOUNT;
            }

            if (itemId == ItemId::NAME_TAG && (!tooltipData.m_pPlayer->isSneaking() || !isTamed()))
            {
                if (tooltipData.m_pItemInstance->hasCustomHoverName()
                    && (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0))
                    return IDS_TOOLTIPS_NAME;
                if (!isTamed() || !getPassengers()->empty())
                    return IDS_NONE;
                return IDS_TOOLTIPS_MOUNT;
            }

            if (isBaby())
                return horseFood ? IDS_TOOLTIPS_FEED : IDS_NONE;

            if (!isTamed())
            {
                if (tooltipData.m_pItemInstance->isEmpty())
                {
                    if (getPassengers()->empty())
                        return IDS_TOOLTIPS_TAME;
                }
                else if (horseFood && itemId != BlockId::HAY_BLOCK)
                {
                    return IDS_TOOLTIPS_FEED;
                }
                return IDS_NONE;
            }

            if ((tooltipData.m_pPlayer->isSneaking() && (!isVehicle() || hasPassenger(tooltipData.m_pPlayer))
                 && isTamed())
                || (itemId == ItemId::SADDLE && !isSaddled()))
                return IDS_TOOLTIPS_OPEN;

            if (itemId == BlockId::CHEST && !hasChest())
                return IDS_TOOLTIPS_ATTACH;

            if (loveFood && !isInLove() && !isBaby() && getPassengers()->empty() && !isBreedCoolingDown())
                return IDS_TOOLTIPS_LOVEMODE;

            if (horseFood && getHealth() < getMaxHealth())
                return IDS_TOOLTIPS_HEAL;

            if (getPassengers()->empty())
                return IDS_TOOLTIPS_MOUNT;
        }
    }
    return IDS_NONE;
}

bool Donkey::HideUseTooltips()
{
    return true;
}

unsigned long long Donkey::GetType()
{
    return EntityType::DONKEY;
}

Entity* Donkey::create(Level* level)
{
    return new Donkey(level);
}
