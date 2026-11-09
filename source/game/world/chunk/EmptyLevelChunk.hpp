#ifndef GAME_WORLD_CHUNK_EMPTYLEVELCHUNK_HPP
#define GAME_WORLD_CHUNK_EMPTYLEVELCHUNK_HPP

#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/chunk/LevelChunk.hpp"

class AABB;
class BlockPos;
class BlockState;
class Entity;

class EmptyLevelChunk : public LevelChunk
{
public:
    EmptyLevelChunk(Level* level, int x, int z);

    virtual bool isAt(int x, int z);
    virtual unsigned char getHeightmap(int x, int z);
    virtual void recalcHeightmapOnly();
    virtual void recalcHeightmap();
    virtual int getBlockLightBlock(const BlockPos& pos);
    virtual Block* getBlock(const BlockPos& pos);
    virtual const BlockState* getBlockState(const BlockPos& pos);
    virtual int getData(const BlockPos& pos);
    virtual int getBrightness(LightLayer::variety layer, const BlockPos& pos);
    virtual void getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z);
    virtual void setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness);
    virtual int getRawBrightness(const BlockPos& pos, int skyDarken);
    virtual void addEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity, int section);
    virtual bool isSkyLit(const BlockPos& pos);
    virtual boost::shared_ptr<BlockEntity> getBlockEntity(const BlockPos& pos, EntityCreationType type);
    virtual void addBlockEntity(boost::shared_ptr<BlockEntity> entity);
    virtual void setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity);
    virtual void removeBlockEntity(const BlockPos& pos);
    virtual void load(bool blockEntitiesOnly);
    virtual void unload(bool removeBlockEntities, bool removeEntities);
    virtual bool containsPlayer();
    virtual void markUnsaved();
    virtual void getEntities(boost::shared_ptr<Entity> except, const AABB* box,
                             std::vector<boost::shared_ptr<Entity> >& entities,
                             const Predicate<boost::shared_ptr<Entity> >* predicate);
    virtual void getEntitiesOfClass(const std::type_info& type, const AABB* box,
                                    std::vector<boost::shared_ptr<Entity> >& entities,
                                    const Predicate<boost::shared_ptr<Entity> >* predicate, bool exact);
    virtual bool shouldSave(bool force);
    virtual int getBlocksAndData(arrayWithLength<unsigned char>* output, int x, int y, int z, int width, int height,
                                 int depth, int offset, bool includeLight);
    virtual int setBlocksAndData(arrayWithLength<unsigned char> input, int x, int y, int z, int width, int height,
                                 int depth, int offset, bool includeLight);
    virtual Random* getRandom(long long seed);
    virtual bool isEmpty();
    virtual const BlockState* setBlock(const BlockPos& pos, const BlockState* state);
    virtual void getElementRange(arrayWithLength<unsigned short> elements, int start, int end);
    virtual void compressLighting();
    virtual void compressBlocks();
    virtual void compressData();
    virtual void setAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data);
    virtual void getAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data);
    virtual void getBlockData(arrayWithLength<unsigned char> data);
    virtual void getDataData(arrayWithLength<unsigned char> data);
    virtual void getSkyLightData(arrayWithLength<unsigned char> data);
    virtual void getBlockLightData(arrayWithLength<unsigned char> data);
    virtual void setSkyLightData(arrayWithLength<unsigned char> data);
    virtual void setBlockLightData(arrayWithLength<unsigned char> data);
    virtual void dropLighting();
};

#endif
