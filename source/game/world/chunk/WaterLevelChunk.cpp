#include "game/world/chunk/WaterLevelChunk.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

WaterLevelChunk::WaterLevelChunk(Level* level, ChunkPrimer* primer, int x, int z) : LevelChunk(level, primer, x, z)
{
    mDontSave = true;
    mChunkFlags = TERRAIN_POPULATED_ALL;
}

bool WaterLevelChunk::isAt(int x, int z)
{
    return x == this->x && z == this->z;
}

void WaterLevelChunk::recalcBlockLights()
{
}

void WaterLevelChunk::recalcHeightmapOnly()
{
}

void WaterLevelChunk::recalcHeightmap()
{
}

void WaterLevelChunk::lightLava()
{
}

const BlockState* WaterLevelChunk::setBlock(const BlockPos&, const BlockState* state)
{
    return state;
}

bool WaterLevelChunk::setBlockAndData(int, int, int, int, int, bool, bool)
{
    return true;
}

void WaterLevelChunk::setExtraElement(int, int, int, unsigned short)
{
}

unsigned short WaterLevelChunk::getExtraElement(int, int, int)
{
    return 0;
}

bool WaterLevelChunk::setData(int, int, int, int, int, bool* changed)
{
    *changed = true;
    return true;
}

void WaterLevelChunk::setBrightness(LightLayer::variety, const BlockPos&, int)
{
}

void WaterLevelChunk::setLevelChunkBrightness(LightLayer::variety layer, int x, int y, int z, int brightness)
{
    LevelChunk::setBrightness(layer, BlockPos(x, y, z), brightness);
}

void WaterLevelChunk::addEntity(boost::shared_ptr<Entity>)
{
}

void WaterLevelChunk::removeEntity(boost::shared_ptr<Entity>)
{
}

void WaterLevelChunk::removeEntity(boost::shared_ptr<Entity>, int)
{
}

boost::shared_ptr<BlockEntity> WaterLevelChunk::getBlockEntity(const BlockPos&, EntityCreationType)
{
    return boost::shared_ptr<BlockEntity>();
}

void WaterLevelChunk::addBlockEntity(boost::shared_ptr<BlockEntity>)
{
}

void WaterLevelChunk::setBlockEntity(const BlockPos&, boost::shared_ptr<BlockEntity>)
{
}

void WaterLevelChunk::removeBlockEntity(const BlockPos&)
{
}

void WaterLevelChunk::load(bool)
{
}

void WaterLevelChunk::unload(bool, bool)
{
}

bool WaterLevelChunk::containsPlayer()
{
    return false;
}

void WaterLevelChunk::markUnsaved()
{
}

void WaterLevelChunk::getEntities(boost::shared_ptr<Entity>, const AABB*, std::vector<boost::shared_ptr<Entity> >&,
                                  const Predicate<boost::shared_ptr<Entity> >*)
{
}

void WaterLevelChunk::getEntitiesOfClass(const std::type_info&, const AABB*, std::vector<boost::shared_ptr<Entity> >&,
                                         const Predicate<boost::shared_ptr<Entity> >*, bool)
{
}

int WaterLevelChunk::countEntities()
{
    return 0;
}

bool WaterLevelChunk::shouldSave(bool)
{
    return false;
}

int WaterLevelChunk::setBlocksAndData(arrayWithLength<unsigned char>, int x, int y, int z, int width, int height,
                                      int depth, int, bool extra)
{
    int sizeX = width - x;
    int sizeY = height - y;
    int sizeZ = depth - z;
    int volume = sizeX * sizeY * sizeZ;
    if (extra)
        return volume + volume / 2 * 3;
    return volume + volume / 2;
}

Random* WaterLevelChunk::getRandom(long long seed)
{
    return new Random(level->getSeed() + (long long)(x * x * 0x4c1906) + (long long)(x * 0x5ac0db)
                          + (long long)(z * z * 0x4307a7) + (long long)(z * 0x5f24f)
                      ^ seed);
}

Biome* WaterLevelChunk::getBiome(const BlockPos&, BiomeSource*)
{
    return Biome::getBiome(0);
}

void WaterLevelChunk::setFromRawData(arrayWithLength<unsigned char>)
{
}

void WaterLevelChunk::reSyncLighting()
{
}

void WaterLevelChunk::dropLighting()
{
}
