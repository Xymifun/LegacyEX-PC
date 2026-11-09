#ifndef GAME_DATA_MCREGIONLEVELSTORAGE_HPP
#define GAME_DATA_MCREGIONLEVELSTORAGE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/data/DirectoryLevelStorage.hpp"
#include "game/world/chunk/McRegionChunkStorage.hpp"

class ConsoleSaveFile;
class DataFixerUpper;
class Dimension;
class File;
class LevelData;
class Player;

class McRegionLevelStorage : public DirectoryLevelStorage
{
public:
    McRegionLevelStorage(ConsoleSaveFile* saveFile, File folder, const std::wstring& levelId, bool createIfMissing,
                         DataFixerUpper* fixer);
    virtual ~McRegionLevelStorage();
    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    virtual void saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players);
    virtual void closeAll();
};

#endif
