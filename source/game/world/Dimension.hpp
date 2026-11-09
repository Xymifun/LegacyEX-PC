#ifndef GAME_WORLD_DIMENSION_HPP
#define GAME_WORLD_DIMENSION_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/File.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"

class BiomeSource;
class ChunkGenerator;
class DimensionType;
class Level;
class LevelChunk;
class LevelType;
class OldChunkStorage;
class ServerPlayer;
class SuperflatConfig;
class Vec3;
class WorldBorder;

class Dimension
{
public:
    static const float MOON_BRIGHTNESS_PER_PHASE[8];

    Dimension();
    virtual void init(Level* level);
    virtual void init();
    virtual void updateLightRamp();
    virtual ~Dimension();
    virtual ChunkGenerator* createRandomLevelGenerator() const;
    virtual ChunkGenerator* createFlatLevelSource() const;
    virtual OldChunkStorage* createStorage(File file);
    virtual bool isValidSpawn(int x, int z) const;
    virtual float getTimeOfDay(long long time, float partialTick) const;
    virtual int getMoonPhase(long long time) const;
    virtual bool isNaturalDimension();
    virtual float* getSunriseColor(float celestialAngle, float timeOfDay);
    virtual Vec3* getFogColor(float celestialAngle, float timeOfDay);
    virtual bool mayRespawn() const;
    virtual float getCloudHeight();
    virtual bool hasGround();
    virtual BlockPos* getSpawnPos() const;
    virtual int getSpawnYPosition();
    virtual bool isFoggyAt(int x, int z);
    virtual WorldBorder* createWorldBorder(Level* level);
    virtual void saveData();
    virtual void tick();
    virtual bool canUnloadChunk(int x, int z);
    virtual const DimensionType* getType() const = 0;
    virtual int getXZSize();
    virtual AABB* getPlayerConstraints(const BlockPos& pos);
    virtual bool inPlayerConstraints(const BlockPos& pos);
    virtual bool containsChunk(int x, int z);
    virtual int getBlockLightColour();

    double getClearColorScale();
    BiomeSource* getBiomeSource();
    unsigned char isUltraWarm();
    unsigned char isHasCeiling();
    unsigned char isHasSkyLight();
    float* getBrightnessRamp();
    void onPlayerJoin(const boost::shared_ptr<ServerPlayer>& player);
    void onPlayerLeave(const boost::shared_ptr<ServerPlayer>& player);
    int getSeaLevel();

protected:
    friend class LevelChunk;
    Level* mLevel;
    LevelType* mLevelType;
    SuperflatConfig* mSuperflatConfig;
    BiomeSource* mBiomeSource;
    unsigned char mUltraWarm;
    unsigned char mHasCeiling;
    unsigned char mHasSkyLight;
    float* mBrightnessRamp;
    float mSunriseColor[4];
    int mSeaLevel;
    int mXZSize;
};

#endif
