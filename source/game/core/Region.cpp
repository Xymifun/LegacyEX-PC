#include "game/core/Region.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/PIXNamedEvents.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/entity/mob/EntityTickCache.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/Direction.hpp"

Region::Region(Level* level, const BlockPos& from, const BlockPos& to, int expansion)
{
    mLevel = level;
    mMinChunkX = (from.x - expansion) >> 4;
    mMinChunkZ = (from.z - expansion) >> 4;
    int maxX = (to.x + expansion) >> 4;
    int maxZ = (to.z + expansion) >> 4;
    mChunks = new array2DWithLength<LevelChunk*>(maxX - mMinChunkX + 1, maxZ - mMinChunkZ + 1);
    mAllEmpty = true;
    int x;
    for (x = mMinChunkX; x <= maxX; ++x)
    {
        int z;
        for (z = mMinChunkZ; z <= maxZ; ++z)
            (*mChunks)[x - mMinChunkX]->data[z - mMinChunkZ] = level->getChunk(x, z);
    }
    for (x = from.x >> 4; x <= to.x >> 4; ++x)
    {
        int z;
        for (z = from.z >> 4; z <= to.z >> 4; ++z)
        {
            LevelChunk* chunk = mChunks->data[x - mMinChunkX]->data[z - mMinChunkZ];
            if (chunk != NULL && !chunk->isYSpaceEmpty(from.y, to.y))
                mAllEmpty = false;
        }
    }
    mCachedX = -1;
    mCachedStates = NULL;
    mCachedZ = -1;
}

Region::~Region()
{
    for (unsigned int i = 0; i < mChunks->length; i++)
    {
        arrayWithLength<LevelChunk*>* row = (*mChunks)[i];
        delete[] row->data;
        delete row;
    }
    delete[] mChunks->data;
    delete mChunks;
}

bool Region::isAllEmpty()
{
    return mAllEmpty;
}

boost::shared_ptr<BlockEntity> Region::getBlockEntity(const BlockPos& pos, LevelChunk::EntityCreationType type)
{
    return getLevelChunk(pos)->getBlockEntity(pos, type);
}

boost::shared_ptr<BlockEntity> Region::getBlockEntity(const BlockPos& pos)
{
    return getBlockEntity(pos, LevelChunk::IMMEDIATE);
}

int Region::getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness)
{
    if (!mLevel->isWithinLevelBounds(pos.x, pos.z))
        return 15;
    if (useNeighbourBrightness && getBlockState(pos)->doesPropagate())
    {
        int brightest = 0;
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
             it != Direction::VALUES.end(); it++)
        {
            int value = getRawBrightness(pos.relative(*it), false);
            if (value > brightest)
                brightest = value;
            if (brightest >= 15)
                break;
        }
        return brightest;
    }
    if (pos.y < 0)
        return 0;
    if (pos.y >= 256)
    {
        int value = 15 - mLevel->getSkyDarken();
        if (value < 0)
            value = 0;
        return value;
    }
    return getLevelChunk(pos)->getRawBrightness(pos, mLevel->getSkyDarken());
}

int Region::getRawBrightness(const BlockPos& pos)
{
    return getRawBrightness(pos, true);
}

float Region::getBrightness(const BlockPos& pos, int lightValue)
{
    int value = getRawBrightness(pos);
    if (value < lightValue)
        value = lightValue;
    return mLevel->mDimension->getBrightnessRamp()[value];
}

int Region::getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos, int lightValue)
{
    if (layer == LightLayer::SKY && !mLevel->mDimension->isHasSkyLight())
        return 0;
    BlockPos target = pos;
    if (target.y < 0)
        target = BlockPos(target.x, 0, target.z);
    if (!mLevel->reallyHasChunkAt(target))
        return layer;
    if (getBlockState(target)->doesPropagate())
    {
        int brightest = 0;
        for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            int value = getBrightness(layer, target.relative(*it));
            if (value > brightest)
                brightest = value;
            if (brightest >= 15)
                break;
        }
        return brightest;
    }
    return getLevelChunk(target)->getBrightness(layer, target);
}

int Region::getLightColor(const BlockPos& pos, int lightValue, int cacheIndex)
{
    int sky = getBrightnessPropagate(LightLayer::SKY, pos, cacheIndex);
    int block = getBrightnessPropagate(LightLayer::BLOCK, pos, cacheIndex);
    if (block < lightValue)
        block = lightValue;
    return (sky << 20) | (block << 4);
}

float Region::getBrightness(const BlockPos& pos)
{
    return mLevel->mDimension->getBrightnessRamp()[getRawBrightness(pos)];
}

const BlockState* Region::getBlockState(const BlockPos& pos)
{
    if ((unsigned int)pos.y < 256)
    {
        int chunkX = pos.x >> 4;
        int chunkZ = pos.z >> 4;
        if (mCachedStates != NULL && chunkX == mCachedX && chunkZ == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            int yEnd = mCachedY * 16 + 17;
            if (pos.y >= yBase && pos.y < yEnd)
            {
                int index = (pos.x & 15) | ((pos.z & 15) << 4) | ((pos.y - yBase) << 8);
                return mCachedStates[index];
            }
        }
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getBlockState(pos);
        }
    }
    return Blocks::air->defaultBlockState();
}

const BlockState* Region::getBlockState(EntityTickCache& tickCache, const BlockPos& pos)
{
    const BlockState* state;
    if ((unsigned int)pos.y < 256)
    {
        state = tickCache.getBlockState(pos.x, pos.y, pos.z);
        if (state == NULL)
            return getBlockState(pos);
    }
    else
    {
        state = Blocks::air->defaultBlockState();
    }
    return state;
}

const BlockState* Region::getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos)
{
    return getBlockState(tickCache, pos);
}

int Region::getBlockId(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        int chunkX = x >> 4;
        int chunkZ = z >> 4;
        if (mCachedStates != NULL && chunkX == mCachedX && chunkZ == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            int yEnd = mCachedY * 16 + 17;
            if (y >= yBase && y < yEnd)
                return Block::getId(mCachedStates[(x & 15) | ((z & 15) << 4) | ((y - yBase) << 8)]->getBlock());
        }
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getBlockId(x, y, z);
        }
    }
    return 0;
}

int Region::getBlockId(EntityTickCache& tickCache, int x, int y, int z)
{
    unsigned short element = tickCache.getElement(x, y, z);
    if (element == 0xffff)
        element = getElement(x, y, z);
    return element >> 4;
}

unsigned short Region::getElement(EntityTickCache& tickCache, int x, int y, int z)
{
    unsigned short element = tickCache.getElement(x, y, z);
    if (element == 0xffff)
        element = getElement(x, y, z);
    return element;
}

int Region::getBlockData(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        int chunkX = x >> 4;
        int chunkZ = z >> 4;
        if (mCachedStates != NULL && chunkX == mCachedX && chunkZ == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            int yEnd = mCachedY * 16 + 17;
            if (y >= yBase && y < yEnd)
            {
                int index = (x & 15) | ((z & 15) << 4) | ((y - yBase) << 8);
                return mCachedStates[index]->getBlock()->convertBlockStateToLegacyData(mCachedStates[index]);
            }
        }
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getData(x, y, z);
        }
    }
    return 0;
}

unsigned short Region::getElement(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        int chunkX = x >> 4;
        int chunkZ = z >> 4;
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getElement(x, y, z);
        }
    }
    return 0;
}

unsigned short Region::getExtraElement(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        int chunkX = x >> 4;
        int chunkZ = z >> 4;
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getExtraElement(x, y, z);
        }
    }
    return 0;
}

void Region::getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement)
{
    if ((unsigned int)y < 256)
    {
        int chunkX = x >> 4;
        int chunkZ = z >> 4;
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                chunk->getElementAndExtraElement(x, y, z, element, extraElement);
        }
    }
    else
    {
        *element = 0;
        *extraElement = 0;
    }
}

Block* Region::getBlock(const BlockPos& pos)
{
    if ((unsigned int)pos.y < 256)
    {
        int chunkX = pos.x >> 4;
        int chunkZ = pos.z >> 4;
        if (mCachedStates != NULL && chunkX == mCachedX && chunkZ == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            int yEnd = mCachedY * 16 + 17;
            if (pos.y >= yBase && pos.y < yEnd)
                return mCachedStates[(pos.x & 15) | ((pos.z & 15) << 4) | ((pos.y - yBase) << 8)]->getBlock();
        }
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getBlock(pos);
        }
    }
    return Blocks::air;
}

bool Region::isSolidRender(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);
    if (state == NULL)
        return false;
    if (LeafBlock::isLeaves(state->getBlock()))
    {
        int xOffsets[6] = {1, -1, 0, 0, 0, 0};
        int yOffsets[6] = {0, 0, 1, -1, 0, 0};
        int zOffsets[6] = {0, 0, 0, 0, 1, -1};
        for (int i = 0; i < 6; ++i)
        {
            const BlockState* neighbour = mLevel->getBlockState(pos.offset(xOffsets[i], yOffsets[i], zOffsets[i]));
            if (!LeafBlock::isLeaves(neighbour->getBlock()) && !neighbour->isSolidRender())
                return false;
        }
        return true;
    }
    return state->getBlock()->isSolidRender(state);
}

BiomeSource* Region::getBiomeSource()
{
    return mLevel->getBiomeSource();
}

Biome* Region::getBiome(const BlockPos& pos)
{
    return getLevelChunk(pos)->getBiome(pos, mLevel->getBiomeSource());
}

int Region::getExtraBlockId(int x, int y, int z)
{
    return getExtraElement(x, y, z) >> 4;
}

void Region::getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId)
{
    *blockId = getBlockId(x, y, z);
    *extraBlockId = getExtraBlockId(x, y, z);
}

int Region::getExtraBlockData(int x, int y, int z)
{
    return getExtraElement(x, y, z) & 15;
}

void Region::getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock)
{
    *block = getBlock(pos);
    *extraBlock = getExtraBlock(pos);
}

Block* Region::getExtraBlock(const BlockPos& pos)
{
    if ((unsigned int)pos.y < 256)
    {
        int chunkX = pos.x >> 4;
        int chunkZ = pos.z >> 4;
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getExtraBlock(pos);
        }
    }
    return Blocks::air;
}

Block* Region::getLiquidBlock(const BlockPos& pos)
{
    Block* extra = getExtraBlock(pos);
    if (extra != NULL && extra != Blocks::air)
        return extra;
    return getBlock(pos);
}

const BlockState* Region::getLiquidBlockState(const BlockPos& pos)
{
    const BlockState* extra = getExtraBlockState(pos);
    if (extra != NULL && extra != Blocks::air->defaultBlockState())
        return extra;
    return getBlockState(pos);
}

const BlockState* Region::getExtraBlockState(const BlockPos& pos)
{
    if ((unsigned int)pos.y < 256)
    {
        int chunkX = pos.x >> 4;
        int chunkZ = pos.z >> 4;
        chunkX -= mMinChunkX;
        chunkZ -= mMinChunkZ;
        if (chunkX >= 0 && chunkX < (int)mChunks->length && chunkZ >= 0 && chunkZ < (int)(*mChunks)[chunkX]->length)
        {
            LevelChunk* chunk = (*mChunks)[chunkX]->data[chunkZ];
            if (chunk != NULL)
                return chunk->getExtraBlockState(pos);
        }
    }
    return Blocks::air->defaultBlockState();
}

bool Region::isEmptyBlock(const BlockPos& pos)
{
    return getBlockId(pos.x, pos.y, pos.z) == BlockId::AIR;
}

bool Region::isEmptyWaterBlock(const BlockPos& pos)
{
    return getBlockId(pos.x, pos.y, pos.z) == BlockId::WATER;
}

int Region::getBrightness(LightLayer::variety layer, const BlockPos& pos)
{
    if ((unsigned int)pos.y >= 256)
        return layer;
    return getLevelChunk(pos)->getBrightness(layer, pos);
}

int Region::getMaxBuildHeight()
{
    return 256;
}

int Region::getDirectSignal(const BlockPos& pos, const Direction* direction)
{
    return getBlockState(pos)->getDirectSignal(this, pos, direction);
}

LevelType* Region::getGeneratorType()
{
    return mLevel->getGeneratorType();
}

unsigned int Region::elementToBlockId(unsigned short element)
{
    return (element >> 4) & 0xffff;
}

unsigned int Region::elementToData(unsigned short element)
{
    return element & 15;
}

void Region::setCachedBlocksAndData(unsigned short* elements, int x, int y, int z, const BlockState** states)
{
    mCachedX = x;
    mCachedY = y;
    mCachedZ = z;
    mCachedStates = states;

    PIXBeginNamedEvent(0.0f, "Region::setCachedBlocksAndData");

    unsigned int lastBlockId = 0xffff;
    unsigned char lastData = -1;
    const BlockState* lastState = NULL;
    const BlockState* airState = Blocks::air->defaultBlockState();
    int yBase = y * 16 - 1;

    for (int yi = 0; yi < 18; ++yi)
    {
        for (int xi = 0; xi < 16; ++xi)
        {
            for (int zi = 0; zi < 16; ++zi)
            {
                int worldY = yi + yBase;
                unsigned int blockId = BlockId::AIR;
                unsigned char data = 0;
                if ((unsigned int)worldY < 256)
                {
                    unsigned short element =
                        elements[((worldY << 8) & 0x8000) | (xi << 11) | (zi << 7) | (worldY & 0x7f)];
                    blockId = elementToBlockId(element);
                    data = elementToData(element);
                }
                if (blockId == BlockId::AIR)
                {
                    states[yi * 256 + zi * 16 + xi] = airState;
                }
                else
                {
                    if (blockId != lastBlockId || data != lastData)
                    {
                        lastState = Block::byId(blockId)->getBlockState(data);
                        lastBlockId = blockId;
                        lastData = data;
                    }
                    states[yi * 256 + zi * 16 + xi] = lastState;
                }
            }
        }
    }

    PIXEndNamedEvent();
}
