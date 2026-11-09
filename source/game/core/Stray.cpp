#include "game/core/Stray.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/RestrictSunGoal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Wolf.hpp"
#include "game/world/generation/Player.hpp"

Stray::Stray(Level* level) : AbstractSkeleton(level)
{
    Mob::fjDerivedCtorCalls();
}

Entity* Stray::create(Level* level)
{
    return new Stray(level);
}

void Stray::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::STRAY);
}

bool Stray::checkSpawnRules()
{
    return Monster::checkSpawnRules() && mLevel->canSeeSky(BlockPos(this));
}

void Stray::registerGoals()
{
    AbstractSkeleton::registerGoals();
    mGoalSelector.addGoal(2, new RestrictSunGoal(this), true);
    mGoalSelector.addGoal(4, new AvoidEntityGoal(this, typeid(Wolf), 6.0f, 1.0, 1.2), true);
}

ResourceLocation* Stray::getDefaultLootTable()
{
    return &BuiltInLootTables::STRAY;
}

ResourceLocation* Stray::getDefaultGearTable()
{
    return &BuiltInLootTables::SKELETON_GEAR;
}

const SoundEvent* Stray::getAmbientSound()
{
    return SoundEvents::STRAY_AMBIENT;
}

const SoundEvent* Stray::getHurtSound(const DamageSource* source)
{
    return SoundEvents::STRAY_HURT;
}

const SoundEvent* Stray::getDeathSound()
{
    return SoundEvents::STRAY_DEATH;
}

const SoundEvent* Stray::getStepSound()
{
    return SoundEvents::STRAY_STEP;
}

boost::shared_ptr<Arrow> Stray::getArrow(float distanceFactor)
{
    boost::shared_ptr<Arrow> arrow = AbstractSkeleton::getArrow(distanceFactor);
    if (eTYPE_DERIVED_FROM(EntityType::TIPPABLE_ARROW, arrow->GetType()))
        boost::static_pointer_cast<TippableArrow>(arrow)->addEffect(
            new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 600));
    return arrow;
}

unsigned long long Stray::GetType()
{
    return EntityType::STRAY;
}
