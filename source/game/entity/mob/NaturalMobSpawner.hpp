#ifndef GAME_ENTITY_MOB_NATURALMOBSPAWNER_HPP
#define GAME_ENTITY_MOB_NATURALMOBSPAWNER_HPP

#include <boost/unordered_map.hpp>

#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCache.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class Biome;
class BlockPos;
class BlockState;
class Level;
class Random;
class ServerLevel;

class NaturalMobSpawner : public boost::unordered_map<ChunkPos, bool, ChunkPosKeyHash, ChunkPosKeyEq>
{
public:
    NaturalMobSpawner(int chunkRadius, ServerLevel* level);

    void addChunkPos(int chunkX, int chunkZ);
    void addEdgeChunkPos(int chunkX, int chunkZ);

    static BlockPos getRandomPosWithin(Level* level, int chunkX, int chunkZ);
    static bool isValidEmptyUnderWaterSpawnBlock(const BlockState* state);
    static bool isValidEmptySpawnBlock(const BlockState* state);
    static bool isSpawnPositionOk(Mob::SpawnPlacementType placementType, Level* level, const BlockPos& pos, int xSize,
                                  int zSize);

    unsigned int tick(ServerLevel* level, bool spawnEnemies, bool spawnFriendlies, bool spawnPersistent);

    static void spawnMobsForChunkGeneration(Level* level, Biome* biome, int x, int y, int z, int count, Random* random);

private:
    static const long long CHUNK_POLL_REFRESH_MS = 1000;
    enum
    {
        CHUNK_POLL_RADIUS = 8,
        SPAWN_GROUPS = 3,
        SPAWN_PACK_SIZE = 4,
        SPAWN_POLL_BUDGET = (2 * CHUNK_POLL_RADIUS + 1) * (2 * CHUNK_POLL_RADIUS + 1) * SPAWN_GROUPS * SPAWN_PACK_SIZE
    };
    static const unsigned int MAX_GHASTS = 4;
    static const unsigned int PIG_ZOMBIE_CAP_MARGIN = 10;

    MobCache mMobCache;
    long long mLastSpawnTime;
    int mMinChunk;
    int mMaxChunk;
    ServerLevel* mLevel;
};

#endif
