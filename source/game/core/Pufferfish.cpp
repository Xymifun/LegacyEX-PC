#include "game/core/Pufferfish.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/System.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/RandomSwimmingGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/AABB.hpp"

Pufferfish::Pufferfish(Level* level)
    : Fish(level, true),
      mPuffState(PUFFED_STATE_SMALL),
      mNextPuffState(PUFFED_STATE_SMALL),
      mPuffTransitionTime(0),
      mPoisonDuration(200),
      mDamage(2),
      mSmallPuffRadius(1.5f),
      mLargePuffRadius(10.0f),
      mStingRange(0.2f),
      mStingSoundDelay(2)
{
    mSwimAmplitude = 0.02f;
    mSwimFrequency = 0.01f;
    mWaterValueC = 6.0f;
    setSize(0.8f, 0.8f);
    mLastStingSoundTime = 0;
    fjDerivedCtorCalls();
}

Entity* Pufferfish::create(Level* level)
{
    return new Pufferfish(level);
}

void Pufferfish::setNextPuffedState(PuffedState state, int delay)
{
    mNextPuffState = state;
    mPuffTransitionTime = delay;
}

void Pufferfish::damageNearbyMobs()
{
    if (mPuffState < PUFFED_STATE_LARGE)
        return;

    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        shared_from_this(), mBoundingBox->grow((double)mStingRange, (double)mStingRange, (double)mStingRange), NULL);
    bool stung = false;
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        bool isServerPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType());
        if (!isServerPlayer && !eTYPE_DERIVED_FROM(EntityType::MONSTER, entity->GetType()))
            continue;

        boost::shared_ptr<Mob> living = boost::static_pointer_cast<Mob>(entity);
        DamageSource* source = DamageSource::CreateMobAttack(boost::static_pointer_cast<Mob>(shared_from_this()));
        living->hurt(source, mDamage);
        living->addEffect(new MobEffectInstance(MobEffects::POISON, mPoisonDuration), boost::shared_ptr<Entity>());
        stung = true;
    }

    if (mLevel->mIsClientSide && stung)
    {
        int64_t now = System::processTimeInSeconds();
        if (mLastStingSoundTime + mStingSoundDelay < now)
        {
            mLastStingSoundTime = now;
            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::PUFFERFISH_STING, SoundSource::eSoundSource_Hostile, 1.0f,
                                   1.0f, false, 16.0f);
        }
    }
}

bool Pufferfish::validatePuffedTypes(boost::shared_ptr<Entity> entity)
{
    if (entity == NULL)
        return false;
    if (!eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
        return false;
    if (eTYPE_DERIVED_FROM(EntityType::FISH, entity->GetType()))
        return false;
    if (eTYPE_DERIVED_FROM(EntityType::SALMON, entity->GetType()))
        return false;
    if (eTYPE_DERIVED_FROM(EntityType::PUFFERFISH, entity->GetType()))
        return false;
    if (eTYPE_DERIVED_FROM(EntityType::TROPICAL_FISH, entity->GetType()))
        return false;
    return true;
}

void Pufferfish::checkForDangerousMobs()
{
    float radius = mPuffState == PUFFED_STATE_SMALL ? mSmallPuffRadius : mLargePuffRadius;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        shared_from_this(), mBoundingBox->grow((double)radius, (double)radius, (double)radius), NULL);
    bool foundDangerousMob = false;
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (!validatePuffedTypes(entity))
            continue;

        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        int isCreativePlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                               && boost::static_pointer_cast<Player>(entity)->mAbilities.mInstabuild;
        if (isCreativePlayer)
            continue;

        if (mPuffState == PUFFED_STATE_SMALL)
        {
            foundDangerousMob = true;
            setNextPuffedState(PUFFED_STATE_MEDIUM, 1);
        }
    }
    if (!foundDangerousMob && mPuffState == PUFFED_STATE_LARGE && mNextPuffState != PUFFED_STATE_MEDIUM)
        setNextPuffedState(PUFFED_STATE_MEDIUM, 200);
}

void Pufferfish::baseTick()
{
    Fish::baseTick();

    PuffedState nextState = mNextPuffState;
    PuffedState currentState = mPuffState;
    if (nextState != currentState)
    {
        --mPuffTransitionTime;
        if (mPuffTransitionTime < 1)
        {
            if (currentState == PUFFED_STATE_SMALL)
            {
                mPuffState = nextState;
                setNextPuffedState(PUFFED_STATE_LARGE, 1);
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::PUFFERFISH_BLOW_UP, SoundSource::eSoundSource_Hostile,
                                       0.6f, 1.0f, false, 16.0f);
            }
            else
            {
                mPuffState = nextState;
                if (currentState == PUFFED_STATE_LARGE)
                {
                    setNextPuffedState(PUFFED_STATE_SMALL, 20);
                }
                else if (nextState == PUFFED_STATE_LARGE)
                {
                    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
                }
                else
                {
                    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.3);
                    mLevel->playLocalSound(mX, mY, mZ, SoundEvents::PUFFERFISH_BLOW_OUT,
                                           SoundSource::eSoundSource_Hostile, 0.6f, 1.0f, false, 16.0f);
                }
            }
        }
    }

    if (mPuffState == PUFFED_STATE_LARGE)
    {
        damageNearbyMobs();
    }

    checkForDangerousMobs();
}

bool Pufferfish::hurt(const DamageSource* source, float damage)
{
    bool result = LivingEntity::hurt(source, damage);
    if (getHealth() > 0.0f && mPuffState == PUFFED_STATE_SMALL)
        setNextPuffedState(PUFFED_STATE_MEDIUM, 1);
    return result;
}

void Pufferfish::registerGoals()
{
    mGoalSelector.addGoal(3, new RandomSwimmingGoal(this, 1.0, 10, 16, 4), true);
}

void Pufferfish::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.3);
}

const SoundEvent* Pufferfish::getFlopSound() const
{
    return SoundEvents::PUFFERFISH_FLOP;
}

const SoundEvent* Pufferfish::getHurtSound(const DamageSource*)
{
    return SoundEvents::PUFFERFISH_HURT;
}

const SoundEvent* Pufferfish::getDeathSound()
{
    return SoundEvents::PUFFERFISH_DEATH;
}

ResourceLocation* Pufferfish::getDefaultLootTable()
{
    return &BuiltInLootTables::PUFFERFISH;
}

unsigned long long Pufferfish::GetType()
{
    return EntityType::PUFFERFISH;
}
