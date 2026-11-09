#include "game/core/Endermite.hpp"
#include "game/core/EntityType.hpp"

#include <string>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/core/EnderMan.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/PIXNamedEvents.hpp"

Endermite::Endermite(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    mLifetime = 0;
    mPlayerSpawned = false;
    mExperienceReward = 3;
    setSize(0.4f, 0.3f);
    mCategories |= EntityCategory::eEntityCategory_Arthropod;
}

unsigned long long Endermite::GetType()
{
    return EntityType::ENDERMITE;
}

float Endermite::getEyeHeight()
{
    return 0.1f;
}

void Endermite::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(3, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(EnderMan), true, false, false), true);
}

void Endermite::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(2.0);
}

bool Endermite::makeStepSound()
{
    return true;
}

const SoundEvent* Endermite::getAmbientSound()
{
    return SoundEvents::SILVERFISH_AMBIENT;
}

const SoundEvent* Endermite::getHurtSound(const DamageSource*)
{
    return SoundEvents::SILVERFISH_HURT;
}

const SoundEvent* Endermite::getDeathSound()
{
    return SoundEvents::SILVERFISH_DEATH;
}

void Endermite::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::SILVERFISH_STEP, 0.15f, 1.0f);
}

ResourceLocation* Endermite::getDefaultLootTable()
{
    return &BuiltInLootTables::ENDERMITE;
}

void Endermite::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::ENDERMITE);
}

void Endermite::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mLifetime = tag->getInt(std::wstring(L"Lifetime"));
    mPlayerSpawned = tag->getBoolean(std::wstring(L"PlayerSpawned"));
}

void Endermite::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"Lifetime"), mLifetime);
    tag->putBoolean(std::wstring(L"PlayerSpawned"), mPlayerSpawned);
}

void Endermite::tick()
{
    PIXBeginNamedEvent(0.0f, "Endermite tick");
    mYBodyRot = mYRot;
    Monster::tick();
    PIXEndNamedEvent();
}

void Endermite::setYBodyRot(float yBodyRot)
{
    mYRot = yBodyRot;
    LivingEntity::setYBodyRot(yBodyRot);
}

double Endermite::getRidingHeight()
{
    return 0.1;
}

void Endermite::setPlayerSpawned(bool playerSpawned)
{
    mPlayerSpawned = playerSpawned;
}

void Endermite::aiStep()
{
    Monster::aiStep();
    if (mLevel->mIsClientSide)
    {
        for (int i = 0; i < 2; ++i)
        {
            mLevel->addParticle(PARTICLE_ENDER, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                mY + mRandom->nextDouble() * mBbHeight, mZ + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                (mRandom->nextDouble() - 0.5) * 2.0, -mRandom->nextDouble(),
                                (mRandom->nextDouble() - 0.5) * 2.0, arrayWithLength<int>());
        }
    }
    else
    {
        if (!isPersistenceRequired())
            ++mLifetime;
        if (mLifetime >= 2400)
            remove();
    }
}

bool Endermite::isDarkEnoughToSpawn()
{
    return true;
}

bool Endermite::checkSpawnRules()
{
    if (Monster::checkSpawnRules())
    {
        boost::shared_ptr<Player> player = mLevel->getNearestPlayer(shared_from_this(), 5.0, -1.0);
        return player == NULL;
    }
    return false;
}

EMobType Endermite::getMobType()
{
    return MOB_TYPE_ARTHROPOD;
}

Entity* Endermite::create(Level* level)
{
    return new Endermite(level);
}
