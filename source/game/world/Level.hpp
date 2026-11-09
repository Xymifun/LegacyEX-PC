#ifndef GAME_WORLD_LEVEL_HPP
#define GAME_WORLD_LEVEL_HPP

#include <vector>
#include <string>
#include <typeinfo>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/CustomSet.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/AABB.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockProperties.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/Chunk.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/GameRules.hpp"

class LevelStorage;
class LevelData;
class LootTables;
class Dimension;
class LevelChunk;
class ChunkSource;
class Biome;
class BiomeSource;
class LevelSettings;
class Block;
class BlockState;
class Direction;
class Entity;
class EntityTickCache;
class LevelListener;
class SoundEvent;
class ParticleType;
class Player;
class MinecraftServer;
class PathNavigationListener;
class Random;
class WorldBorder;
class SavedDataStorage;
class SavedData;
class HitResult;
class BlockEntity;
class CompoundTag;
class Packet;
struct PlayerUID;
class LevelGenerationOptions;
class LevelType;
class LevelSource;
class BiomeSource;
class Villages;
class Explosion;
class Scoreboard;
class TickNextTickData;

class Level : public LevelSource
{
    friend class LevelChunk;
    friend class ServerLevel;
    friend class DynamicLiquidBlock;
    friend class GenerationWriteTask;
    friend class VillageSiege;
    friend class VillagerGolem;

public:
    enum ESPAWN_TYPE
    {
        eSpawnType_Egg,
        eSpawnType_Breed,
        eSpawnType_Portal
    };
    class RangeModifier
    {
    public:
        virtual ~RangeModifier() {}
        virtual double apply(boost::shared_ptr<Player> player) const = 0;
    };

    static const int maxBuildHeight = 256;

    Level(boost::shared_ptr<LevelStorage> levelStorage, LevelData* levelData, Dimension* dimension, bool isClientSide);
    ~Level();
    void _init();
    bool isWaterAt(const BlockPos& pos);
    bool shouldFreeze(const BlockPos& pos, bool checkNeighbours);
    bool shouldFreezeIgnoreNeighbors(const BlockPos& pos);
    long long getSeed();
    LevelData* getLevelData();
    void clampWithinLevelBounds(double& x, double& z, double margin);
    bool isWithinLevelBounds(int x, int z);
    void clampWithinLevelHeight(double& y, double margin);
    static void enableLightingCache();
    static void destroyLightingCache();
    static bool getInstaTick();
    static void setInstaTick(bool instaTick);
    bool hasEntitiesToRemove();
    virtual Level* postConstruct();
    virtual ChunkSource* createChunkSource() = 0;
    LevelChunk* getChunk(int chunkX, int chunkZ);
    LevelChunk* getChunkAt(const BlockPos& pos);
    bool hasChunkAt(const BlockPos& pos, bool allowEmpty);
    bool hasChunkAt(const BlockPos& pos);
    virtual Biome* getBiome(const BlockPos& pos);
    virtual BiomeSource* getBiomeSource();
    virtual void initializeLevel(LevelSettings* settings);
    virtual MinecraftServer* getServer();
    void setSpawnPos(const BlockPos& pos);
    virtual void validateSpawn();
    int getSeaLevel();
    const BlockState* getTopBlockState(const BlockPos& pos);
    bool isInWorldBounds(const BlockPos& pos);
    bool isInWorldBounds(int x, int y, int z);
    bool isOutsideBuildHeight(const BlockPos& pos);
    virtual bool isEmptyBlock(const BlockPos& pos);
    bool isEmptyWaterBlock(const BlockPos& pos);
    bool hasChunksAt(const BlockPos& pos, int radius, bool allowEmpty);
    bool hasChunksAt(const BlockPos& pos, int radius);
    bool hasChunksAt(const BlockPos& from, const BlockPos& to, bool allowEmpty);
    bool hasChunksAt(const BlockPos& from, const BlockPos& to);
    bool hasChunksAt(BoundingBox box, bool allowEmpty);
    bool hasChunksAt(BoundingBox box);
    bool reallyHasChunk(int chunkX, int chunkZ);
    bool reallyHasChunkAt(const BlockPos& pos);
    bool reallyHasChunksAt(const BlockPos& from, const BlockPos& to);
    bool reallyHasChunksAt(const BlockPos& pos, int radius);
    virtual bool hasChunksAt(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool allowEmpty);
    virtual bool hasChunk(int chunkX, int chunkZ, bool allowEmpty) = 0;
    virtual LevelChunk* getChunkIfHasChunk(int chunkX, int chunkZ, bool allowEmpty) = 0;
    LevelChunk* getChunkAt(int x, int z);
    LevelChunk* getChunkLoadedOrUnloadedAt(int x, int z);
    virtual bool isChunkGeneratedAt(int chunkX, int chunkZ);
    virtual bool setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    virtual bool setExtraBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    void setBlockBrightnessChanged(const BlockPos& pos);
    void setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness, bool markDirty);
    int getBrightnessCached(unsigned int* cache, LightLayer::variety layer, int x, int y, int z);
    bool canSeeSky(const BlockPos& pos);
    int getEmissionCached(unsigned int* cache, Block* block, const BlockPos& pos);
    int getExpectedLight(unsigned int* cache, const BlockPos& pos, LightLayer::variety layer, bool ignoreEmission);
    int getBlockingCached(unsigned int* cache, LightLayer::variety layer, Block* block, Block* extraBlock,
                          const BlockPos& pos);
    void initCachePartial(unsigned int* cache, int x, int y, int z);
    void initCacheComplete(unsigned int* cache, int x, int y, int z);
    void setBrightnessCached(unsigned int* cache, unsigned long long* dirty, LightLayer::variety layer, int x, int y,
                             int z, int brightness);
    void setBlocksDirty(const BlockPos& from, const BlockPos& to);
#ifdef MINECRAFT_PC
    void flushCache(unsigned int* cache, unsigned long long dirty, LightLayer::variety layer, int referenceX,
                    int referenceZ);
#else
    void flushCache(unsigned int* cache, unsigned long long dirty, LightLayer::variety layer);
#endif
    bool checkLight(LightLayer::variety layer, const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre);
    bool checkLight(const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre);
    void neighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos);
    void observedNeighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos);
    void updateObservingBlocksAt(const BlockPos& pos, Block* block);
    void updateNeighborsAt(const BlockPos& pos, Block* block, bool updateObservers);
    void updateNeighborsOfNeighborsAt(const BlockPos& pos, Block* block);
    void blockUpdated(const BlockPos& pos, Block* block, bool updateObservers);
    virtual bool internalSetBlock(bool extraBlock, const BlockPos& pos, const BlockState* state, int flags,
                                  bool forceUpdate);
    virtual bool setData(const BlockPos& pos, int data, int flags, bool forceUpdate);
    virtual bool setData(int x, int y, int z, int data, int flags, bool forceUpdate);
    bool setBlockAndData(int x, int y, int z, int blockId, int data, int flags);
    virtual bool removeBlock(const BlockPos& pos);
    void levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data);
    void levelEvent(int type, const BlockPos& pos, int data);
    virtual bool destroyBlock(const BlockPos& pos, bool dropResources);
    virtual bool setBlockAndUpdate(const BlockPos& pos, const BlockState* state);
    virtual void sendBlockUpdated(const BlockPos& pos, const BlockState* oldState, const BlockState* newState,
                                  int flags, bool immediate);
    void playMusic(double x, double y, double z, const std::wstring& sound, float volume);
    Vec3* getSkyColor(boost::shared_ptr<Entity> entity, float partialTick);
    Vec3* getFogColor(boost::shared_ptr<Entity> entity, float partialTick);
    BlockPos getTopSolidBlockPos(const BlockPos& pos);
    int getAboveTopSolidBlock(const BlockPos& pos, bool checkLiquid, bool checkSolid);
    void lightColumnChanged(int x, int z, int fromY, int toY);
    void updateNeighborsAtExceptFromFacing(const BlockPos& pos, Block* block, const Direction* skipSide);
    virtual bool isBlockToBeTickedAt(const BlockPos& pos, Block* block);
    bool canSeeSkyFromBelowWater(const BlockPos& pos);
    int getDaytimeRawBrightness(const BlockPos& pos);
    int getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness);
    int getRawBrightness(const BlockPos& pos);
    int getHeightmapHeight(int x, int z);
    BlockPos getHeightmapPos(const BlockPos& pos);
    int getHeightmap(int x, int z);
    int getLowestHeightmap(int x, int z);
    int getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos);
    virtual void getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z);
    int getBrightness(LightLayer::variety layer, const BlockPos& pos);
    int getLightColor(const BlockPos& pos, int lightValue, int cacheIndex);
    float getBrightness(const BlockPos& pos, int lightValue);
    float getBrightness(const BlockPos& pos);
    virtual const BlockState* getBlockState(const BlockPos& pos);
    virtual const BlockState* getBlockState(EntityTickCache& tickCache, const BlockPos& pos);
    virtual const BlockState* getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos);
    virtual int getBlockId(int x, int y, int z);
    virtual int getBlockId(EntityTickCache& tickCache, int x, int y, int z);
    virtual void getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId);
    virtual unsigned short getElement(EntityTickCache& tickCache, int x, int y, int z);
    virtual unsigned short getElement(int x, int y, int z);
    unsigned short getExtraElement(int x, int y, int z);
    void getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement);
    bool sectionContainsBlockId(int x, int y, int z, int blockId);
    int getExtraBlockId(int x, int y, int z);
    Block* getExtraBlock(const BlockPos& pos);
    void getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock);
    Block* getLiquidBlock(const BlockPos& pos);
    const BlockState* getLiquidBlockState(const BlockPos& pos);
    virtual const BlockState* getExtraBlockState(const BlockPos& pos);
    virtual int getBlockData(int x, int y, int z);
    int getExtraBlockData(int x, int y, int z);
    Block* getBlock(const BlockPos& pos);
    bool isSolidRender(const BlockPos& pos);
    bool isDay();
    HitResult* clip(Vec3* from, Vec3* to, bool stopOnLiquid, bool ignoreBlocksWithoutShape,
                    bool returnLastUncollidableBlock, bool testUseBlocks,
                    boost::unordered_set<Block*, boost::hash<Block*>, std::equal_to<Block*>, std::allocator<Block*> >*
                        alwaysClipBlocks,
                    bool clipExtraBlocks);
    HitResult* clip(Vec3* from, Vec3* to);
    HitResult* clip(Vec3* from, Vec3* to, bool stopOnLiquid);
    void playSoundToAllPlayers(const BlockPos& pos, const SoundEvent* sound, SoundSource::ESoundSource source,
                               float volume, float pitch, float range);
    virtual void playSound(boost::shared_ptr<Player> player, const BlockPos& pos, const SoundEvent* sound,
                           SoundSource::ESoundSource source, float volume, float pitch, float range);
    virtual void playSound(boost::shared_ptr<Player> player, double x, double y, double z, const SoundEvent* sound,
                           SoundSource::ESoundSource source, float volume, float pitch, float range);
    virtual void playLocalSound(double x, double y, double z, const SoundEvent* sound, SoundSource::ESoundSource source,
                                float volume, float pitch, bool distanceDelay, float range);
    void playStreamingMusic(const BlockPos& pos, const SoundEvent* sound, bool playing);
    void addParticle(ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya,
                     double za, arrayWithLength<int> params);
    void addParticle(const ParticleType* type, double x, double y, double z, double xa, double ya, double za,
                     arrayWithLength<int> params);
    void addAlwaysVisibleParticle(ePARTICLE_TYPE type, double x, double y, double z, double xa, double ya, double za,
                                  arrayWithLength<int> params);
    void addParticle(const ParticleType* type, bool overrideLimiter, double x, double y, double z, double xa, double ya,
                     double za, arrayWithLength<int> params);
    virtual bool addGlobalEntity(boost::shared_ptr<Entity> entity);
    virtual bool addEntity(boost::shared_ptr<Entity> entity);
    virtual void entityAdded(boost::shared_ptr<Entity> entity);
    virtual void entityRemoved(boost::shared_ptr<Entity> entity);
    virtual void playerRemoved(boost::shared_ptr<Entity> player);
    virtual void removeEntity(boost::shared_ptr<Entity> entity);
    void removeEntityImmediately(boost::shared_ptr<Entity> entity);
    virtual bool hasTickNextTick(const BlockPos& pos, Block* block);
    virtual void addToTickNextTick(const BlockPos& pos, Block* block, int delay);
    virtual void addToTickNextTick(const BlockPos& pos, Block* block, int delay, int priority);
    virtual void forceAddBlockTick(const BlockPos& pos, Block* block, int delay, int priority);
    void addListener(LevelListener* listener);
    void removeListener(LevelListener* listener);
    bool isInsideWorldBorder(boost::shared_ptr<Entity> entity);
    bool internalGetCollisionAABBs(boost::shared_ptr<Entity> entity, const AABB* box, bool stopOnFirst,
                                   std::vector<AABB*>* collisions, bool noEntities, bool includePlayerConstraints,
                                   bool forParticles);
    std::vector<boost::shared_ptr<Entity> >* getEntities(boost::shared_ptr<Entity> excluded, const AABB* box,
                                                         const Predicate<boost::shared_ptr<Entity> >* predicate,
                                                         std::vector<boost::shared_ptr<Entity> >* entities);
    std::vector<boost::shared_ptr<Entity> >* getEntities(boost::shared_ptr<Entity> excluded, const AABB* box,
                                                         std::vector<boost::shared_ptr<Entity> >* entities);
    std::vector<AABB*>* getCollisionAABBs(boost::shared_ptr<Entity> entity, const AABB* box, bool noEntities,
                                          bool includePlayerConstraints, bool forParticles);
    bool hasBlockCubes(const AABB* box);
    bool checkAndHandleWater(AABB* box, Material* material, boost::shared_ptr<Entity> entity);
    float getTimeOfDay(float partialTick);
    float getRainLevel(float partialTick);
    float getThunderLevel(float partialTick);
    int getOldSkyDarken(float partialTick);
    float getSkyDarken(float partialTick);
    int getMoonPhase();
    float getMoonBrightness();
    float getSunAngle(float partialTick);
    Vec3* getCloudColor(float partialTick);
    bool biomeHasRain(const BlockPos& pos);
    bool biomeHasSnow(const BlockPos& pos);
    float getStarBrightness(float partialTick);
    const Difficulty* getDifficulty();
    DifficultyInstance getCurrentDifficultyAt(const BlockPos& pos);
    int getSkyDarken();
    void setSkyDarken(int value);
    int getSkyFlashTime();
    void setSkyFlashTime(int value);
    bool isFindingSpawn();
    void setFindingSpawn(bool value);
    GameRules* getGameRules();
    int getMaxBuildHeight();
    int getHeight();
    WorldBorder* getWorldBorder();
    int getCloudHeight();
    int getFogDistance();
    void setFogDistance(int value);
    bool useNewSeaLevel();
    bool isFlatLand();
    bool isSpawnChunk(int chunkX, int chunkZ);
    bool addBlockEntity(boost::shared_ptr<BlockEntity> entity);
    void AddToBlockEntityList(boost::shared_ptr<BlockEntity> entity);
    virtual void tickEntities();
    virtual void tickServerPlayers();
    void addAllPendingBlockEntities(std::vector<boost::shared_ptr<BlockEntity> >& entities);
    virtual void tick(boost::shared_ptr<Entity> entity, bool updateChunk);
    void tick(boost::shared_ptr<Entity> entity);
    bool isUnobstructed(const AABB* box, boost::shared_ptr<Entity> entity);
    bool isUnobstructed(const AABB* box);
    bool containsAnyBlocks(AABB* box);
    bool containsAnyLiquid(AABB* box);
    bool containsAnysolidBlocking(AABB* box);
    bool containsAnyLiquid_NoLoad(AABB* box);
    bool containsFireBlock(AABB* box);
    bool containsFireBlock(Entity* entity, AABB* box);
    bool containsMaterial(AABB* box, Material* material);
    bool isInLava(Entity* entity, AABB* box);
    bool containsLiquid(AABB* box, Material* material);
    bool canProvideSupport(const BlockPos& pos, const Direction* direction, EBlockSupportType supportType);
    bool isUnderWater(const BlockPos& pos, const BlockState* state);
    bool isUnderWater(const BlockPos& pos, int x, int z);
    bool isTouchingMaterial(const BlockPos& pos, const Material* material);
    double getSeenPercent(Vec3* source, AABB* box);
    bool extinguishFire(boost::shared_ptr<Player> player, const BlockPos& pos, const Direction* direction);
    std::wstring gatherChunkSourceStats();
    boost::shared_ptr<BlockEntity> getPendingBlockEntityAt(const BlockPos& pos);
    virtual boost::shared_ptr<BlockEntity> getBlockEntity(const BlockPos& pos);
    void setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity);
    void removeBlockEntity(const BlockPos& pos);
    void markForRemoval(boost::shared_ptr<BlockEntity> entity);
    bool isFullAABBBlock(const BlockPos& pos);
    bool isSolidBlockingBlockInLoadedChunk(const BlockPos& pos, bool allowEmpty);
    bool isSolidBlockingBlock(const BlockPos& pos);
    bool isConsideredSolidBlock(int x, int y, int z);
    bool isConsideredSolidBlock(const BlockPos& pos);
    void updateSkyBrightness();
    virtual void tick();
    virtual boost::shared_ptr<Explosion> explode(boost::shared_ptr<Entity> source, double x, double y, double z,
                                                 float radius, bool fire, bool breaksBlocks, bool playersOnly,
                                                 bool noBlockDrops, bool silent, float damageRadius);
    virtual void blockEntityAddedExtra(boost::shared_ptr<BlockEntity> entity);
    virtual void blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> entity);
    void prepareWeather();
    virtual void tickWeather(bool blockEvents);
    virtual void buildAndPrepareChunksToPoll();
    virtual int getChunkPollRange() = 0;
    virtual void tickClientSideBlocks(int x, int z, LevelChunk* chunk);
    virtual void tickBlocks();
    void instaTick(const BlockPos& pos, const BlockState* state, Random& random);
    bool shouldFreeze(const BlockPos& pos);
    int calcSnowBlockDepth(const BlockPos& pos, int depth);
    bool canPlaceSnowLayer(const BlockPos& pos, bool checkNeighbors, bool allowExisting, int* depth);
    bool tryToPlaceTopSnow(const BlockPos& pos, bool checkNeighbors, bool allowExisting);
    virtual bool tickPendingTicks(bool runAll);
    virtual std::vector<TickNextTickData>* fetchTicksInChunk(LevelChunk* chunk, bool runAll);
    virtual std::vector<TickNextTickData>* fetchTicksInArea(BoundingBox* box, bool runAll);
    std::vector<boost::shared_ptr<Player> >* getPlayers(unsigned long long flags,
                                                        const Predicate<boost::shared_ptr<Player> >* predicate);
    std::vector<boost::shared_ptr<Entity> >* getEntitiesOfClass(const std::type_info& type, const AABB* box,
                                                                const Predicate<boost::shared_ptr<Entity> >* predicate,
                                                                bool excludeInvisiblePlayers);
    std::vector<boost::shared_ptr<Entity> >* getEntitiesOfClass(const std::type_info& type, const AABB* box);
    bool hasEntitiesOfClass(const std::type_info& type, const AABB* box,
                            const Predicate<boost::shared_ptr<Entity> >* predicate, bool excludeInvisiblePlayers);
    boost::shared_ptr<Entity> getClosestEntityOfClass(const std::type_info& type, const AABB* box,
                                                      boost::shared_ptr<Entity> excluded);
    virtual boost::shared_ptr<Entity> getEntity(int id);
    std::vector<boost::shared_ptr<Entity> > getAllEntities();
    void blockEntityChanged(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity);
    int countInstanceOf(unsigned long long type, bool exact, unsigned int* count, unsigned int* result);
    int countInstanceOfInRange(unsigned long long type, bool exact, int range, int x, int y, int z);
    virtual void addEntities(std::vector<boost::shared_ptr<Entity> >* entities);
    virtual void removeEntities(std::vector<boost::shared_ptr<Entity> >* entities);
    bool mayPlace(Block* block, const BlockPos& pos, bool replaceable, const Direction* direction,
                  boost::shared_ptr<Entity> entity);
    boost::shared_ptr<Player> getNearestPlayerPredicate(boost::shared_ptr<Entity> entity, double distance,
                                                        const Predicate<boost::shared_ptr<Entity> >* predicate);
    boost::shared_ptr<Player> getNearestPlayerPredicate(double x, double y, double z, double distance,
                                                        const Predicate<boost::shared_ptr<Entity> >* predicate);
    boost::shared_ptr<Player> getNearestPlayer(double x, double y, double z, double distance, bool includeCreative,
                                               double maxYDistance);
    std::vector<boost::shared_ptr<Player> >* getNearbyPlayers(boost::shared_ptr<Entity> entity, double distance);
    bool hasNearPlayer(double x, double y, double z, double distance);
    boost::shared_ptr<Player> getNearestPlayer(double x, double z, double distance);
    boost::shared_ptr<Player> getNearestAttackablePlayer(double x, double y, double z, double distance,
                                                         double maxYDistance, const RangeModifier* modifier,
                                                         const Predicate<boost::shared_ptr<Entity> >* predicate);
    boost::shared_ptr<Player> getNearestAttackablePlayer(const BlockPos& pos, double distance, double angle);
    boost::shared_ptr<Player> getPlayerByName(const std::wstring& name);
    boost::shared_ptr<Player> getPlayerByUUID(const std::wstring& uuid);
    arrayWithLength<unsigned char> getBlocksAndData(int minX, int minY, int minZ, int maxX, int maxY, int maxZ,
                                                    bool includeLight);
    void setBlocksAndData(int minX, int minY, int minZ, int maxX, int maxY, int maxZ,
                          arrayWithLength<unsigned char> data, bool includeLight);
    void setFromRawData(int x, int z, arrayWithLength<unsigned char> data);
    virtual void disconnect(bool save);
    void checkSession();
    void ensureAdded(boost::shared_ptr<Entity> entity);
    virtual bool mayInteract(boost::shared_ptr<Player> player, const BlockPos& pos, Block* block);
    virtual void broadcastEntityEvent(boost::shared_ptr<Entity> entity, unsigned char event, int data);
    virtual void blockEvent(const BlockPos& pos, Block* block, int event, int data);
    virtual void updateSleepingPlayerList();
    int getSaveVersion();
    int getOriginalSaveVersion();
    bool isHumidAt(const BlockPos& pos);
    virtual void createFireworks(double x, double y, double z, double vx, double vy, double vz, CompoundTag* tag);
    virtual Scoreboard* GetScoreboard() = 0;
    virtual void updateNeighbourForOutputSignal(const BlockPos& pos, Block* block);
    boost::shared_ptr<Villages> getVillages();
    float getSpecialMultiplier(int type);
    virtual void sendPacketToServer(boost::shared_ptr<Packet> packet);
    LootTables* getLootTables();
    virtual BlockPos* findNearestMapFeature(const std::wstring& feature, const BlockPos& pos, bool findUnexplored);
    int getAuxValueForMap(PlayerUID uid, int dimension, const BlockPos& pos, int scale);
    LevelChunk* getCacheChunkDirect(int x, int z);
    bool isChunkPostPostProcessed(int x, int z);
    bool isChunkFinalised(int x, int z);
    void refreshLevelInfoFromLevelGen();
    void refreshFromLevelGen();
    bool canCreateMore(unsigned long long type, ESPAWN_TYPE spawnType);
    void removeAllEntities(bool players, bool mobs, bool items, bool vehicles, int x, int y, int z, int radius);
    virtual bool newPrimedTntAllowed();
    virtual bool newFallingBlockAllowed();
    virtual bool explodeNextEnderCrystal();
    virtual bool AllPlayersAreSleeping();
    bool getHasBeenInCreative();
    bool isGenerateMapFeatures();
    BlockPos getTopRainBlockPos(const BlockPos& pos);
    bool isSnowTemperature(const BlockPos& pos);
    bool isRainingAt(const BlockPos& pos);
    bool isSnowingAt(const BlockPos& pos);
    void setThunderLevel(float value);
    void setRainLevel(float value);
    bool isThundering();
    bool isRaining();
    void setSavedDataStorage(SavedDataStorage* storage);
    SavedDataStorage* getSavedDataStorage();
    void setSavedData(const std::wstring& id, boost::shared_ptr<SavedData> data);
    boost::shared_ptr<SavedData> getSavedData(const std::type_info& type, const std::wstring& id);
    int getFreeAuxValueFor(const std::wstring& id);
    int getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale);
    void globalLevelEvent(int type, const BlockPos& pos, int data);
    Random* getRandomFor(int x, int y, int z);
    static long long fjGetRandomFor(long long seed, int x, int y, int z);
    bool isAllEmpty();
    double getHorizonHeight();
    void destroyBlockProgress(int breakerId, const BlockPos& pos, int progress);
    void setSeaLevel(int value);
    LevelType* getGeneratorType();
    void setSpawnSettings(bool spawnEnemies, bool spawnFriendlies);
    int getDirectSignal(const BlockPos& pos, const Direction* direction);
    int getDirectSignalTo(const BlockPos& pos);
    int getSignal(const BlockPos& pos, const Direction* direction);
    bool hasSignal(const BlockPos& pos, const Direction* direction);
    bool hasNeighborSignal(const BlockPos& pos);
    int getBestNeighborSignal(const BlockPos& pos);
    boost::shared_ptr<Player> getNearestPlayer(boost::shared_ptr<Entity> entity, double distance, bool includeCreative,
                                               double maxYDistance);
    boost::shared_ptr<Player> getNearestPlayer(boost::shared_ptr<Entity> entity, double distance, double maxYDistance);
    boost::shared_ptr<Player> getNearestPlayerNotCreative(boost::shared_ptr<Entity> entity, double distance,
                                                          double maxYDistance);
    void setGameTime(long long value);
    long long getGameTime();
    long long getDayTime();
    void setDayTime(long long value);
    void forceDayTime(long long value);
    bool ShouldForceTimeUpdate();
    BlockPos getSharedSpawnPos();
    ChunkSource* getChunkSource();
    PathNavigationListener* getPathNavigationListener();
    LevelStorage* getLevelStorage();
    unsigned char getWeatherState();
    bool isWithinSpawnChunks(const BlockPos& pos);
    int getUnsavedChunkCount();
    void incrementUnsavedChunkCount();
    void decrementUnsavedChunkCount();
    void gatherStats();
    boost::shared_ptr<Player> getNearestAttackablePlayer(boost::shared_ptr<Entity> entity, double distance,
                                                         double angle);
    int mSeaLevel;

protected:
    OSFastMutex mEntityMutex;
    std::vector<boost::shared_ptr<Entity> > mEntities;
    std::vector<boost::shared_ptr<Entity> > mEntitiesToRemove;
    bool mScheduledUpdatesAreImmediate;
    OSFastMutex mBlockEntityMutex;
    std::vector<boost::shared_ptr<BlockEntity> > mBlockEntities;
    std::vector<boost::shared_ptr<BlockEntity> > mTickingBlockEntities;
    std::vector<boost::shared_ptr<BlockEntity> > mPendingBlockEntities;
    std::vector<boost::shared_ptr<BlockEntity> > mBlockEntitiesToRemove;

public:
    std::vector<boost::shared_ptr<Player> > mPlayers;
    std::vector<boost::shared_ptr<Entity> > mGlobalEntities;

protected:
    boost::unordered_map<int, boost::shared_ptr<Entity>, IntKeyHash2, IntKeyEq> mEntityMap;

private:
    int mCloudColour;
    int mSkyDarken;
    bool mForceTimeUpdate;

protected:
    int mRandValue;
    LootTables* mLootTables;
    int mAddend;
    float mOldRainLevel;
    float mRainLevel;
    float mOldThunderLevel;
    float mThunderLevel;

private:
    int mSkyFlashTime;

public:
    Random* mRandom;

public:
    bool mIsNew;
    Dimension* mDimension;

private:
    PathNavigationListener* mPathListener;

public:
    std::vector<LevelListener*> mListeners;

protected:
    ChunkSource* mChunkSource;
    boost::shared_ptr<LevelStorage> mLevelStorage;
    LevelData* mLevelData;
    bool mFindingSpawn;
    SavedDataStorage* mSavedDataStorage;
    boost::shared_ptr<Villages> mVillages;

public:
    bool mIsClientSide;

protected:
    CustomSet mChunksToPoll;

private:
    bool mSpawnEnemies;
    bool mSpawnFriendlies;
    bool mProcessingBlockEntities;
    WorldBorder* mWorldBorder;
    bool mHasDirtyRegion;
    int mDirtyMinX;
    int mDirtyMaxX;
    int mDirtyMinY;
    int mDirtyMaxY;
    int mDirtyMinZ;
    int mDirtyMaxZ;
    std::vector<AABB*> mCollisionBoxes;
    unsigned int* mLightUpdateQueue;
    std::vector<boost::shared_ptr<Entity> > mEntityQueryResult;

protected:
    LevelChunk** mChunkCache;
    int mChunkCacheSize;

private:
    OSFastMutex mLightMutex;
    int mAmbientTickCountdown;
    int mFogDistance;
};

#endif
