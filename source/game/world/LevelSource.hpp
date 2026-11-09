#ifndef GAME_WORLD_LEVELSOURCE_HPP
#define GAME_WORLD_LEVELSOURCE_HPP

#include <boost/shared_ptr.hpp>

class Biome;
class BiomeSource;
class Block;
class BlockEntity;
class BlockPos;
class BlockState;
class Direction;
class EntityTickCache;
class LevelChunk;
class LevelType;

class LightLayer
{
public:
    enum variety
    {
        BLOCK = 0,
        SKY = 15
    };
};

class LevelSource
{
public:
    virtual ~LevelSource() {}

    virtual boost::shared_ptr<BlockEntity> getBlockEntity(const BlockPos& pos) = 0;
    virtual int getLightColor(const BlockPos& pos, int lightValue, int cacheIndex) = 0;
    virtual int getBrightness(LightLayer::variety lightLayer, const BlockPos& pos) = 0;
    virtual float getBrightness(const BlockPos& pos, int lightValue) = 0;
    virtual float getBrightness(const BlockPos& pos) = 0;
    virtual const BlockState* getBlockState(const BlockPos& pos) = 0;
    virtual const BlockState* getBlockState(EntityTickCache& tickCache, const BlockPos& pos) = 0;
    virtual const BlockState* getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos) = 0;
    virtual int getBlockId(int x, int y, int z) = 0;
    virtual int getBlockId(EntityTickCache& tickCache, int x, int y, int z) = 0;
    virtual void getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId) = 0;
    virtual unsigned short getElement(EntityTickCache& tickCache, int x, int y, int z) = 0;
    virtual unsigned short getElement(int x, int y, int z) = 0;
    virtual int getBlockData(int x, int y, int z) = 0;
    virtual unsigned short getExtraElement(int x, int y, int z) = 0;
    virtual void getElementAndExtraElement(int x, int y, int z, unsigned short* element,
                                           unsigned short* extraElement) = 0;
    virtual Block* getBlock(const BlockPos& pos) = 0;
    virtual void getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock) = 0;
    virtual bool isSolidRender(const BlockPos& pos) = 0;
    virtual bool isEmptyBlock(const BlockPos& pos) = 0;
    virtual bool isEmptyWaterBlock(const BlockPos& pos) = 0;
    virtual Biome* getBiome(const BlockPos& pos) = 0;
    virtual BiomeSource* getBiomeSource() = 0;
    virtual int getMaxBuildHeight() = 0;
    virtual bool isAllEmpty() = 0;
    virtual int getDirectSignal(const BlockPos& pos, const Direction* direction) = 0;
    virtual LevelType* getGeneratorType() = 0;
    virtual int getExtraBlockId(int x, int y, int z) = 0;
    virtual int getExtraBlockData(int x, int y, int z) = 0;
    virtual Block* getExtraBlock(const BlockPos& pos) = 0;
    virtual Block* getLiquidBlock(const BlockPos& pos) = 0;
    virtual const BlockState* getLiquidBlockState(const BlockPos& pos) = 0;
    virtual const BlockState* getExtraBlockState(const BlockPos& pos) = 0;
};

#endif
