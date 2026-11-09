#include "game/data/LevelStorageProfilerDecorator.hpp"

#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/ChunkStorageProfilerDecorator.hpp"

LevelStorageProfilerDecorator::~LevelStorageProfilerDecorator()
{
}

LevelData* LevelStorageProfilerDecorator::prepareLevel()
{
    return mLevelStorage->prepareLevel();
}

void LevelStorageProfilerDecorator::checkSession()
{
    mLevelStorage->checkSession();
}

ChunkStorage* LevelStorageProfilerDecorator::createChunkStorage(Dimension* dimension)
{
    return new ChunkStorageProfilerDecorator(mLevelStorage->createChunkStorage(dimension));
}

void LevelStorageProfilerDecorator::saveLevelData(LevelData* levelData,
                                                  std::vector<boost::shared_ptr<Player> >* players)
{
    mLevelStorage->saveLevelData(levelData, players);
}

void LevelStorageProfilerDecorator::saveLevelData(LevelData* levelData)
{
    mLevelStorage->saveLevelData(levelData);
}

PlayerIO* LevelStorageProfilerDecorator::getPlayerIO()
{
    return mLevelStorage->getPlayerIO();
}

void LevelStorageProfilerDecorator::closeAll()
{
    mLevelStorage->closeAll();
}

ConsoleSavePath LevelStorageProfilerDecorator::getDataFile(const std::wstring& name)
{
    return mLevelStorage->getDataFile(name);
}

std::wstring LevelStorageProfilerDecorator::getLevelId()
{
    return mLevelStorage->getLevelId();
}

ConsoleSaveFile* LevelStorageProfilerDecorator::getSaveFile()
{
    return mLevelStorage->getSaveFile();
}
