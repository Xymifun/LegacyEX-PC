#include "game/world/chunk/ChunkPrimer.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IdMapper.hpp"
#include "game/world/block/Material.hpp"

ChunkPrimer::ChunkPrimer(bool ownsElementIds, arrayWithLength<unsigned short> elementIds) : mElementIds(elementIds)
{
    mDefaultState = Blocks::air->defaultBlockState();
    mOwnsElementIds = ownsElementIds;
    mEmissiveMaxHeight = 0;
}

ChunkPrimer::~ChunkPrimer()
{
    if (mOwnsElementIds)
    {
        delete[] mElementIds.data;
    }
}

const BlockState* ChunkPrimer::getState(int index)
{
    return Block::getStateByElementId(mElementIds.data[index]);
}

const BlockState* ChunkPrimer::getState(int x, int y, int z)
{
    int half = 0;

    if (mElementIds.length > 0x8000 && y >= 128)
    {
        y -= 128;
        half = 0x8000;
    }

    return getState(half + ((x << 11) | (z << 7) | y));
}

void ChunkPrimer::setState(int index, const BlockState* state)
{
    unsigned short elementId = 0;

    if (state != NULL)
    {
        elementId = (unsigned short)Block::blockStateRegistry->getId(state);
    }

    mElementIds.data[index] = elementId;

    if (state != NULL && state->getLightEmission() > 0)
    {
        int y = index & 0x7F;

        if (index > 0x8000)
        {
            y += 128;
        }

        if (y + 1 > mEmissiveMaxHeight)
        {
            mEmissiveMaxHeight = (unsigned short)(y + 1);
        }
    }
}

void ChunkPrimer::setState(int x, int y, int z, const BlockState* state)
{
    int half = 0;

    if (mElementIds.length > 0x8000 && y >= 128)
    {
        y -= 128;
        half = 0x8000;
    }

    setState(half + ((x << 11) | (z << 7) | y), state);
}

void ChunkPrimer::setBlockAndData(int index, int blockId, int blockData)
{
    unsigned short elementId = (unsigned short)((blockId << 4) | blockData);

    mElementIds.data[index] = elementId;

    const BlockState* state = Block::getStateByElementId(elementId);

    if (state != NULL && state->getLightEmission() > 0)
    {
        int y = index & 0x7F;

        if (index > 0x8000)
        {
            y += 128;
        }

        if (y + 1 > mEmissiveMaxHeight)
        {
            mEmissiveMaxHeight = (unsigned short)(y + 1);
        }
    }
}

int ChunkPrimer::getBlockId(int index)
{
    return mElementIds.data[index] >> 4;
}

arrayWithLength<unsigned short> ChunkPrimer::getElementIds()
{
    return mElementIds;
}

unsigned short ChunkPrimer::getEmissiveMaxHeight()
{
    return mEmissiveMaxHeight;
}

int ChunkPrimer::getHighestNonAirPos(int x, int z)
{
    int columnTop = ((x << 11) | (z << 7)) + 127;

    for (int y = 127; y >= 0; y--)
    {
        if (Block::byId(mElementIds[columnTop + y] >> 4) != Blocks::air)
        {
            return y;
        }
    }

    return 0;
}

int ChunkPrimer::getHighestSolidPos(int x, int z)
{
    int columnTop = ((x << 11) | (z << 7)) + 127;

    for (int y = 127; y >= 0; y--)
    {
        Block* block = Block::byId(mElementIds[columnTop + y] >> 4);

        if (block->mMaterial->blocksMotion() && block->mMaterial != Material::LEAVES)
        {
            return y;
        }
    }

    return 0;
}
