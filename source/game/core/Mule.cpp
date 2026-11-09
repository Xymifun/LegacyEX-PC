#include "game/core/Mule.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"

Mule::Mule(Level* level) : AbstractChestedHorse(level)
{
    fjDerivedCtorCalls();
}

Entity* Mule::create(Level* level)
{
    return new Mule(level);
}

void Mule::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractChestedHorse::addDataWalkers(fixer, EntityType::MULE);
}

ResourceLocation* Mule::getDefaultLootTable()
{
    return &BuiltInLootTables::MULE;
}

const SoundEvent* Mule::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::MULE_AMBIENT;
}

const SoundEvent* Mule::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::MULE_DEATH;
}

const SoundEvent* Mule::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::MULE_HURT;
}

void Mule::playChestEquipsSound()
{
    playSound(SoundEvents::MULE_CHEST, 1.0f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
}

unsigned int Mule::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_bAllowedToInteract)
    {
        int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
        bool horseFood = itemId == BlockId::HAY_BLOCK || itemId == ItemId::APPLE || itemId == ItemId::WHEAT
                         || itemId == ItemId::GOLDEN_APPLE || itemId == ItemId::SUGAR
                         || itemId == ItemId::GOLDEN_CARROT;

        if (itemId == ItemId::SPAWN_EGG)
        {
            if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) != EntityType::MULE)
            {
                if (isLeashedToPlayer(tooltipData.m_pPlayer))
                    return IDS_TOOLTIPS_UNLEASH;
            }
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

            if (horseFood && getHealth() < getMaxHealth())
                return IDS_TOOLTIPS_HEAL;

            if (getPassengers()->empty())
                return IDS_TOOLTIPS_MOUNT;
        }
    }
    return IDS_NONE;
}

bool Mule::HideUseTooltips()
{
    return true;
}

unsigned long long Mule::GetType()
{
    return EntityType::MULE;
}
