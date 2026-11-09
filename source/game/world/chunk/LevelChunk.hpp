#ifndef GAME_WORLD_CHUNK_LEVELCHUNK_HPP
#define GAME_WORLD_CHUNK_LEVELCHUNK_HPP

#include <vector>
#include <deque>
#include <typeinfo>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/LevelSource.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/data/SparseLightStorage.hpp"

class AABB;
class BiomeSource;
class Biome;
class Block;
class BlockEntity;
class BlockState;
class ChunkGenerator;
class ChunkPos;
class ChunkPrimer;
class ChunkSource;
class CompoundTag;
class DataInputStream;
class DataOutputStream;
class ElementStorage;
class Entity;
class Level;
class SparseLightStorage;
class Direction;
class OldChunkStorage;
class ServerChunkCache;


class LevelChunk
{
public:
    friend class OldChunkStorage;
    friend class ServerChunkCache;
    friend class ClientChunkCache;
    friend class Level;
    friend class ConsoleSchematicFile;

    typedef boost::unordered_map<BlockPos, boost::shared_ptr<BlockEntity>, boost::hash<const BlockPos>,
                                 std::equal_to<const BlockPos> >
        BlockEntityMap;
    enum EntityCreationType
    {
        IMMEDIATE,
        QUEUED,
        NO_CREATE
    };
    static const int CURRENT_CHUNK_VERSION = 12;
    static const unsigned int LIGHT_SECTION_BYTES = 16 * 16 * 128 / 2;

    enum BiomePresentFlag
    {
        BIOME_PRESENT_OCEAN = 1,
        BIOME_PRESENT_DEEP_OCEAN = 2,
        BIOME_PRESENT_SLIME_CHUNK = 4,
        BIOME_PRESENT_SWAMP = 8
    };

    enum TerrainPopulatedFlag
    {
        TERRAIN_POPULATED_SELF = 0x0002,
        TERRAIN_POPULATED_WEST = 0x0004,
        TERRAIN_POPULATED_NORTH = 0x0008,
        TERRAIN_POPULATED_NORTH_WEST = 0x0010,
        TERRAIN_POPULATED_SOUTH_WEST = 0x0020,
        TERRAIN_POPULATED_SOUTH = 0x0040,
        TERRAIN_POPULATED_SOUTH_EAST = 0x0080,
        TERRAIN_POPULATED_EAST = 0x0100,
        TERRAIN_POPULATED_NORTH_EAST = 0x0200,
        TERRAIN_POPULATED_ALL_NEIGHBOURS = 0x03fe,
        TERRAIN_POPULATED_FULL = 0x0400,
        TERRAIN_POPULATED_ALL = 0x07fe
    };
    class BlockChange
    {
    public:
        BlockChange(short packedPos, unsigned int element) : mPackedPos(packedPos), mElement(element) {}
        short GetPackedPos() const;
        unsigned int GetElement() const;
        BlockPos GetPos() const;
        const BlockState* GetState() const;
        const BlockState* GetExtraState() const;

    private:
        short mPackedPos;
        short mReserved;
        int mElement;
    };
    class ThreadStorage
    {
    public:
        ThreadStorage();
        unsigned short* elementStorage;
        unsigned short* extraElementStorage;
        unsigned char* tempLightingStorage;
    };

    LevelChunk(Level* level, int x, int z);
    LevelChunk(Level* level, ChunkPrimer* primer, int x, int z);
    LevelChunk(Level* level, int x, int z, LevelChunk* source);
    bool isReady();
    int getBlockId(int x, int y, int z);
    unsigned int getLowestHeightmap();
    unsigned short getElement(int x, int y, int z);
    bool sectionContainsBlockId(int x, int y, int z, int blockId);
    int getExtraBlockId(int x, int y, int z);
    void getBlockAndExtraId(int x, int y, int z, int* blockId, int* extraBlockId);
    unsigned int getExtraBlockData(int x, int y, int z);
    unsigned short getElement(int x, int y, int z, bool* present);
    int getHighestSectionPosition();
    BlockPos getTopRainBlock(const BlockPos& pos);
    void updateBiomeFlags(int x, int z);
    bool biomeHasRain(int x, int z);
    bool biomeHasSnow(int x, int z);
    long long getInhabitedTime();
    bool isTerrainPopulated();
    bool isRenderChunkEmpty(int section);
    virtual void getBlockData(arrayWithLength<unsigned char> data);
    virtual void setAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data);
    virtual void setAllElements(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data);
#ifdef MINECRAFT_PC
    void ReplaceUnknownBlocks();
    void WriteCompressedElementsForSave(DataOutputStream* stream);
#endif
    virtual void getAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data);
    virtual void getElementRange(arrayWithLength<unsigned short> elements, int start, int end);
    virtual void getElementRange(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data,
                                 int start, int end);
    static int getRawDataMaxBytes();
    virtual arrayWithLength<unsigned char> getRawData();
    virtual void setFromRawData(arrayWithLength<unsigned char> data);
    virtual void getDataData(arrayWithLength<unsigned char> data);
    virtual void getSkyLightData(arrayWithLength<unsigned char> data);
    virtual void getBlockLightData(arrayWithLength<unsigned char> data);
    virtual void setSkyLightData(arrayWithLength<unsigned char> data);
    virtual void setBlockLightData(arrayWithLength<unsigned char> data);
    void setSkyLightDataAllBright();
    bool isLowerBlockLightStorageCompressed();
    void writeCompressedElements(DataOutputStream* stream);
    void readCompressedElements(DataInputStream* stream);
    void readCompressedElementsFromBlockAndData(DataInputStream* stream);
    void writeCompressedSkyLightData(DataOutputStream* stream);
    void writeCompressedBlockLightData(DataOutputStream* stream);
    unsigned char* GetTempLightingStorage();
    void readCompressedSkyLightData(DataInputStream* stream);
    void readCompressedBlockLightData(DataInputStream* stream);
    unsigned int getSparseBlockAndDataMemoryAllocated();
    bool getHasGapsToCheck();
    unsigned int getBiomePresentFlag();
    virtual void reSyncLighting();
    virtual void reSyncLightingNearby();
    virtual void dropLighting();
    void SetInvalid(bool invalid);
    bool hasLightingBeenDropped();
    virtual void init(Level* level, int x, int z);
    virtual ~LevelChunk();
    virtual bool isAt(int x, int z);
    int getHeightmap(const BlockPos& pos);
    virtual unsigned char getHeightmap(int x, int z);
    virtual void recalcBlockLights();
    int getHighestNonEmptyY();
    unsigned short* GetElementStorage();
    unsigned short* GetExtraElementStorage();
    void setUnsaved(bool unsaved);
    virtual void recalcHeightmapOnly();
    void lightGaps(int x, int z);
    virtual void recalcHeightmap();
    Block* getChunkBlock(int x, int y, int z);
    virtual void lightLava();
    void lightGap(int x, int y, int z, int value);
    void lightGap(int x, int y, int z);
    void recheckGaps(bool onlyOne);
    void recalcHeight(int x, int y, int z);
    virtual int getBlockLightBlock(const BlockPos& pos);
    int getChunkBlockLightBlock(int x, int y, int z);
    virtual Block* getBlock(const BlockPos& pos);
    virtual Block* getExtraBlock(const BlockPos& pos);
    const BlockState* getBlockState(int x, int y, int z);
    virtual const BlockState* getBlockState(const BlockPos& pos);
    virtual const BlockState* getExtraBlockState(const BlockPos& pos);
    void getBlockAndExtraId_NoLock(int x, int y, int z, int* blockId, int* extraBlockId);
    void getElementReadLock();
    void releaseElementReadLock();
    unsigned int getElementFrameLastUpdated();
    void setElement(int x, int y, int z, unsigned short element);
    unsigned short getChunkBlockData(int x, int y, int z);
    virtual int getData(const BlockPos& pos);
    unsigned int getData(int x, int y, int z);
    virtual bool setData(int x, int y, int z, int data, int dataMask, bool* maskedChange);
    virtual const BlockState* setBlock(const BlockPos& pos, const BlockState* state);
    virtual const BlockState* setExtraBlock(const BlockPos& pos, const BlockState* state);
    virtual const BlockState* internalSetBlock(bool extra, const BlockPos& pos, const BlockState* state);
    virtual bool setBlockAndData(int x, int y, int z, int blockId, int data, bool update, bool extra);
    virtual void setExtraElement(int x, int y, int z, unsigned short element);
    virtual unsigned short getExtraElement(int x, int y, int z);
    virtual void getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement);
    virtual int getBrightness(LightLayer::variety layer, const BlockPos& pos);
    virtual void getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z);
    virtual void setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness);
    virtual int getRawBrightness(const BlockPos& pos, int skyDarken);
    virtual void addEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity, int section);
    virtual bool isSkyLit(const BlockPos& pos);
    boost::shared_ptr<BlockEntity> createBlockEntity(const BlockPos& pos);
    virtual boost::shared_ptr<BlockEntity> getBlockEntity(const BlockPos& pos, EntityCreationType type);
    virtual void addBlockEntity(boost::shared_ptr<BlockEntity> entity);
    virtual void setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity);
    virtual void removeBlockEntity(const BlockPos& pos);
    virtual void load(bool blockEntitiesOnly);
    virtual void unload(bool removeBlockEntities, bool removeEntities);
    virtual bool containsPlayer();
    virtual bool isUnloaded();
    virtual void markUnsaved();
    virtual void getEntities(boost::shared_ptr<Entity> except, const AABB* box,
                             std::vector<boost::shared_ptr<Entity> >& entities,
                             const Predicate<boost::shared_ptr<Entity> >* predicate);
    virtual void getEntitiesOfClass(const std::type_info& type, const AABB* box,
                                    std::vector<boost::shared_ptr<Entity> >& entities,
                                    const Predicate<boost::shared_ptr<Entity> >* predicate,
                                    bool excludeInvisiblePlayers);
    virtual int countEntities();
    virtual bool shouldSave(bool force);
    virtual int getBlocksAndData(arrayWithLength<unsigned char>* output, int x, int y, int z, int width, int height,
                                 int depth, int flags, bool extra);
    static void blockUpdatedCallback(int x, int y, int z, void* data, int value);
    virtual int setBlocksAndData(arrayWithLength<unsigned char> input, int x, int y, int z, int width, int height,
                                 int depth, int flags, bool extra);
    virtual Random* getRandom(long long seed);
    virtual bool isEmpty();
    static void staticCtor();
    void postProcess();
    void flagPostProcessComplete(short value);
    void doPostProcess(ChunkSource* source, ChunkGenerator* generator);
    void checkPostProcess(ChunkSource* source, ChunkGenerator* generator);
    void checkChests(ChunkSource* source, int x, int z);
    void tick(bool update);
    ChunkPos getPos();
    bool isYSpaceEmpty(int y0, int y1);
    void reloadBiomes();
    virtual Biome* getBiome(const BlockPos& pos, BiomeSource* source);
    arrayWithLength<unsigned char> getBiomes();
    void updateBiomePresentFlags();
    void setBiomes(arrayWithLength<unsigned char> biomes);
    void setCheckAllLight();
    bool postProcessAt(int x, int z);
    Level* getLevel();
    arrayWithLength<unsigned char> getHeightmap();
    void recalcMinAndMaxHeights();
    BlockEntityMap* getBlockEntities();
    void setLastSaveHadEntities(bool value);
    void setLastSaveTime(long long value);
    void setInhabitedTime(long long value);
    virtual void compressLighting();
    virtual void compressBlocks();
    virtual void compressData();
    void setChunkVersion(int version);
    void swapEntities(LevelChunk* chunk);
    static bool touchedSky;
    static OSFastMutex m_csLighting;
    static OSFastMutex m_csEntities;
    static OSFastMutex m_csTileEntities;
    static int PackBlockPos(const BlockPos& pos);
    static BlockPos UnpackBlockPos(short packed);
    static void CreateNewThreadStorage();
    static void ReleaseThreadStorage();
    int getBlocksAllocatedSize(int*, int*, int*, int*, int*);
    void setWillBeSaved(bool);
    bool getWillBeSaved();
    Block* getBlockLocal(int, int, int);
    int getDataLocal(int, int, int);
    int getExtraBlockData(const BlockPos&);
    bool isTicked();
    void checkNextLight();
    void postProcessEdge(const Direction*);
    bool isLoaded();
    void setLoaded(bool);
    void setHeightmap(arrayWithLength<int>);
    void setTerrainPopulated(short);
    bool isLightPopulated();
    void setLightPopulated(bool);
    int getChunkVersion();

private:
    arrayWithLength<unsigned char> mBiomes;
    int mEntitySectionCount;
    ElementStorage* mElements;
    bool mLoaded;
    unsigned char mPrecipitationHeightmap[256];
    unsigned char mBiomeFlags[128];

public:
    Level* level;

private:
    SparseLightStorage* mSkyLight;
    SparseLightStorage* mSkyLightUpper;
    SparseLightStorage* mBlockLight;
    SparseLightStorage* mBlockLightUpper;
    arrayWithLength<unsigned char> mHeightmap;
    int mMinHeight;
    int mMaxHeight;

public:
    int x;
    int z;

private:
    bool mHasGapsToCheck;
    unsigned int mBiomePresent;
    BlockEntityMap mBlockEntityMap;
    std::vector<boost::shared_ptr<Entity> >** mEntitySections;

protected:
    short mChunkFlags;

private:
    unsigned short mChunkFlagsPadding;
    short* mServerChunkFlags;
    bool mPostProcess;
    bool mLightPopulated;
    bool mUnsaved;

protected:
    bool mDontSave;

private:
    bool mLastSaveHadEntities;
    unsigned short mEmissiveMaxHeight;
    int mEntityCount;
    int mLightCheckIndex;
    int mQueuedLightChecks;
    long long mLastSaveTime;
    bool mReservedFlag;
    int mLowestHeightmap;
    long long mInhabitedTime;
    bool mUnloaded;
    CompoundTag* mPendingBlockEntityTag;
    bool mInvalid;
    int mCheckAllLight;
    int mChunkVersion;
    std::deque<BlockPos> mPendingBlockEntities;
    unsigned char mPostProcessFlags;
    bool mLightingDropped;
    bool mOwnsBiomes;
};

#endif
