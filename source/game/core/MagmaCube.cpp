#include "game/core/MagmaCube.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"

MagmaCube::MagmaCube(Level* level) : Slime(level)
{
    fjDerivedCtorCalls();
    mFireImmune = true;
}

Entity* MagmaCube::create(Level* level)
{
    return new MagmaCube(level);
}

void MagmaCube::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::MAGMA_CUBE);
}

void MagmaCube::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
}

void MagmaCube::defineSynchedData()
{
}

bool MagmaCube::checkSpawnRules()
{
    return mLevel->getDifficulty() != Difficulty::PEACEFUL && mLevel->isUnobstructed(mBoundingBox)
           && mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty()
           && !mLevel->containsAnyLiquid(mBoundingBox);
}

bool MagmaCube::checkSpawnObstruction()
{
    return mLevel->isUnobstructed(mBoundingBox, shared_from_this())
           && mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty()
           && !mLevel->containsAnyLiquid(mBoundingBox);
}

void MagmaCube::setSize(int size, bool reset)
{
    Slime::setSize(size, reset);
    getAttribute(SharedMonsterAttributes::ARMOR)->setBaseValue((double)(size * 3));
}

int MagmaCube::getLightColor()
{
    return 0xf000f0;
}

float MagmaCube::getBrightness()
{
    return 1.0f;
}

const ParticleType* MagmaCube::getParticleType()
{
    return PARTICLE_FLAME;
}

boost::shared_ptr<Slime> MagmaCube::createChild()
{
    return boost::shared_ptr<MagmaCube>(new MagmaCube(mLevel));
}

ResourceLocation* MagmaCube::getDefaultLootTable()
{
    return isTiny() ? BuiltInLootTables::EMPTY : &BuiltInLootTables::MAGMA_CUBE;
}

bool MagmaCube::isOnFire()
{
    return false;
}

bool MagmaCube::breathesLava()
{
    return true;
}

int MagmaCube::getJumpDelay()
{
    return Slime::getJumpDelay() * 4;
}

void MagmaCube::decreaseSquish()
{
    mTargetSquish *= 0.9f;
}

void MagmaCube::jumpFromGround()
{
    mYd = 0.42f + getSize() * 0.1f;
    mHasImpulse = true;
}

void MagmaCube::jumpInLava()
{
    mYd = 0.22f + getSize() * 0.05f;
    mHasImpulse = true;
}

void MagmaCube::causeFallDamage(float, float)
{
}

bool MagmaCube::isDealsDamage()
{
    return true;
}

int MagmaCube::getAttackDamage()
{
    return Slime::getAttackDamage() + 2;
}

const SoundEvent* MagmaCube::getHurtSound(const DamageSource*)
{
    return isTiny() ? SoundEvents::SMALL_MAGMACUBE_HURT : SoundEvents::MAGMACUBE_HURT;
}

const SoundEvent* MagmaCube::getDeathSound()
{
    return isTiny() ? SoundEvents::SMALL_MAGMACUBE_DEATH : SoundEvents::MAGMACUBE_DEATH;
}

const SoundEvent* MagmaCube::getSquishSound()
{
    return getSize() > 1 ? SoundEvents::SMALL_MAGMACUBE_SQUISH : SoundEvents::MAGMACUBE_SQUISH;
}

const SoundEvent* MagmaCube::getJumpSound()
{
    return SoundEvents::MAGMACUBE_JUMP;
}

unsigned long long MagmaCube::GetType()
{
    return EntityType::MAGMA_CUBE;
}
