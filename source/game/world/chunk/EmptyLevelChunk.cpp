#include "game/world/chunk/EmptyLevelChunk.hpp"

#include <cstring>

#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

EmptyLevelChunk::EmptyLevelChunk(Level* level, int x, int z) : LevelChunk(level, x, z)
{
    mDontSave = true;
    mChunkFlags = TERRAIN_POPULATED_ALL;
}

bool EmptyLevelChunk::isAt(int x, int z)
{
    return x == this->x && z == this->z;
}

unsigned char EmptyLevelChunk::getHeightmap(int, int)
{
    return 0;
}

void EmptyLevelChunk::recalcHeightmapOnly()
{
}

void EmptyLevelChunk::recalcHeightmap()
{
}

int EmptyLevelChunk::getBlockLightBlock(const BlockPos&)
{
    return 255;
}

Block* EmptyLevelChunk::getBlock(const BlockPos&)
{
    return Blocks::air;
}

const BlockState* EmptyLevelChunk::getBlockState(const BlockPos&)
{
    return Blocks::air->defaultBlockState();
}

int EmptyLevelChunk::getData(const BlockPos&)
{
    return 0;
}

int EmptyLevelChunk::getBrightness(LightLayer::variety layer, const BlockPos&)
{
    return layer == LightLayer::SKY ? LightLayer::SKY : LightLayer::BLOCK;
}

void EmptyLevelChunk::getNeighbourBrightnesses(int* brightnesses, LightLayer::variety, int, int, int)
{
    brightnesses[0] = 0;
    brightnesses[1] = 0;
    brightnesses[2] = 0;
    brightnesses[3] = 0;
    brightnesses[4] = 0;
    brightnesses[5] = 0;
}

void EmptyLevelChunk::setBrightness(LightLayer::variety, const BlockPos&, int)
{
}

int EmptyLevelChunk::getRawBrightness(const BlockPos&, int)
{
    return 0;
}

void EmptyLevelChunk::addEntity(boost::shared_ptr<Entity>)
{
}

void EmptyLevelChunk::removeEntity(boost::shared_ptr<Entity>)
{
}

void EmptyLevelChunk::removeEntity(boost::shared_ptr<Entity>, int)
{
}

bool EmptyLevelChunk::isSkyLit(const BlockPos&)
{
    return false;
}

boost::shared_ptr<BlockEntity> EmptyLevelChunk::getBlockEntity(const BlockPos&, EntityCreationType)
{
    return boost::shared_ptr<BlockEntity>();
}

void EmptyLevelChunk::addBlockEntity(boost::shared_ptr<BlockEntity>)
{
}

void EmptyLevelChunk::setBlockEntity(const BlockPos&, boost::shared_ptr<BlockEntity>)
{
}

void EmptyLevelChunk::removeBlockEntity(const BlockPos&)
{
}

void EmptyLevelChunk::load(bool)
{
}

void EmptyLevelChunk::unload(bool, bool)
{
}

bool EmptyLevelChunk::containsPlayer()
{
    return false;
}

void EmptyLevelChunk::markUnsaved()
{
}

void EmptyLevelChunk::getEntities(boost::shared_ptr<Entity>, const AABB*, std::vector<boost::shared_ptr<Entity> >&,
                                  const Predicate<boost::shared_ptr<Entity> >*)
{
}

void EmptyLevelChunk::getEntitiesOfClass(const std::type_info&, const AABB*, std::vector<boost::shared_ptr<Entity> >&,
                                         const Predicate<boost::shared_ptr<Entity> >*, bool)
{
}

bool EmptyLevelChunk::shouldSave(bool)
{
    return false;
}

int EmptyLevelChunk::getBlocksAndData(arrayWithLength<unsigned char>* output, int x, int y, int z, int width,
                                      int height, int depth, int offset, bool includeLight)
{
    int volume = (width - x) * (height - y) * (depth - z);
    int length;
    if (includeLight)
        length = volume + (volume / 2) * 3;
    else
        length = volume + volume / 2;
    unsigned char* begin = &output->data[offset];
    unsigned char* end = &output->data[offset + length];
    while (begin != end)
        *begin++ = 0;
    return length;
}

int EmptyLevelChunk::setBlocksAndData(arrayWithLength<unsigned char>, int x, int y, int z, int width, int height,
                                      int depth, int, bool includeLight)
{
    int volume = (width - x) * (height - y) * (depth - z);
    int length;
    if (includeLight)
        length = volume + (volume / 2) * 3;
    else
        length = volume + volume / 2;
    return length;
}

Random* EmptyLevelChunk::getRandom(long long seed)
{
    return new Random(getLevel()->getSeed() + (long long)(x * x * 0x4c1906) + (long long)(x * 0x5ac0db)
                          + (long long)(z * z * 0x4307a7) + (long long)(z * 0x5f24f)
                      ^ seed);
}

bool EmptyLevelChunk::isEmpty()
{
    return true;
}

const BlockState* EmptyLevelChunk::setBlock(const BlockPos&, const BlockState*)
{
    return Blocks::air->defaultBlockState();
}

void EmptyLevelChunk::getElementRange(arrayWithLength<unsigned short> elements, int, int)
{
    std::memset(elements.data, 0, elements.length * sizeof(unsigned short));
}

void EmptyLevelChunk::compressLighting()
{
}

void EmptyLevelChunk::compressBlocks()
{
}

void EmptyLevelChunk::compressData()
{
}

void EmptyLevelChunk::setAllElements(arrayWithLength<unsigned char>, arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::getAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data)
{
    std::memset(elements.data, 0, elements.length);
    std::memset(data.data, 0, data.length);
}

void EmptyLevelChunk::getBlockData(arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::getDataData(arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::getSkyLightData(arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::getBlockLightData(arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::setSkyLightData(arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::setBlockLightData(arrayWithLength<unsigned char>)
{
}

void EmptyLevelChunk::dropLighting()
{
}
