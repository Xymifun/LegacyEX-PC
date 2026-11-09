#ifndef GAME_WORLD_CHUNK_WATERLEVELCHUNK_HPP
#define GAME_WORLD_CHUNK_WATERLEVELCHUNK_HPP

#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/chunk/LevelChunk.hpp"

class AABB;
class Biome;
class BiomeSource;
class BlockEntity;
class BlockPos;
class BlockState;
class ChunkPrimer;
class Entity;
class Level;

class WaterLevelChunk : public LevelChunk
{
public:
    WaterLevelChunk(Level* level, ChunkPrimer* primer, int x, int z);

    virtual bool isAt(int x, int z);
    virtual void recalcBlockLights();
    virtual void recalcHeightmapOnly();
    virtual void recalcHeightmap();
    virtual void lightLava();
    virtual const BlockState* setBlock(const BlockPos& pos, const BlockState* state);
    virtual bool setBlockAndData(int x, int y, int z, int blockId, int data, bool update, bool extra);
    virtual void setExtraElement(int x, int y, int z, unsigned short element);
    virtual unsigned short getExtraElement(int x, int y, int z);
    virtual bool setData(int x, int y, int z, int data, int flags, bool* changed);
    virtual void setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness);
    void setLevelChunkBrightness(LightLayer::variety layer, int x, int y, int z, int brightness);
    virtual void addEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity, int section);
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
    virtual int countEntities();
    virtual bool shouldSave(bool force);
    virtual int setBlocksAndData(arrayWithLength<unsigned char> input, int x, int y, int z, int width, int height,
                                 int depth, int flags, bool extra);
    virtual Random* getRandom(long long seed);
    virtual Biome* getBiome(const BlockPos& pos, BiomeSource* source);
    virtual void setFromRawData(arrayWithLength<unsigned char> data);
    virtual void reSyncLighting();
    virtual void dropLighting();
};

#endif
