#include "game/entity/mob/Skeleton.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/Wolf.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/RestrictSunGoal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

Skeleton::Skeleton(Level* level) : AbstractSkeleton(level)
{
    Mob::fjDerivedCtorCalls();
}

Entity* Skeleton::create(Level* level)
{
    return new Skeleton(level);
}

void Skeleton::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SKELETON);
}

void Skeleton::registerGoals()
{
    AbstractSkeleton::registerGoals();
    mGoalSelector.addGoal(2, new RestrictSunGoal(this), true);
    mGoalSelector.addGoal(4, new AvoidEntityGoal(this, typeid(Wolf), 6.0f, 1.0, 1.2), true);
}

ResourceLocation* Skeleton::getDefaultLootTable()
{
    return &BuiltInLootTables::SKELETON;
}

ResourceLocation* Skeleton::getDefaultGearTable()
{
    return &BuiltInLootTables::SKELETON_GEAR;
}

const SoundEvent* Skeleton::getAmbientSound()
{
    return SoundEvents::SKELETON_AMBIENT;
}

const SoundEvent* Skeleton::getHurtSound(const DamageSource*)
{
    return SoundEvents::SKELETON_HURT;
}

const SoundEvent* Skeleton::getDeathSound()
{
    return SoundEvents::SKELETON_DEATH;
}

const SoundEvent* Skeleton::getStepSound()
{
    return SoundEvents::SKELETON_STEP;
}

void Skeleton::die(const DamageSource* source)
{
    AbstractSkeleton::die(source);

    if (source->getEntity() && eTYPE_DERIVED_FROM(EntityType::CREEPER, source->getEntity()->GetType()))
    {
        boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(source->getEntity());
        if (creeper->isPowered() && creeper->canDropMobsSkull())
        {
            creeper->increaseDroppedSkulls();
            spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 0)), 0.0f);
        }
    }
}

boost::shared_ptr<Arrow> Skeleton::getArrow(float distanceFactor)
{
    not_null_ptr<ItemInstance> item = getItemSlot(EquipmentSlot::OFFHAND);
    boost::shared_ptr<Arrow> arrow = AbstractSkeleton::getArrow(distanceFactor);
    if (item->getItem() == Items::tippedArrow && eTYPE_DERIVED_FROM(EntityType::TIPPABLE_ARROW, arrow->GetType()))
        boost::static_pointer_cast<TippableArrow>(arrow)->setEffectsFromItem(item);
    return arrow;
}

unsigned long long Skeleton::GetType()
{
    return EntityType::SKELETON;
}
