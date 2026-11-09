#ifndef GAME_WORLD_THEENDDIMENSION_HPP
#define GAME_WORLD_THEENDDIMENSION_HPP

#include "game/world/Dimension.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class AABB;
class BlockPos;
class ChunkGenerator;
class DimensionType;
class EndDragonFight;

class TheEndDimension : public Dimension
{
public:
    TheEndDimension();
    virtual ~TheEndDimension();
    EndDragonFight* getDragonFight();
    virtual int getXZSize();
    virtual void init();
    virtual ChunkGenerator* createRandomLevelGenerator() const;
    virtual float getTimeOfDay(long long time, float partialTick) const;
    virtual float* getSunriseColor(float celestialAngle, float timeOfDay);
    virtual Vec3* getFogColor(float celestialAngle, float timeOfDay);
    virtual bool hasGround();
    virtual bool mayRespawn() const;
    virtual bool isNaturalDimension();
    virtual float getCloudHeight();
    virtual bool isValidSpawn(int x, int z) const;
    virtual BlockPos* getSpawnPos() const;
    virtual int getSpawnYPosition();
    virtual bool isFoggyAt(int x, int z);
    virtual const DimensionType* getType() const;
    virtual void saveData();
    virtual void tick();
    static ChunkPos* closestAreaToPoint(int x, int z);
    static ChunkPos AREAS[2];
    virtual AABB* getPlayerConstraints(const BlockPos& pos);
    virtual bool containsChunk(int x, int z);
    virtual int getBlockLightColour();

private:
    EndDragonFight* mDragonFight;
    char* mChunkData;
    bool mModernEnd;
};

#endif
