#include "game/core/Bat.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/SharedConstants.hpp"

const EntityDataAccessor<unsigned char>* Bat::DATA_ID_FLAGS = NULL;

Bat::Bat(Level* level) : AmbientCreature(level)
{
    fjDerivedCtorCalls();
    mTargetPosition = NULL;
    setSize(0.5f, 0.9f);
    setResting(true);
    mCategories |= EntityCategory::eEntityCategory_Bat;
}

Bat::~Bat()
{
    delete mTargetPosition;
}

void Bat::staticCtor()
{
    DATA_ID_FLAGS = SynchedEntityData::defineId(EntityType::BAT, EntityType::AMBIENT_CREATURE,
                                                EntityDataSerializers::BYTE);
}

void Bat::setResting(bool resting)
{
    unsigned char flags = mEntityData->get(DATA_ID_FLAGS);
    if (resting)
    {
        mEntityData->set(DATA_ID_FLAGS, (unsigned char)(flags | 1), true);
    }
    else
    {
        mEntityData->set(DATA_ID_FLAGS, (unsigned char)(flags & 0xfe), true);
    }
}

void Bat::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_FLAGS, (unsigned char)0);
}

float Bat::getSoundVolume()
{
    return 0.1f;
}

float Bat::getVoicePitch()
{
    return LivingEntity::getVoicePitch() * 0.95f;
}

const SoundEvent* Bat::getAmbientSound()
{
    if (isResting() && mRandom->nextInt(4) != 0)
        return NULL;
    return SoundEvents::BAT_AMBIENT;
}

const SoundEvent* Bat::getHurtSound(const DamageSource*)
{
    return SoundEvents::BAT_HURT;
}

const SoundEvent* Bat::getDeathSound()
{
    return SoundEvents::BAT_DEATH;
}

bool Bat::isPushable(boost::shared_ptr<Entity>)
{
    return false;
}

void Bat::doPush(boost::shared_ptr<Entity>)
{
}

void Bat::pushEntities()
{
}

void Bat::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
}

bool Bat::isResting()
{
    return (mEntityData->get(DATA_ID_FLAGS) & 1) != 0;
}

void Bat::tick()
{
    PIXBeginNamedEvent(0.0f, "Bat tick");
    Mob::tick();
    if (isResting())
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
        mY = (double)Mth::floor(mY) + 1.0 - (double)mBbHeight;
    }
    else
    {
        mYd *= 0.6f;
    }
    PIXEndNamedEvent();
}

void Bat::customServerAiStep()
{
    Mob::customServerAiStep();

    BlockPos pos(shared_from_this());
    BlockPos above(pos.above());
    if (isResting())
    {
        const BlockState* state = mLevel->getBlockState(above);
        if (!state->getBlock()->isSolidBlockingCubeAndNotSignalSource(state))
        {
            setResting(false);
            mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_BAT_LIFTOFF, pos, 0);
        }
        else
        {
            if (mRandom->nextInt(200) == 0)
                mYHeadRot = (float)mRandom->nextInt(360);
            if (mLevel->getNearestPlayer(shared_from_this(), 4.0, -1.0) != NULL)
            {
                setResting(false);
                mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_BAT_LIFTOFF, pos, 0);
            }
        }
        return;
    }

    if (mTargetPosition != NULL && (!mLevel->isEmptyBlock(*mTargetPosition) || mTargetPosition->getY() < 1))
    {
        delete mTargetPosition;
        mTargetPosition = NULL;
    }
    if (mTargetPosition == NULL || mRandom->nextInt(30) == 0
        || mTargetPosition->distSqr((int)mX, (int)mY, (int)mZ) < 4.0)
    {
        delete mTargetPosition;
        mTargetPosition = new BlockPos((int)mX + mRandom->nextInt(7) - mRandom->nextInt(7),
                                       (int)mY + mRandom->nextInt(6) - 2,
                                       (int)mZ + mRandom->nextInt(7) - mRandom->nextInt(7));
    }

    double xDelta = ((double)mTargetPosition->getX() + 0.5) - mX;
    double yDelta = ((double)mTargetPosition->getY() + 0.1) - mY;
    double zDelta = ((double)mTargetPosition->getZ() + 0.5) - mZ;
    float xDirection = (float)((xDelta > 0.0) - (xDelta < 0.0));
    float yDirection = (float)((yDelta > 0.0) - (yDelta < 0.0));
    float zDirection = (float)((zDelta > 0.0) - (zDelta < 0.0));
    mXd += (xDirection * 0.5f - mXd) * 0.1f;
    mYd += (yDirection * 0.7f - mYd) * 0.1f;
    mZd += (zDirection * 0.5f - mZd) * 0.1f;
    float wantedYRot = (float)(std::atan2(mZd, mXd) * Mth::RAD_TO_DEG) - 90.0f;
    float yRotDelta = Mth::wrapDegrees(wantedYRot - mYRot);
    mYRot += yRotDelta;
    mZza = 0.5f;
    const BlockState* aboveState = mLevel->getBlockState(above);
    if (mRandom->nextInt(100) == 0 && aboveState->getBlock()->isSolidBlockingCubeAndNotSignalSource(aboveState))
        setResting(true);
}

bool Bat::makeStepSound()
{
    return false;
}

void Bat::causeFallDamage(float, float)
{
}

void Bat::checkFallDamage(double, bool, Block*, const BlockPos&)
{
}

bool Bat::isIgnoringBlockTriggers()
{
    return true;
}

bool Bat::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    if (!mLevel->mIsClientSide && isResting())
        setResting(false);
    return LivingEntity::hurt(source, damage);
}

void Bat::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::BAT);
}

void Bat::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mEntityData->set(DATA_ID_FLAGS, tag->getByte(L"BatFlags"), true);
}

void Bat::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putByte(L"BatFlags", mEntityData->get(DATA_ID_FLAGS));
}

bool Bat::checkSpawnRules()
{
    BlockPos pos(mX, mBoundingBox->mMinY, mZ);
    if (pos.y >= mLevel->getSeaLevel())
        return false;
    int brightness = mLevel->getRawBrightness(pos);
    int maxLight = 4;
    if (SharedConstants::IS_HALLOWEEN_SEASON)
        maxLight = 7;
    else if (mRandom->nextBoolean())
        return false;
    if (brightness > mRandom->nextInt(maxLight))
        return false;
    return Mob::checkSpawnRules();
}

float Bat::getEyeHeight()
{
    return (double)(mBbHeight * 0.5f);
}

ResourceLocation* Bat::getDefaultLootTable()
{
    return &BuiltInLootTables::BAT;
}

unsigned long long Bat::GetType()
{
    return EntityType::BAT;
}

Entity* Bat::create(Level* level)
{
    return new Bat(level);
}
