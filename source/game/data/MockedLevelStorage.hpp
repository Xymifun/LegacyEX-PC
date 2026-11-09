#ifndef GAME_DATA_MOCKEDLEVELSTORAGE_HPP
#define GAME_DATA_MOCKEDLEVELSTORAGE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/data/LevelStorage.hpp"

class Dimension;
class LevelData;
class Player;

class MockedLevelStorage : public LevelStorage
{
public:
    virtual ~MockedLevelStorage();
    virtual LevelData* prepareLevel();
    virtual void checkSession();
    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    virtual void saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players);
    virtual void saveLevelData(LevelData* levelData);
    virtual PlayerIO* getPlayerIO();
    virtual void closeAll();
    virtual File getFolder();
    virtual ConsoleSavePath getDataFile(const std::wstring& name);
    virtual std::wstring getLevelId();
    virtual ConsoleSaveFile* getSaveFile();
    virtual StructureManager* getStructureManager();
};

#endif
