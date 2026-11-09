#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/item/ItemId.hpp"

SkeletonHorse::SkeletonHorse(Level* level) : AbstractHorse(level), mSkeletonTrapGoal(this), mTrap(false), mTrapTime(0)
{
    fjDerivedCtorCalls();
}

Entity* SkeletonHorse::create(Level* level)
{
    return new SkeletonHorse(level);
}

void SkeletonHorse::setTrap(bool trap)
{
    if (mTrap == trap)
    {
        return;
    }

    mTrap = trap;
    if (mTrap)
    {
        mGoalSelector.addGoal(1, &mSkeletonTrapGoal, false);
    }
    else
    {
        mGoalSelector.removeGoal(&mSkeletonTrapGoal);
    }
}

void SkeletonHorse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(15.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
    getAttribute(JUMP_STRENGTH)->setBaseValue(generateRandomJumpStrength());
}

void SkeletonHorse::registerGoals()
{
    AbstractHorse::registerGoals();
    mGoalSelector.removeGoal(mFloatGoal);
    mGoalSelector.removeGoal(mWaterAvoidingRandomStrollGoal);
    mGoalSelector.addGoal(6, new SinkInWaterRandomStrollGoal(this, 0.7), true);
}

void SkeletonHorse::playUnderwaterSound(const BlockPos& pos, Block* block)
{
    if (mOnGround)
    {
        if (isVehicle())
        {
            ++mGallopSoundCounter;
            if (mGallopSoundCounter > 5 && mGallopSoundCounter % 3 == 0)
            {
                playSound(SoundEvents::SKELETON_HORSE_GALLOP_WATER, 0.4f, 1.0f);
            }
            else if (mGallopSoundCounter <= 5)
            {
                playSound(SoundEvents::SKELETON_HORSE_STEP_WATER, 0.4f, 1.0f);
            }
        }
        else
        {
            playSound(SoundEvents::SKELETON_HORSE_STEP_WATER, 0.4f, 1.0f);
        }
    }
    Entity::playUnderwaterSound(pos, block);
}

void SkeletonHorse::playGallopSound(const SoundType* sound)
{
    playSound(isInWater() ? SoundEvents::SKELETON_HORSE_GALLOP_WATER : SoundEvents::HORSE_GALLOP,
              sound->getVolume() * 0.15f, sound->getPitch());
}

const SoundEvent* SkeletonHorse::getJumpSound()
{
    return isInWater() ? SoundEvents::SKELETON_HORSE_JUMP_WATER : SoundEvents::HORSE_JUMP;
}

const SoundEvent* SkeletonHorse::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return isInWater() ? SoundEvents::SKELETON_HORSE_AMBIENT_WATER : SoundEvents::SKELETON_HORSE_AMBIENT;
}

const SoundEvent* SkeletonHorse::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::SKELETON_HORSE_DEATH;
}

const SoundEvent* SkeletonHorse::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::SKELETON_HORSE_HURT;
}

EMobType SkeletonHorse::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

double SkeletonHorse::getRideHeight()
{
    return Entity::getRideHeight() - 0.1875;
}

ResourceLocation* SkeletonHorse::getDefaultLootTable()
{
    return &BuiltInLootTables::SKELETON_HORSE;
}

boost::shared_ptr<AgableMob> SkeletonHorse::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<SkeletonHorse>(new SkeletonHorse(mLevel));
}

bool SkeletonHorse::isTamed()
{
    return true;
}

bool SkeletonHorse::isTrap()
{
    return mTrap;
}

void SkeletonHorse::aiStep()
{
    AbstractHorse::aiStep();
    if (isTrap())
    {
        if (mTrapTime++ >= 18000)
        {
            remove();
        }
    }
}

void SkeletonHorse::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractHorse::addDataWalkers(fixer, EntityType::SKELETON_HORSE);
}

void SkeletonHorse::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractHorse::addAdditonalSaveData(tag);
    tag->putBoolean(std::wstring(L"SkeletonTrap"), isTrap());
    tag->putInt(std::wstring(L"SkeletonTrapTime"), mTrapTime);
}

void SkeletonHorse::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractHorse::readAdditionalSaveData(tag);
    setTrap(tag->getBoolean(std::wstring(L"SkeletonTrap")));
    mTrapTime = tag->getInt(std::wstring(L"SkeletonTrapTime"));
}

bool SkeletonHorse::breathesWater()
{
    return true;
}

bool SkeletonHorse::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool notEmpty = !item->isEmpty();

    if (notEmpty && item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);

    if (!isTamed() && notEmpty)
        return false;

    if (isLeashedToPlayer(player))
        return false;

    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);

    if (player->isSneaking())
    {
        openInventory(player);
        return true;
    }

    if (isVehicle())
        return AbstractHorse::mobInteract(player, hand);

    if (notEmpty)
    {
        if (item->getItem() == Items::saddle && !isSaddled())
        {
            openInventory(player);
            return true;
        }
        if (item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand))
            return true;
    }

    doPlayerRide(player);
    return true;
}

unsigned int SkeletonHorse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    unsigned int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    if (!tooltipData.m_bAllowedToInteract)
        return IDS_NONE;

    if (hasPassenger(EntityType::SKELETON))
    {
        if (!tooltipData.m_bAllowedToInteract)
            return IDS_NONE;
        if (isLeashedToPlayer(tooltipData.m_pPlayer))
            return IDS_TOOLTIPS_UNLEASH;
        if (itemId != ItemId::LEAD)
            return IDS_NONE;
        return isLeashed() ? IDS_NONE : IDS_TOOLTIPS_LEASH;
    }

    bool horseArmor = itemId >= ItemId::IRON_HORSE_ARMOR
                      && (itemId <= ItemId::DIAMOND_HORSE_ARMOR || itemId == ItemId::LEATHER_HORSE_ARMOR);

    if (itemId == ItemId::SPAWN_EGG)
    {
        if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::SKELETON_HORSE)
            return IDS_TOOLTIPS_PLACE;
        if (isLeashedToPlayer(tooltipData.m_pPlayer))
            return IDS_TOOLTIPS_UNLEASH;
        return IDS_NONE;
    }

    if (isLeashedToPlayer(tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;

    if (itemId == ItemId::LEAD)
        return isLeashed() ? IDS_NONE : IDS_TOOLTIPS_LEASH;

    if (itemId == ItemId::NAME_TAG
        && !(tooltipData.m_pPlayer->isSneaking() && isTamed() && !hasPassenger(EntityType::SKELETON)))
    {
        if (tooltipData.m_pItemInstance->hasCustomHoverName())
        {
            if (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0)
                return IDS_TOOLTIPS_NAME;
            return IDS_NONE;
        }
        if (!isTamed())
            return IDS_NONE;
        return getPassengers()->empty() ? IDS_TOOLTIPS_MOUNT : IDS_NONE;
    }

    if (isBaby())
        return IDS_NONE;

    if (!isTamed())
    {
        if (!tooltipData.m_pItemInstance->isEmpty() || isVehicle() || !getPassengers()->empty())
            return IDS_NONE;
        return IDS_TOOLTIPS_TAME;
    }

    if (!hasPassenger(EntityType::SKELETON)
        && (tooltipData.m_pPlayer->isSneaking() && (!isVehicle() || hasPassenger(tooltipData.m_pPlayer)) && isTamed()
            || itemId == ItemId::SADDLE && !isSaddled() || wearsArmor() && horseArmor))
        return IDS_TOOLTIPS_OPEN;

    if (getPassengers()->empty())
        return IDS_TOOLTIPS_MOUNT;

    return IDS_NONE;
}

unsigned long long SkeletonHorse::GetType()
{
    return EntityType::SKELETON_HORSE;
}
