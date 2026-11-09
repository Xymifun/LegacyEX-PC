#ifndef GAME_CORE_REGION_HPP
#define GAME_CORE_REGION_HPP

#include "game/world/LevelSource.hpp"
#include "game/world/chunk/array2DWithLength.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class BlockEntity;
class BlockState;
class Biome;
class BiomeSource;
class Direction;
class Dimension;
class EntityTickCache;
class Level;

class Region : public LevelSource
{
public:
    Region(Level* level, const BlockPos& from, const BlockPos& to, int expansion);
    virtual ~Region();

    virtual boost::shared_ptr<BlockEntity> getBlockEntity(const BlockPos& pos);
    boost::shared_ptr<BlockEntity> getBlockEntity(const BlockPos& pos, LevelChunk::EntityCreationType type);
    int getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness);
    int getRawBrightness(const BlockPos& pos);
    virtual float getBrightness(const BlockPos& pos, int lightValue);
    int getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos, int lightValue);
    virtual int getLightColor(const BlockPos& pos, int lightValue, int cacheIndex);
    virtual float getBrightness(const BlockPos& pos);
    virtual const BlockState* getBlockState(const BlockPos& pos);
    virtual const BlockState* getBlockState(EntityTickCache& tickCache, const BlockPos& pos);
    virtual const BlockState* getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos);
    virtual int getBlockId(int x, int y, int z);
    virtual int getBlockId(EntityTickCache& tickCache, int x, int y, int z);
    virtual void getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId);
    virtual unsigned short getElement(EntityTickCache& tickCache, int x, int y, int z);
    virtual unsigned short getElement(int x, int y, int z);
    virtual int getBlockData(int x, int y, int z);
    virtual unsigned short getExtraElement(int x, int y, int z);
    virtual void getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement);
    virtual Block* getBlock(const BlockPos& pos);
    virtual void getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock);
    virtual bool isSolidRender(const BlockPos& pos);
    virtual Biome* getBiome(const BlockPos& pos);
    virtual BiomeSource* getBiomeSource();
    virtual int getMaxBuildHeight();
    virtual bool isAllEmpty();
    virtual int getDirectSignal(const BlockPos& pos, const Direction* direction);
    virtual LevelType* getGeneratorType();
    virtual int getExtraBlockId(int x, int y, int z);
    virtual int getExtraBlockData(int x, int y, int z);
    virtual Block* getExtraBlock(const BlockPos& pos);
    virtual Block* getLiquidBlock(const BlockPos& pos);
    virtual const BlockState* getLiquidBlockState(const BlockPos& pos);
    virtual const BlockState* getExtraBlockState(const BlockPos& pos);
    virtual bool isEmptyBlock(const BlockPos& pos);
    virtual bool isEmptyWaterBlock(const BlockPos& pos);
    virtual int getBrightness(LightLayer::variety layer, const BlockPos& pos);
    void setCachedBlocksAndData(unsigned short* elements, int x, int y, int z, const BlockState** states);
    unsigned int elementToBlockId(unsigned short element);
    unsigned int elementToData(unsigned short element);
    LevelChunk* getLevelChunk(const BlockPos& pos)
    {
        int chunkX = (pos.x >> 4) - mMinChunkX;
        return mChunks->data[chunkX]->data[(pos.z >> 4) - mMinChunkZ];
    }

private:
    int mMinChunkX;
    int mMinChunkZ;
    array2DWithLength<LevelChunk*>* mChunks;
    Level* mLevel;
    bool mAllEmpty;
    int mCachedX;
    int mCachedY;
    int mCachedZ;
    const BlockState** mCachedStates;
};

#endif
