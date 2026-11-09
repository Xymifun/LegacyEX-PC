#ifndef GAME_DATA_LEVELSTORAGEPROFILERDECORATOR_HPP
#define GAME_DATA_LEVELSTORAGEPROFILERDECORATOR_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/data/LevelStorage.hpp"

class ChunkStorage;
class Dimension;
class LevelData;
class Player;
class PlayerIO;
class ConsoleSaveFile;
class ConsoleSavePath;

class LevelStorageProfilerDecorator : public LevelStorage
{
public:
    LevelStorageProfilerDecorator(LevelStorage* levelStorage);
    virtual ~LevelStorageProfilerDecorator();

    virtual LevelData* prepareLevel();
    virtual void checkSession();
    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    virtual void saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players);
    virtual void saveLevelData(LevelData* levelData);
    virtual PlayerIO* getPlayerIO();
    virtual void closeAll();
    virtual ConsoleSavePath getDataFile(const std::wstring& name);
    virtual std::wstring getLevelId();
    virtual ConsoleSaveFile* getSaveFile();

private:
    LevelStorage* mLevelStorage;
};

#endif
