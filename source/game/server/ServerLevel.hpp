#ifndef GAME_SERVER_SERVERLEVEL_HPP
#define GAME_SERVER_SERVERLEVEL_HPP

#include <cstddef>
#include <list>
#include <set>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "game/client/render/ParticleType.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/TickNextTickData.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockEventData.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class BlockEntity;
class BlockState;
class BoundingBox;
class ChunkSource;
class ClientboundLevelParticlesPacket;
class Entity;
class EntityTracker;
class Explosion;
class LevelChunk;
class LevelData;
class LevelSettings;
class LevelStorage;
class MinecraftServer;
class MobCategory;
class NaturalMobSpawner;
class Player;
class PlayerChunkMap;
class PortalForcer;
class ProgressListener;
class Scoreboard;
class ServerLevelListener;
class ServerPlayer;
class ServerScoreboard;
class VillageSiege;
struct PlayerUID;

struct TickNextTickDataKeyCompare
{
    bool operator()(const TickNextTickData& first, const TickNextTickData& second) const
    {
        return TickNextTickData::compare_fnct(first, second);
    }
};

struct TickNextTickDataKeyHash
{
    std::size_t operator()(const TickNextTickData& data) const { return TickNextTickData::hash_fnct(data); }
};

struct TickNextTickDataKeyEq
{
    bool operator()(const TickNextTickData& first, const TickNextTickData& second) const
    {
        return TickNextTickData::eq_test(first, second);
    }
};

class ServerLevel : public Level
{
    friend class PlayerList;

public:
    struct QueuedSendBlockData
    {
        BlockPos pos;
        const BlockState* oldState;
        const BlockState* newState;

        QueuedSendBlockData(int x, int y, int z, const BlockState* previousState, const BlockState* currentState)
        {
            pos = BlockPos(x, y, z);
            oldState = previousState;
            newState = currentState;
        }
    };

    ServerLevel(MinecraftServer* server, boost::shared_ptr<LevelStorage> levelStorage, LevelData* levelData,
                int dimension);
    virtual ~ServerLevel();

    static void staticCtor();
    static int runUpdate(void* param);

    virtual Level* postConstruct();
    virtual ChunkSource* createChunkSource();
    virtual void initializeLevel(LevelSettings* settings);
    virtual MinecraftServer* getServer();
    virtual void validateSpawn();
    virtual bool hasChunk(int chunkX, int chunkZ, bool allowEmpty);
    virtual LevelChunk* getChunkIfHasChunk(int chunkX, int chunkZ, bool allowEmpty);
    virtual bool setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    virtual bool isBlockToBeTickedAt(const BlockPos& pos, Block* block);
    virtual bool addGlobalEntity(boost::shared_ptr<Entity> entity);
    virtual bool addEntity(boost::shared_ptr<Entity> entity);
    virtual void entityAdded(boost::shared_ptr<Entity> entity);
    virtual void entityRemoved(boost::shared_ptr<Entity> entity);
    virtual bool hasTickNextTick(const BlockPos& pos, Block* block);
    virtual void addToTickNextTick(const BlockPos& pos, Block* block, int delay);
    virtual void addToTickNextTick(const BlockPos& pos, Block* block, int delay, int priority);
    virtual void forceAddBlockTick(const BlockPos& pos, Block* block, int delay, int priority);
    virtual void tickEntities();
    virtual void tickServerPlayers();
    virtual void tick(boost::shared_ptr<Entity> entity, bool updateChunk);
    virtual void tick();
    virtual boost::shared_ptr<Explosion> explode(boost::shared_ptr<Entity> source, double x, double y, double z,
                                                 float radius, bool fire, bool breaksBlocks, bool playersOnly,
                                                 bool noBlockDrops, bool silent, float damageRadius);
    virtual void blockEntityAddedExtra(boost::shared_ptr<BlockEntity> blockEntity);
    virtual void blockEntityRemovedExtra(boost::shared_ptr<BlockEntity> blockEntity);
    virtual void tickWeather(bool blockEvents);
    virtual int getChunkPollRange();
    virtual void tickBlocks();
    virtual bool tickPendingTicks(bool runAll);
    virtual std::vector<TickNextTickData>* fetchTicksInChunk(LevelChunk* chunk, bool removeTicks);
    virtual std::vector<TickNextTickData>* fetchTicksInArea(BoundingBox* box, bool removeTicks);
    virtual boost::shared_ptr<Entity> getEntity(int id);
    virtual void addEntities(std::vector<boost::shared_ptr<Entity> >* entities);
    virtual bool mayInteract(boost::shared_ptr<Player> player, const BlockPos& pos, Block* block);
    virtual void broadcastEntityEvent(boost::shared_ptr<Entity> entity, unsigned char event, int data);
    virtual void blockEvent(const BlockPos& pos, Block* block, int event, int data);
    virtual void updateSleepingPlayerList();
    virtual Scoreboard* GetScoreboard();
    virtual BlockPos* findNearestMapFeature(const std::wstring& feature, const BlockPos& pos, bool findUnexplored);
    virtual bool newPrimedTntAllowed();
    virtual bool newFallingBlockAllowed();
    virtual bool explodeNextEnderCrystal();
    virtual bool AllPlayersAreSleeping();
    virtual void setInitialSpawn(LevelSettings* settings);
    virtual void saveLevelData();

    EntityTracker* getTracker();
    ChunkSource* getChunkSource();
    std::vector<boost::shared_ptr<BlockEntity> >* getBlockEntitiesInRegion(int x0, int y0, int z0, int x1, int y1,
                                                                           int z1);
    PlayerChunkMap* getChunkMap();
    BlockPos* getDimensionSpecificSpawn();
    PortalForcer* getPortalForcer();
    bool allPlayersAreSleeping();
    void stopWeather();
    void awakenAllPlayers();
    void save(bool force, ProgressListener* progressListener, bool autosave);
    bool doBlockEvent(BlockEventData* event);
    void runBlockEvents();
    void runQueuedSendBlockUpdates();
    Biome::MobSpawnerData* getRandomMobSpawnAt(MobCategory* category, const BlockPos& pos);
    bool canMobSpawnAt(MobCategory* category, Biome::MobSpawnerData* spawnerData, const BlockPos& pos);
    BlockPos findLightingTargetAround(const BlockPos& pos);
    void resetEmptyTime();
    bool isAnimalsAllowed();
    bool isNpcsAllowed();
    void forceTick(boost::shared_ptr<Entity> entity, bool updateChunk);
    void setupDebugLevel();
    void generateBonusItemsNearSpawn();
    void Suspend();
    void saveToDisc(ProgressListener* progressListener, bool autosave);
    bool removeDuplicateAndPendingEntity(const boost::shared_ptr<Entity>& entity);
    void entityAddedExtra(boost::shared_ptr<Entity> entity);
    void entityRemovedExtra(boost::shared_ptr<Entity> entity);
    void closeLevelStorage();
    void setTimeAndAdjustBlockTicks(long long newTime);
    void sendParticles(boost::shared_ptr<ServerPlayer> player, bool longDistance,
                       boost::shared_ptr<ClientboundLevelParticlesPacket> packet);
    void sendParticles(const ParticleType* type, bool longDistance, double x, double y, double z, int count,
                       double xDist, double yDist, double zDist, double maxSpeed, arrayWithLength<int> params);
    void sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist,
                       double zDist, double maxSpeed, arrayWithLength<int> params);
    void sendParticles(const ParticleType* type, double x, double y, double z, int count, double xDist, double yDist,
                       double zDist, double maxSpeed, arrayWithLength<int> params, double xAux, double yAux,
                       double zAux);
    void sendParticles(boost::shared_ptr<ServerPlayer> player, const ParticleType* type, bool longDistance, double x,
                       double y, double z, int count, double xDist, double yDist, double zDist, double maxSpeed,
                       arrayWithLength<int> params);
    boost::shared_ptr<Entity> getEntityByUUID(const std::wstring& uuid);
    bool isEntityIdInUse(int id);
    boost::shared_ptr<Player> getPlayer(const PlayerUID& uid) const;
    void queueSendBlockUpdate(int x, int y, int z, const BlockState* oldState, const BlockState* newState);
    bool atBlockEntityLimit(Block* block);
    bool atEntityLimit(boost::shared_ptr<Entity> entity);
    void flagEntitiesToBeRemoved(unsigned int* flags, bool* removedFound);
    ServerScoreboard* GetServerScoreboard();
    void invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ);

    MinecraftServer* server;
    EntityTracker* tracker;
    PlayerChunkMap* chunkMap;

private:
    OSFastMutex tickNextTickMutex;
    std::set<TickNextTickData, TickNextTickDataKeyCompare> tickNextTickList;
    boost::unordered_set<TickNextTickData, TickNextTickDataKeyHash, TickNextTickDataKeyEq> tickNextTickSet;
    boost::unordered_map<std::wstring, boost::shared_ptr<Entity> > entitiesByUuid;
    std::vector<QueuedSendBlockData*> queuedSendBlockUpdates;
    OSFastMutex queuedSendBlockUpdateMutex;
    int saveInterval;
    bool unusedFlag;

public:
    bool noSave;

private:
    bool allPlayersSleeping;
    PortalForcer* portalForcer;
    NaturalMobSpawner* mobSpawner;
    VillageSiege* villageSiege;
    int emptyTime;
    bool somePlayerSleeping;

public:
    ServerScoreboard* serverScoreboard;

private:
    std::vector<BlockEventData> blockEvents[2];
    int activeBlockEventList;
    ServerLevelListener* serverLevelListener;
    std::vector<TickNextTickData> toBeTicked;
    unsigned int bannerCount;
    unsigned int primedTntCount;
    unsigned int fallingBlockCount;
    unsigned int enderCrystalExplosionCount;
    unsigned int fireworksRocketCount;
    unsigned int beaconCount;
    OSFastMutex entityLimitMutex;
    OSFastMutex blockEntityLimitMutex;
    std::list<boost::shared_ptr<Entity> > itemEntities;
    std::list<boost::shared_ptr<Entity> > hangingEntities;
    std::list<boost::shared_ptr<Entity> > arrowEntities;
    std::list<boost::shared_ptr<Entity> > experienceOrbs;
    std::list<boost::shared_ptr<Entity> > areaEffectClouds;
    int primedTntCheckDelay;
};

#endif
