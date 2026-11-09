#include "game/entity/mob/NaturalMobSpawner.hpp"

#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <boost/unordered_set.hpp>
#include <string>
#include <utility>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/MobSpawnPlacements.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/core/MemSect.hpp"

NaturalMobSpawner::NaturalMobSpawner(int chunkRadius, ServerLevel* level)
{
    mLastSpawnTime = 0;
    int radius = chunkRadius / 2;
    mLevel = level;
    mMinChunk = -radius;
    mMaxChunk = radius - 1;
}

void NaturalMobSpawner::addChunkPos(int chunkX, int chunkZ)
{
    if (chunkX < mMinChunk || chunkX > mMaxChunk || chunkZ < mMinChunk || chunkZ > mMaxChunk)
        return;
    if (!mLevel->hasChunk(chunkX, chunkZ, true))
        return;
    insert(std::make_pair(ChunkPos(chunkX, chunkZ), false));
}

void NaturalMobSpawner::addEdgeChunkPos(int chunkX, int chunkZ)
{
    if (chunkX < mMinChunk || chunkX > mMaxChunk || chunkZ < mMinChunk || chunkZ > mMaxChunk)
        return;
    if (!mLevel->hasChunk(chunkX, chunkZ, true))
        return;
    ChunkPos pos(chunkX, chunkZ);
    if (find(pos) == end())
        insert(std::make_pair(pos, true));
}

BlockPos NaturalMobSpawner::getRandomPosWithin(Level* level, int chunkX, int chunkZ)
{
    LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
    int x = (chunkX << 4) + level->mRandom->nextInt(16);
    int z = (chunkZ << 4) + level->mRandom->nextInt(16);
    int height = Mth::roundUp(chunk->getHeightmap(BlockPos(x, 0, z)) + 1, 16);
    int y = level->mRandom->nextInt(height > 0 ? height : 256);
    return BlockPos(x, y, z);
}

bool NaturalMobSpawner::isValidEmptyUnderWaterSpawnBlock(const BlockState* state)
{
    if (state->isSolidBlockingCube() || state->isSignalSource() || !state->getMaterial()->isLiquid()
        || BaseRailBlock::isRail(state))
        return false;
    return true;
}

bool NaturalMobSpawner::isValidEmptySpawnBlock(const BlockState* state)
{
    if (state->isSolidBlockingCube() || state->isSignalSource() || state->getMaterial()->isLiquid()
        || BaseRailBlock::isRail(state))
        return false;
    return true;
}

bool NaturalMobSpawner::isSpawnPositionOk(Mob::SpawnPlacementType placementType, Level* level, const BlockPos& pos,
                                          int xSize, int zSize)
{
    if (!level->hasChunkAt(pos) || !level->getWorldBorder()->isWithinBounds(pos))
        return false;

    const BlockState* state = level->getBlockState(pos);
    if (placementType == Mob::SPAWN_PLACEMENT_TYPE_IN_WATER)
    {
        return state->getMaterial() == Material::WATER
               && level->getBlockState(pos.below())->getMaterial() == Material::WATER
               && !level->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource();
    }
    else if (placementType == Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER)
    {
        int depth = 0;
        int blocksBelow = 0;
        MutableBlockPos column(0, 0, 0);
        while (pos.y - depth >= 0 && depth < 5)
        {
            column.set(pos.x, pos.y - depth, pos.z);
            if (level->getBlock(column)->mMaterial->isLiquid())
                ++blocksBelow;
            ++depth;
        }

        bool surrounded = false;
        if (blocksBelow == 5)
        {
            if (level->getBlockState(pos.offset(5, 0, 0))->getMaterial()->isLiquid()
                && level->getBlockState(pos.offset(-5, 0, 0))->getMaterial()->isLiquid()
                && level->getBlockState(pos.offset(0, 0, 5))->getMaterial()->isLiquid()
                && level->getBlockState(pos.offset(0, 0, -5))->getMaterial()->isLiquid())
                surrounded = true;
        }

        return state->getMaterial()->isLiquid() && surrounded
               && !level->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource();
    }
    else if (placementType == Mob::SPAWN_PLACEMENT_TYPE_ON_WATER_FLOOR)
    {
        BlockPos below = pos.below();
        if (!level->getBlockState(below)->isTopSolidBlocking() && level->getBlock(pos)->mMaterial->isLiquid())
            return false;
        Block* floor = level->getBlock(below);
        bool validFloor = floor != Blocks::bedrock && floor != Blocks::barrier;
        return validFloor && isValidEmptyUnderWaterSpawnBlock(state)
               && isValidEmptyUnderWaterSpawnBlock(level->getBlockState(pos.above()));
    }
    else
    {
        BlockPos below = pos.below();
        if (level->getBlockState(below)->isTopSolidBlocking())
        {
            Block* floor = level->getBlock(below);
            bool validFloor = floor != Blocks::bedrock && floor != Blocks::barrier;
            return validFloor && isValidEmptySpawnBlock(state)
                   && isValidEmptySpawnBlock(level->getBlockState(pos.above()));
        }
    }
    return false;
}

unsigned int NaturalMobSpawner::tick(ServerLevel* level, bool spawnEnemies, bool spawnFriendlies, bool spawnPersistent)
{
    if (!spawnEnemies && !spawnFriendlies && !spawnPersistent)
        return 0;

    MemSect(20);
    long long now = System::processTimeInMilliSecs();
    if (now - mLastSpawnTime > CHUNK_POLL_REFRESH_MS)
    {
        mLastSpawnTime = now;
        clear();
        PIXBeginNamedEvent(0.0f, "Setup chunks to poll");

        int playerCount = (int)level->mPlayers.size();
        int* playerChunkX = new int[playerCount];
        int* playerChunkZ = new int[playerCount];
        for (int i = 0; i < playerCount; ++i)
        {
            boost::shared_ptr<Player> player = level->mPlayers[i];
            if (!player->isSpectator())
            {
                playerChunkX[i] = Mth::floor(player->mX / 16.0);
                playerChunkZ[i] = Mth::floor(player->mZ / 16.0);
                addChunkPos(playerChunkX[i], playerChunkZ[i]);
            }
        }

        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Setup edges");

        for (int distance = 1; distance <= CHUNK_POLL_RADIUS; ++distance)
        {
            for (int i = 0; i < distance * 2; ++i)
            {
                for (int p = 0; p < playerCount; ++p)
                {
                    bool edge = distance == CHUNK_POLL_RADIUS;
                    if (edge)
                    {
                        addEdgeChunkPos(playerChunkX[p] - distance + i, playerChunkZ[p] - distance);
                        addEdgeChunkPos(playerChunkX[p] + distance, playerChunkZ[p] - distance + i);
                        addEdgeChunkPos(playerChunkX[p] + distance - i, playerChunkZ[p] + distance);
                        addEdgeChunkPos(playerChunkX[p] - distance, playerChunkZ[p] + distance - i);
                    }
                    else
                    {
                        addChunkPos(playerChunkX[p] - distance + i, playerChunkZ[p] - distance);
                        addChunkPos(playerChunkX[p] + distance, playerChunkZ[p] - distance + i);
                        addChunkPos(playerChunkX[p] + distance - i, playerChunkZ[p] + distance);
                        addChunkPos(playerChunkX[p] - distance, playerChunkZ[p] + distance - i);
                    }
                }
            }
        }

        delete[] playerChunkX;
        delete[] playerChunkZ;
        PIXEndNamedEvent();
    }
    MemSect(0);

    unsigned int spawned = 0;
    MemSect(31);
    BlockPos sharedSpawn = level->getSharedSpawnPos();
    MemSect(0);
    boost::unordered_set<const MobGroupData*> groupDatas(11);
    PIXBeginNamedEvent(0.0f, "Attempt category spawn");
    MutableBlockPos pos(0, 0, 0);

    for (unsigned int categoryIndex = 0; categoryIndex < MobCategory::CATEGORIES.length; ++categoryIndex)
    {
        MobCategory* category = MobCategory::CATEGORIES.data[categoryIndex];
        bool friendly = category->isFriendly();
        if (friendly ? !spawnFriendlies : !spawnEnemies)
            continue;
        if (category->isPersistent() && !spawnPersistent)
            continue;
        if (friendly && level->mDimension->getType() != DimensionType::OVERWORLD)
            continue;

        PIXBeginNamedEvent(0.0f, "Count instance of");
        unsigned int instanceCount = level->countInstanceOf(category->getEnumBaseClass(), category->isSingleType(),
                                                            NULL, NULL);
        PIXEndNamedEvent();
        if (instanceCount >= category->getMaxInstancesPerLevel())
            continue;

        int chunkCount = (int)size();
        int pollsPerChunk = chunkCount != 0 ? SPAWN_POLL_BUDGET / chunkCount : SPAWN_GROUPS * SPAWN_PACK_SIZE;
        int packSize = std::max(std::min<int>(SPAWN_PACK_SIZE, pollsPerChunk / SPAWN_GROUPS), 1);
        int groupCount = std::max(std::min<int>(SPAWN_GROUPS, pollsPerChunk / packSize), 1);
        if (category == MobCategory::GUARDIAN || category == MobCategory::SLIME)
        {
            packSize = 1;
            groupCount = 1;
        }
        PIXBeginNamedEvent(0.0f, "Poll %d ch, dd %d ll %d, cat %d", chunkCount, groupCount, packSize, categoryIndex);

        bool categoryFull = false;
        for (iterator chunkEnd = end(), chunk = begin(); chunk != chunkEnd; ++chunk)
        {
            if (chunk->second)
                continue;

            LevelChunk* levelChunk = level->getCacheChunkDirect(chunk->first.x, chunk->first.z);
            if (levelChunk != NULL)
            {
                if (category == MobCategory::SLIME && level->mDimension->getType() != DimensionType::NETHER)
                {
                    if (level->mDimension->getType() == DimensionType::THE_END)
                        continue;
                    if ((levelChunk->getBiomePresentFlag()
                         & (LevelChunk::BIOME_PRESENT_SLIME_CHUNK | LevelChunk::BIOME_PRESENT_SWAMP))
                        == 0)
                        continue;
                }
                if (category == MobCategory::GUARDIAN
                    && (levelChunk->getBiomePresentFlag()
                        & (LevelChunk::BIOME_PRESENT_OCEAN | LevelChunk::BIOME_PRESENT_DEEP_OCEAN))
                           == 0)
                    continue;
            }

            BlockPos start = getRandomPosWithin(level, chunk->first.x, chunk->first.z);
            if (level->getBlockState(start)->isSolidBlockingCubeAndNotSignalSource())
                continue;

            int clusterSize = 0;
            bool chunkDone = false;
            for (int group = 0; group < groupCount; ++group)
            {
                int x = start.x;
                int y = start.y;
                int z = start.z;
                Biome::MobSpawnerData* spawnerData = NULL;
                MobGroupData* groupData = NULL;
                for (int attempt = 0; attempt < packSize; ++attempt)
                {
                    x += level->mRandom->nextInt(6) - level->mRandom->nextInt(6);
                    y += level->mRandom->nextInt(1) - level->mRandom->nextInt(1);
                    z += level->mRandom->nextInt(6) - level->mRandom->nextInt(6);
                    pos.set(x, y, z);
                    int chunkX = x >> 4;
                    int chunkZ = z >> 4;
                    if ((chunkX != chunk->first.x || chunkZ != chunk->first.z)
                        && !level->hasChunk(chunkX, chunkZ, true))
                        continue;

                    if (spawnerData == NULL)
                    {
                        spawnerData = level->getRandomMobSpawnAt(category, pos);
                        if (spawnerData == NULL)
                            break;
                    }
                    if (!level->canMobSpawnAt(category, spawnerData, pos))
                        continue;

                    float spawnX = x + 0.5f;
                    float spawnZ = z + 0.5f;
                    if (level->hasNearPlayer(spawnX, y, spawnZ, 24.0))
                        continue;
                    if (sharedSpawn.distSqr(spawnX, y, spawnZ) < 24.0 * 24.0)
                        continue;
                    if (!isSpawnPositionOk(MobSpawnPlacements::getPlacementType(spawnerData->mEntityBaseClass), level,
                                           pos, group, attempt))
                        continue;

                    boost::shared_ptr<Mob> mob;
                    MemSect(29);
                    mob = mMobCache.getTestMob(level, spawnerData->mEntityBaseClass);
                    MemSect(0);

                    unsigned long long mobType = mob->GetType();
                    if (!category->isSingleType()
                        && (mobType & (EntityType::MONSTER | EntityType::CREATURE_CATEGORY)) != 0)
                    {
                        if (mobType == EntityType::GHAST)
                        {
                            if (level->countInstanceOf(EntityType::GHAST, true, NULL, NULL) >= MAX_GHASTS)
                                continue;
                        }
                        else if (mobType == EntityType::PIG_ZOMBIE)
                        {
                            if (level->countInstanceOf(EntityType::PIG_ZOMBIE, true, NULL, NULL)
                                >= category->getMaxInstancesPerLevel() - PIG_ZOMBIE_CAP_MARGIN)
                                continue;
                        }
                        else if (mobType == EntityType::ENDER_MAN
                                 && level->mDimension->getType() == DimensionType::THE_END)
                        {
                            unsigned int maxEndermen = Mth::floor(category->getMaxInstancesPerLevel() * 0.5f);
                            if (level->countInstanceOf(EntityType::ENDER_MAN, false, NULL, NULL) >= maxEndermen)
                                continue;
                        }
                        else if (mobType == EntityType::ENDER_MAN || (mobType & EntityType::MONSTER) != 0)
                        {
                            MobCategory* mobCategory = MobCategory::getCategory(mobType);
                            if (mobCategory != category
                                && level->countInstanceOf(mobType, true, NULL, NULL)
                                       >= mobCategory->getMaxInstancesPerLevel())
                                continue;
                        }
                        else if (level->countInstanceOf(mobType, true, NULL, NULL)
                                 >= category->getMaxInstancesPerLevel() / 2)
                        {
                            continue;
                        }
                    }

                    float yRot = level->mRandom->nextFloat() * 360.0f;
                    mob->moveTo(spawnX, y, spawnZ, yRot, 0.0f);
                    if (mob->checkSpawnRules() && mob->checkSpawnObstruction())
                    {
                        mob->checkDespawn();
                        if (!mob->mRemoved)
                        {
                            mMobCache.removeFromCache(mob);
                            mob->setDespawnProtected();
                            groupData = mob->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(mob)), groupData,
                                                              L"");
                            groupDatas.insert(groupData);
                            if (mob->checkSpawnObstruction())
                            {
                                level->addEntity(mob);
                                ++instanceCount;
                                ++clusterSize;
                            }
                            else
                            {
                                mob->remove();
                            }
                            if (instanceCount >= category->getMaxInstancesPerLevel())
                            {
                                categoryFull = true;
                                break;
                            }
                            if (clusterSize >= mob->getMaxSpawnClusterSize())
                            {
                                chunkDone = true;
                                break;
                            }
                        }
                        else
                        {
                            mob->mRemoved = false;
                        }
                    }
                    spawned += clusterSize;
                }
                if (categoryFull || chunkDone)
                    break;
            }
            if (categoryFull)
                break;
        }

        if (categoryFull)
            continue;
        PIXEndNamedEvent();
    }

    PIXEndNamedEvent();
    long long spawnEnd = System::processTimeInMilliSecs();
    if (level->mDimension->getType() == DimensionType::OVERWORLD)
        PIXAddNamedCounter((float)(spawnEnd - now), "Mob spawning");

    for (boost::unordered_set<const MobGroupData*>::iterator it = groupDatas.begin(); it != groupDatas.end(); ++it)
        delete *it;

    return spawned;
}

void NaturalMobSpawner::spawnMobsForChunkGeneration(Level* level, Biome* biome, int x, int y, int z, int count,
                                                    Random* random)
{
    return;
}
