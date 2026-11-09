#include "game/core/Ghast.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EServerSetting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Settings.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"

static const EntityDataAccessor<bool>* DATA_CHARGING = NULL;

Ghast::Ghast(Level* level) : FlyingMob(level)
{
    Mob::fjDerivedCtorCalls();
    _init();
    setSize(4.0f, 4.0f);
    mFireImmune = true;
    mExperienceReward = 5;
    delete mMoveControl;
    mMoveControl = new GhastMoveControl(this);
}

Entity* Ghast::create(Level* level)
{
    return new Ghast(level);
}

void Ghast::staticCtor()
{
    DATA_CHARGING = SynchedEntityData::defineId<bool>(EntityType::GHAST, EntityType::FLYING_MOB,
                                                      EntityDataSerializers::BOOLEAN);
}

void Ghast::_init()
{
    mExplosionPower = 1;
    mUnused = 0;
}

void Ghast::registerGoals()
{
    mGoalSelector.addGoal(5, new RandomFloatAroundGoal(this), true);
    mGoalSelector.addGoal(7, new GhastLookGoal(this), true);
    mGoalSelector.addGoal(7, new GhastShootFireballGoal(this), true);
    mTargetSelector.addGoal(1, new MobNearestPlayerTargetGoal(this), true);
}

bool Ghast::isCharging()
{
    return mEntityData->get(DATA_CHARGING);
}

void Ghast::setCharging(bool charging)
{
    mEntityData->set(DATA_CHARGING, charging, true);
}

int Ghast::getExplosionPower()
{
    return mExplosionPower;
}

bool Ghast::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    if (source->getDirectEntity()
        && eTYPE_DERIVED_FROM(EntityType::LARGE_FIREBALL, source->getDirectEntity()->GetType()))
    {
        if (source->getEntity() && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->getEntity()->GetType()))
        {
            LivingEntity::hurt(source, 1000.0f);
            boost::dynamic_pointer_cast<Player>(source->getEntity())
                ->awardStat(GenericStats::ghast(), GenericStats::param_ghast());
            return true;
        }
    }
    return LivingEntity::hurt(source, damage);
}

void Ghast::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_CHARGING, false);
}

void Ghast::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(100.0);
}

const SoundEvent* Ghast::getAmbientSound()
{
    return SoundEvents::GHAST_AMBIENT;
}
const SoundEvent* Ghast::getHurtSound(const DamageSource*)
{
    return SoundEvents::GHAST_SCREAM;
}
const SoundEvent* Ghast::getDeathSound()
{
    return SoundEvents::GHAST_DEATH;
}
ResourceLocation* Ghast::getDefaultLootTable()
{
    return &BuiltInLootTables::GHAST;
}
float Ghast::getSoundVolume()
{
    return 0.5f;
}

bool Ghast::checkSpawnRules()
{
    return mRandom->nextInt(20) == 0 && Mob::checkSpawnRules() && mLevel->getDifficulty() != Difficulty::PEACEFUL;
}

int Ghast::getMaxSpawnClusterSize()
{
    return 1;
}

void Ghast::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::GHAST);
}

void Ghast::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"ExplosionPower"), mExplosionPower);
}

void Ghast::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(std::wstring(L"ExplosionPower")))
        mExplosionPower = tag->getInt(std::wstring(L"ExplosionPower"));
}

float Ghast::getEyeHeight()
{
    return 2.6f;
}

void Ghast::tick()
{
    Mob::tick();
    if (!mLevel->mIsClientSide)
    {
        if (mLevel->getDifficulty() == Difficulty::PEACEFUL || Minecraft::GetInstance()->isTutorial())
            remove();
        else if (!gMinecraftServer->mSettings->getBoolean(eServerSetting_SpawnMonsters, true))
            remove();
    }
}

unsigned long long Ghast::GetType()
{
    return EntityType::GHAST;
}

Ghast::GhastLookGoal::GhastLookGoal(Ghast* ghast) : Goal(), mGhast(ghast)
{
    setRequiredControlFlags(2);
}
Ghast::GhastLookGoal::~GhastLookGoal()
{
}
bool Ghast::GhastLookGoal::canUse()
{
    return true;
}
void Ghast::GhastLookGoal::tick()
{
    if (mGhast->getTarget() == NULL)
    {
        mGhast->mYBodyRot = mGhast->mYRot = -(float)std::atan2(mGhast->mXd, mGhast->mZd) * Mth::RAD_TO_DEG;
    }
    else
    {
        boost::shared_ptr<LivingEntity> target = mGhast->getTarget();
        double distance = 64.0;
        if (target->distanceToSqr(mGhast->shared_from_this()) < distance * distance)
        {
            double dx = target->mX - mGhast->mX;
            double dz = target->mZ - mGhast->mZ;
            float rot = -(float)std::atan2(dx, dz) * Mth::RAD_TO_DEG;
            mGhast->mYBodyRot = mGhast->mYRot = rot;
        }
    }
}
Ghast::GhastMoveControl::GhastMoveControl(Ghast* ghast) : MoveControl(ghast), mGhast(ghast), mCountdown(0)
{
}
Ghast::GhastMoveControl::~GhastMoveControl()
{
}
bool Ghast::GhastMoveControl::canReach(double x, double y, double z, double distance)
{
    double dx = (x - mGhast->mX) / distance;
    double dy = (y - mGhast->mY) / distance;
    double dz = (z - mGhast->mZ) / distance;
    AABB* box = mGhast->mBoundingBox;
    for (int step = 1; (double)step < distance; ++step)
    {
        box = box->move(dx, dy, dz);
        if (mGhast->mLevel->getCollisionAABBs(mGhast->shared_from_this(), box, false, false, false)->size() > 0)
            return false;
    }
    return true;
}
void Ghast::GhastMoveControl::tick()
{
    if (mOperation != MOVE_TO)
        return;
    double dx = mWantedX - mGhast->mX;
    double dy = mWantedY - mGhast->mY;
    double dz = mWantedZ - mGhast->mZ;
    double distance = dx * dx + dy * dy + dz * dz;
    if (mCountdown-- <= 0)
    {
        mCountdown += mGhast->getRandom()->nextInt(5) + 2;
        distance = Mth::sqrt(distance);
        if (canReach(mWantedX, mWantedY, mWantedZ, distance))
        {
            mGhast->mXd += dx / distance * 0.1;
            mGhast->mYd += dy / distance * 0.1;
            mGhast->mZd += dz / distance * 0.1;
        }
        else
            mOperation = WAIT;
    }
}

Ghast::GhastShootFireballGoal::GhastShootFireballGoal(Ghast* ghast) : Goal(), mGhast(ghast)
{
}
Ghast::GhastShootFireballGoal::~GhastShootFireballGoal()
{
}
bool Ghast::GhastShootFireballGoal::canUse()
{
    return mGhast->getTarget() != boost::shared_ptr<LivingEntity>();
}
void Ghast::GhastShootFireballGoal::start()
{
    mChargeTime = 0;
}
void Ghast::GhastShootFireballGoal::stop()
{
    mGhast->setCharging(false);
}
void Ghast::GhastShootFireballGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mGhast->getTarget();
    double maxDistance = 64.0;
    if (target->distanceToSqr(mGhast->shared_from_this()) < maxDistance * maxDistance && mGhast->canSee(target))
    {
        Level* level = mGhast->mLevel;
        ++mChargeTime;
        if (mChargeTime == 10)
            level->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_GHAST_WARNING,
                              BlockPos(mGhast->shared_from_this()), 0);
        if (mChargeTime == 20)
        {
            Vec3* viewVector = mGhast->getViewVector(1.0f);
            double xd = target->mX - (mGhast->mX + viewVector->x * 4.0);
            double yd = (target->mBoundingBox->mMinY + (double)(target->mBbHeight * 0.5f))
                        - (mGhast->mY + 0.5 + (double)(mGhast->mBbHeight * 0.5f));
            double zd = target->mZ - (mGhast->mZ + viewVector->z * 4.0);
            level->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_GHAST_FIREBALL,
                              BlockPos(mGhast->shared_from_this()), 0);
            boost::shared_ptr<LargeFireball> fireball(new LargeFireball(
                level, boost::dynamic_pointer_cast<LivingEntity>(mGhast->shared_from_this()), xd, yd, zd));
            fireball->mExplosionPower = mGhast->getExplosionPower();
            fireball->mX = mGhast->mX + viewVector->x * 4.0;
            fireball->mY = mGhast->mY + (double)(mGhast->mBbHeight * 0.5f) + 0.5;
            fireball->mZ = mGhast->mZ + viewVector->z * 4.0;
            level->addEntity(fireball);
            mChargeTime = -40;
        }
    }
    else if (mChargeTime > 0)
    {
        --mChargeTime;
    }
    mGhast->setCharging(mChargeTime > 10);
}
Ghast::RandomFloatAroundGoal::RandomFloatAroundGoal(Ghast* ghast) : Goal(), mGhast(ghast)
{
    setRequiredControlFlags(1);
}
Ghast::RandomFloatAroundGoal::~RandomFloatAroundGoal()
{
}
bool Ghast::RandomFloatAroundGoal::canUse()
{
    MoveControl* control = static_cast<MoveControl*>(mGhast->getMoveControl());
    if (!control->hasWanted())
        return true;

    double dx = control->getWantedX() - mGhast->mX;
    double dy = control->getWantedY() - mGhast->mY;
    double dz = control->getWantedZ() - mGhast->mZ;
    double distance = dx * dx + dy * dy + dz * dz;
    if (distance < 1.0 || distance > 3600.0)
        return true;
    return false;
}
bool Ghast::RandomFloatAroundGoal::canContinueToUse()
{
    return false;
}
void Ghast::RandomFloatAroundGoal::start()
{
    Random* random = mGhast->getRandom();
    double x = mGhast->mX + (random->nextFloat() * 2.0f - 1.0f) * 16.0f;
    double y = mGhast->mY + (random->nextFloat() * 2.0f - 1.0f) * 16.0f;
    double z = mGhast->mZ + (random->nextFloat() * 2.0f - 1.0f) * 16.0f;
    mGhast->getMoveControl()->setWantedPosition(x, y, z, 1.0);
}
