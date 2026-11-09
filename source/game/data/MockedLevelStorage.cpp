#include "game/data/MockedLevelStorage.hpp"

#include "game/core/File.hpp"

MockedLevelStorage::~MockedLevelStorage()
{
}

LevelData* MockedLevelStorage::prepareLevel()
{
    return NULL;
}

void MockedLevelStorage::checkSession()
{
}

ChunkStorage* MockedLevelStorage::createChunkStorage(Dimension* dimension)
{
    return NULL;
}

void MockedLevelStorage::saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players)
{
}

void MockedLevelStorage::saveLevelData(LevelData* levelData)
{
}

PlayerIO* MockedLevelStorage::getPlayerIO()
{
    return NULL;
}

void MockedLevelStorage::closeAll()
{
}

File MockedLevelStorage::getFolder()
{
    return File();
}

ConsoleSavePath MockedLevelStorage::getDataFile(const std::wstring& name)
{
    return ConsoleSavePath(L"");
}

std::wstring MockedLevelStorage::getLevelId()
{
    return std::wstring(L"none");
}

ConsoleSaveFile* MockedLevelStorage::getSaveFile()
{
    return NULL;
}

StructureManager* MockedLevelStorage::getStructureManager()
{
    return NULL;
}
