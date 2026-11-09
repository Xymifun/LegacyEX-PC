#include "game/world/DerivedLevelData.hpp"

DerivedLevelData::DerivedLevelData(LevelData* levelData) : LevelData(), mLevelData(levelData)
{
}

CompoundTag* DerivedLevelData::createTag(std::vector<boost::shared_ptr<Player> >* players)
{
    return mLevelData->createTag(players);
}

void DerivedLevelData::setTagData(CompoundTag* tag)
{
    mLevelData->setTagData(tag);
}
long long DerivedLevelData::getSeed()
{
    return mLevelData->getSeed();
}
int DerivedLevelData::getXSpawn()
{
    return mLevelData->getXSpawn();
}
int DerivedLevelData::getYSpawn()
{
    return mLevelData->getYSpawn();
}
int DerivedLevelData::getZSpawn()
{
    return mLevelData->getZSpawn();
}
long long DerivedLevelData::getGameTime()
{
    return mLevelData->getGameTime();
}
long long DerivedLevelData::getDayTime()
{
    return mLevelData->getDayTime();
}
long long DerivedLevelData::getSizeOnDisk()
{
    return mLevelData->getSizeOnDisk();
}
CompoundTag* DerivedLevelData::getLoadedPlayerTag()
{
    return mLevelData->getLoadedPlayerTag();
}
void DerivedLevelData::setSeed(long long)
{
}
void DerivedLevelData::setXSpawn(int)
{
}
void DerivedLevelData::setYSpawn(int)
{
}
void DerivedLevelData::setZSpawn(int)
{
}
void DerivedLevelData::setGameTime(long long)
{
}
void DerivedLevelData::setDayTime(long long)
{
}
void DerivedLevelData::setSizeOnDisk(long long)
{
}
void DerivedLevelData::setLoadedPlayerTag(CompoundTag*)
{
}
void DerivedLevelData::setSpawn(const BlockPos&)
{
}
std::wstring DerivedLevelData::getLevelName()
{
    return mLevelData->getLevelName();
}
void DerivedLevelData::setLevelName(const std::wstring&)
{
}
int DerivedLevelData::getVersion()
{
    return mLevelData->getVersion();
}
void DerivedLevelData::setVersion(int)
{
}
long long DerivedLevelData::getLastPlayed()
{
    return mLevelData->getLastPlayed();
}
bool DerivedLevelData::isThundering()
{
    return mLevelData->isThundering();
}
int DerivedLevelData::getThunderTime()
{
    return mLevelData->getThunderTime();
}
bool DerivedLevelData::isRaining()
{
    return mLevelData->isRaining();
}
int DerivedLevelData::getRainTime()
{
    return mLevelData->getRainTime();
}
const GameType* DerivedLevelData::getGameType()
{
    return mLevelData->getGameType();
}
void DerivedLevelData::setThundering(bool)
{
}
void DerivedLevelData::setThunderTime(int)
{
}
void DerivedLevelData::setRaining(bool)
{
}
void DerivedLevelData::setRainTime(int)
{
}
bool DerivedLevelData::isGenerateMapFeatures()
{
    return mLevelData->isGenerateMapFeatures();
}
void DerivedLevelData::setGameType(const GameType*)
{
}
bool DerivedLevelData::isHardcore()
{
    return mLevelData->isHardcore();
}
LevelType* DerivedLevelData::getGeneratorType()
{
    return mLevelData->getGeneratorType();
}
void DerivedLevelData::setGeneratorType(LevelType*)
{
}
bool DerivedLevelData::getAllowCommands()
{
    return mLevelData->getAllowCommands();
}
void DerivedLevelData::setAllowCommands(bool)
{
}
bool DerivedLevelData::isInitialized()
{
    return mLevelData->isInitialized();
}
void DerivedLevelData::setInitialized(bool)
{
}
GameRules* DerivedLevelData::getGameRules()
{
    return mLevelData->getGameRules();
}
const Difficulty* DerivedLevelData::getDifficulty()
{
    return mLevelData->getDifficulty();
}
void DerivedLevelData::setDifficulty(const Difficulty*)
{
}
bool DerivedLevelData::isDifficultyLocked()
{
    return mLevelData->isDifficultyLocked();
}
void DerivedLevelData::setDifficultyLocked(bool)
{
}
void DerivedLevelData::setDimensionData(const DimensionType* dimension, CompoundTag* tag)
{
    mLevelData->setDimensionData(dimension, tag);
}
CompoundTag* DerivedLevelData::getDimensionData(const DimensionType* dimension)
{
    return mLevelData->getDimensionData(dimension);
}
int DerivedLevelData::getXZSize()
{
    return mLevelData->getXZSize();
}
int DerivedLevelData::getHellScale()
{
    return mLevelData->getHellScale();
}
void DerivedLevelData::setModernEnd()
{
    mLevelData->setModernEnd();
}
bool DerivedLevelData::getModernEnd()
{
    return mLevelData->getModernEnd();
}
