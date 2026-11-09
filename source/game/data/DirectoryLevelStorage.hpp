#ifndef GAME_DATA_DIRECTORYLEVELSTORAGE_HPP
#define GAME_DATA_DIRECTORYLEVELSTORAGE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/File.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/PlayerIO.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"


class ByteArrayOutputStream;
class CompoundTag;
class ConsoleSaveFile;
class DataFixerUpper;
class DataInputStream;
class DataOutputStream;
class Dimension;
class LevelData;
class Player;
class StructureManager;

class DirectoryLevelStorage : public LevelStorage, public PlayerIO
{
public:
    class PlayerMappings
    {
        friend class DirectoryLevelStorage;

    public:
        void writeMappings(DataOutputStream* output);
        void addMapping(int id, int centreX, int centreZ, int dimension, int scale);
        void readMappings(DataInputStream* input);
        bool getMapping(int& id, int centreX, int centreZ, int dimension, int scale);

    private:
        enum
        {
            COORDINATE_MASK = 0x1FFFFFFF,
            COORDINATE_X_SHIFT = 5,
            COORDINATE_Z_SHIFT = 2,
            SCALE_MASK = 7,
            SCALE_SHIFT = 2,
            DIMENSION_MASK = 3,
            HIGH_WORD_SHIFT = 32
        };

        boost::unordered_map<long long, short> mMappings;
    };

    DirectoryLevelStorage(ConsoleSaveFile* saveFile, File folder, const std::wstring& levelId, bool createIfMissing,
                          DataFixerUpper* fixer);
    virtual ~DirectoryLevelStorage();
    void initiateSession();

    void dontSaveMapMappingForPlayer(PlayerUID uid);
    virtual void save(boost::shared_ptr<Player> player);
    CompoundTag* loadPlayerDataTag(PlayerUID uid);
    virtual CompoundTag* load(boost::shared_ptr<Player> player);
    virtual void clearOldPlayerFiles();
    virtual void saveMapIdLookup();
    virtual void deleteMapFilesForPlayer(boost::shared_ptr<Player> player);
    void deleteMapFilesForPlayer(PlayerUID uid);
    virtual void saveAllCachedData();

    virtual void flushSaveFile(bool force);
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
    virtual StructureManager* getStructureManager();
    virtual ConsoleSaveFile* getSaveFile();
    virtual int getAuxValueForMap(PlayerUID uid, int dimension, int centreX, int centreZ, int scale);
    void resetPlayerPositionsInDimension(int dimension);
    virtual void resetNetherPlayerPositions();
    virtual void resetEndPlayerPositions();

private:
    typedef boost::unordered_map<PlayerUID, PlayerMappings, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >
        PlayerMappingsMap;
    typedef boost::unordered_map<std::wstring, ByteArrayOutputStream*> CachedSaveDataMap;

    static const unsigned int MAX_PLAYER_DATA_SAVES = 80;

    File mFolder;
    std::wstring mPlayerDir;
    std::wstring mDataDir;
    long long mSessionId;
    std::wstring mLevelId;
    StructureManager* mStructureManager;

protected:
    DataFixerUpper* mFixer;

private:
    PlayerMappingsMap mPlayerMappings;
    arrayWithLength<unsigned char> mUsedMappings;
    bool mHasLoadedMapDataMappings;
    CachedSaveDataMap mCachedSaveData;
    std::vector<short> mMapFilesToDelete;

protected:
    ConsoleSaveFile* mSaveFile;

    static const std::wstring NETHER_FOLDER;
    static const std::wstring ENDER_FOLDER;
};

#endif
