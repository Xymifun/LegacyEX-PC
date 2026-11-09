#include "game/core/Fish.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/FollowFlockLeaderGoal.hpp"
#include "game/entity/ai/RandomSwimmingGoal.hpp"
#include "game/entity/ai/SwimIdleGoal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/BucketItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

Fish::Fish(Level* level, bool deferredInit) : WaterAnimal(level)
{
    if (!deferredInit)
    {
        fjDerivedCtorCalls();
        setSize(0.6f, 0.2f);
    }
    _init();
    mRandom->setSeed(static_cast<long long>(getId() + 1));
}

void Fish::_init()
{
    mSwimTicks = 1;
    mSwimFrequency = 0.02f;
    mWaterValueC = 12.0f;
    mSwimAmplitude = 0.01f;
    mSwimSpeed = 1.0f;
    mSwimFlag = false;
    mSwimRotation = 0.0f;
    mSwimRotationOld = 0.0f;
    mMaxFlockSize = mRandom->nextInt(4, 8);
}

void Fish::baseTick()
{
    WaterAnimal::baseTick();
    mSwimRotationOld = mSwimRotation;
    mSwimRotation = mSwimRotation + 1.0f;
    mSwimRotation = mSwimRotation + 0.2f;
}

void Fish::registerGoals()
{
    mGoalSelector.addGoal(3, new FollowFlockLeaderGoal(this, true), true);
    mGoalSelector.addGoal(3, new SwimIdleGoal(this), true);
    mGoalSelector.addGoal(4, new RandomSwimmingGoal(this, 1.0, 10, 16, 4), true);
    mGoalSelector.addGoal(2, new AvoidEntityGoal(this, typeid(Player), 6.0f, 1.2, 1.4), true);
}

void Fish::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.0);
}

float Fish::getEyeHeight()
{
    return static_cast<double>(mBbHeight * 0.5f);
}

bool Fish::breathesWater()
{
    return true;
}

bool Fish::breathesAir()
{
    return false;
}

const SoundEvent* Fish::getSwimSound()
{
    return SoundEvents::FISH_SWIM;
}

const SoundEvent* Fish::getAmbientSound()
{
    return NULL;
}

const SoundEvent* Fish::getHurtSound(const DamageSource*)
{
    return SoundEvents::FISH_HURT;
}

const SoundEvent* Fish::getDeathSound()
{
    return SoundEvents::FISH_HURT;
}

float Fish::getSoundVolume()
{
    return 0.4f;
}

bool Fish::makeStepSound()
{
    return true;
}

ResourceLocation* Fish::getDefaultLootTable()
{
    return &BuiltInLootTables::ENTITY_FISH;
}

int Fish::getExperienceReward(boost::shared_ptr<Player>)
{
    return 0;
}

bool Fish::checkSpawnRules()
{
    return mY > 45.0 && mY < static_cast<double>(mLevel->getSeaLevel()) && WaterAnimal::checkSpawnRules();
}

bool Fish::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (item->getItem() == Items::bucket || item->getItem() == Items::waterBucket)
    {
        boost::shared_ptr<ItemInstance> result =
            static_cast<BucketItem*>(item->getItem())->collectFish(player, shared_from_this(), item);
        player->setItemInHand(hand, result);
        return true;
    }
    return Mob::mobInteract(player, hand);
}

unsigned int Fish::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    int itemId = Item::getId(holder.m_pItemInstance->getItem());
    if (isInWaterOrBubble() && (itemId == ItemId::BUCKET || itemId == ItemId::WATER_BUCKET))
        return IDS_TOOLTIPS_COLLECT;
    return Mob::GetInteractTooltip(holder);
}

bool Fish::canBeLeashed(const boost::shared_ptr<Player>&)
{
    return false;
}

unsigned long long Fish::GetType()
{
    return EntityType::FISH;
}

Entity* Fish::create(Level* level)
{
    return new Fish(level, false);
}
