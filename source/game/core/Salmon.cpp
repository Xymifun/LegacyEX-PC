#include "game/core/Salmon.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/RiverFollowingGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

static const EntityDataAccessor<unsigned char>* DATA_ID_SCALE = NULL;

Salmon::Salmon(Level* level) : Fish(level, true)
{
    fjDerivedCtorCalls();
    SalmonWeightedRandomize();
    mSwimAmplitude = 0.01f;
    mSwimFrequency = 0.02f;
    mWaterValueC = 12.0f;
}

Entity* Salmon::create(Level* level)
{
    return new Salmon(level);
}

void Salmon::staticCtor()
{
    DATA_ID_SCALE = SynchedEntityData::defineId(EntityType::SALMON, EntityType::WATER_ANIMAL,
                                                EntityDataSerializers::BYTE);
}

void Salmon::setSalmonScale(SalmonScale scale)
{
    if (!mLevel->mIsClientSide)
        mEntityData->set(DATA_ID_SCALE, static_cast<unsigned char>(scale), true);
}

void Salmon::SalmonWeightedRandomize()
{
    int value = mRandom->nextInt(95);
    if (value < 30)
        setSalmonScale(SMALL);
    else
    {
        SalmonScale scale = LARGE;
        if (value <= 80)
            scale = MEDIUM;
        setSalmonScale(scale);
    }
}

Salmon::SalmonScale Salmon::getSalmonScale()
{
    return static_cast<SalmonScale>(mEntityData->get(DATA_ID_SCALE));
}

void Salmon::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"SalmonScale", static_cast<int>(getSalmonScale()));
}

void Salmon::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setSalmonScale(static_cast<SalmonScale>(tag->getInt(L"SalmonScale")));
}

void Salmon::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_ID_SCALE->getId() != accessor->getId())
        return;

    switch (getSalmonScale())
    {
    case SMALL:
        setSize(0.5f, 0.5f);
        mSwimFlag = true;
        mSwimSpeed = 0.5f;
        mSwimTicks = 24;
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(2.0);
        break;
    case MEDIUM:
        setSize(1.0f, 1.0f);
        mSwimFlag = false;
        mSwimSpeed = 1.0f;
        mSwimTicks = 1;
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(3.0);
        break;
    case LARGE:
        setSize(1.5f, 1.5f);
        mSwimFlag = true;
        mSwimSpeed = 1.5f;
        mSwimTicks = -7;
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(5.0);
        break;
    }
}

void Salmon::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_SCALE, static_cast<unsigned char>(MEDIUM));
}

void Salmon::registerGoals()
{
    Fish::registerGoals();
    mGoalSelector.addGoal(3, new RiverFollowingGoal(this, 2.0f, 0.07f), true);
}

void Salmon::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(3.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.2);
}

ResourceLocation* Salmon::getDefaultLootTable()
{
    if (getSalmonScale() == LARGE)
        return &BuiltInLootTables::SALMON_LARGE;
    return &BuiltInLootTables::SALMON_NORMAL;
}

unsigned long long Salmon::GetType()
{
    return EntityType::SALMON;
}
