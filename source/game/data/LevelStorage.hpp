#ifndef GAME_DATA_LEVELSTORAGE_HPP
#define GAME_DATA_LEVELSTORAGE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/data/ConsoleSavePath.hpp"
#include "game/world/generation/PlayerUID.hpp"

class ChunkStorage;
class ConsoleSaveFile;
class Dimension;
class File;
class LevelData;
class Player;
class PlayerIO;
class StructureManager;

class LevelStorage
{
public:
    virtual ~LevelStorage() {}

    virtual LevelData* prepareLevel() = 0;
    virtual void checkSession() = 0;
    virtual ChunkStorage* createChunkStorage(Dimension* dimension) = 0;
    virtual void saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players) = 0;
    virtual void saveLevelData(LevelData* levelData) = 0;
    virtual PlayerIO* getPlayerIO() = 0;
    virtual void closeAll() = 0;
    virtual File getFolder() = 0;
    virtual ConsoleSavePath getDataFile(const std::wstring& name) = 0;
    virtual std::wstring getLevelId() = 0;
    virtual StructureManager* getStructureManager() = 0;
    virtual ConsoleSaveFile* getSaveFile();
    virtual void flushSaveFile(bool bForce);
    virtual int getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale);
};

#endif
