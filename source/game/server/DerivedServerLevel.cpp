#include "game/server/DerivedServerLevel.hpp"

#include <typeinfo>

#include <boost/shared_ptr.hpp>

#include "game/data/SavedDataStorage.hpp"
#include "game/world/DerivedLevelData.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/generation/Villages.hpp"

DerivedServerLevel::DerivedServerLevel(MinecraftServer* server, boost::shared_ptr<LevelStorage> storage, int dimension,
                                       ServerLevel* level)
    : ServerLevel(server, storage, new DerivedLevelData(level->getLevelData()), dimension),
      mLevel(level),
      mBorderListener(new DerivedLevelBorderChangeListener(this))
{
    level->getWorldBorder()->addListener(mBorderListener);
}

DerivedServerLevel::~DerivedServerLevel()
{
    delete mBorderListener;
    mSavedDataStorage = NULL;
    serverScoreboard = NULL;
    mLootTables = NULL;
}

void DerivedServerLevel::saveLevelData()
{
}

Level* DerivedServerLevel::postConstruct()
{
    mSavedDataStorage = mLevel->getSavedDataStorage();
    serverScoreboard = mLevel->GetServerScoreboard();
    mLootTables = mLevel->getLootTables();

    std::wstring id = Villages::getFileId(mDimension);
    boost::shared_ptr<Villages> villages = boost::dynamic_pointer_cast<Villages>(
        mSavedDataStorage->get(typeid(Villages), id));
    if (!villages)
    {
        mVillages = boost::shared_ptr<Villages>(new Villages(this));
        mSavedDataStorage->set(id, mVillages);
    }
    else
    {
        mVillages = villages;
        mVillages->setLevel(this);
    }
    return this;
}

void DerivedServerLevel::saveDimensionData()
{
    mDimension->saveData();
}

DerivedServerLevel::DerivedLevelBorderChangeListener::DerivedLevelBorderChangeListener(DerivedServerLevel* level)
    : mLevel(level)
{
}

DerivedServerLevel::DerivedLevelBorderChangeListener::~DerivedLevelBorderChangeListener()
{
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderSizeSet(WorldBorder*, double size)
{
    mLevel->getWorldBorder()->setSize(size);
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderSizeLerping(WorldBorder*, double oldSize,
                                                                               double newSize, long long timeMs)
{
    mLevel->getWorldBorder()->lerpSizeBetween(oldSize, newSize, timeMs);
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderCenterSet(WorldBorder*, double centerX,
                                                                             double centerZ)
{
    mLevel->getWorldBorder()->setCenter(centerX, centerZ);
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderSetWarningTime(WorldBorder*, int warningTime)
{
    mLevel->getWorldBorder()->setWarningTime(warningTime);
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderSetWarningBlocks(WorldBorder*, int warningBlocks)
{
    mLevel->getWorldBorder()->setWarningBlocks(warningBlocks);
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderSetDamagePerBlock(WorldBorder*,
                                                                                     double damagePerBlock)
{
    mLevel->getWorldBorder()->setDamagePerBlock(damagePerBlock);
}

void DerivedServerLevel::DerivedLevelBorderChangeListener::onBorderSetDamageSafeZOne(WorldBorder*,
                                                                                     double damageSafeZone)
{
    mLevel->getWorldBorder()->setDamageSafeZone(damageSafeZone);
}
