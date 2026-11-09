#include "game/world/block/BlockTransaction.hpp"

#include <algorithm>

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPos.hpp"

BlockTransaction::Element g_emptyBlockElement;

BlockTransaction::BlockTransaction(const BlockPos& min, const BlockPos& max, const Vec3i& size, Element* elements,
                                   unsigned int elementCount)
    : mSize(size),
      mElementCount(elementCount),
      mMin(size),
      mMax(0, 0, 0),
      mElements(elements)
{
}

BlockTransaction::~BlockTransaction()
{
    delete[] mElements;
}

BlockTransaction* BlockTransaction::CreateTransaction(const BlockPos& min, const BlockPos& max)
{
    Vec3i size(max.x - min.x, max.y - min.y, max.z - min.z);
    unsigned int elementCount = size.x * size.y * size.z;
    Element* elements = new Element[elementCount];
    memset(elements, 0, elementCount * sizeof(Element));
    return new BlockTransaction(min, max, size, elements, elementCount);
}

void BlockTransaction::updateMinMax(const Vec3i& position)
{
    int x = position.x;
    mMin.x = std::min(mMin.x, x);
    int y = position.y;
    mMin.y = std::min(mMin.y, y);
    int z = position.z;
    mMin.z = std::min(mMin.z, z);
    mMax.x = std::max(mMax.x, x + 1);
    mMax.y = std::max(mMax.y, y + 1);
    mMax.z = std::max(mMax.z, z + 1);
}

BlockTransaction::Element* BlockTransaction::at(const Vec3i& position) const
{
    Element* element = &g_emptyBlockElement;
    unsigned int index = position.y + mSize.y * position.x + mSize.y * mSize.x * position.z;
    if (index < mElementCount)
        element = &mElements[index];
    return element;
}

bool BlockTransaction::inBounds(const Vec3i& position) const
{
    return position.x >= 0 && position.x < mSize.x && position.y >= 0 && position.y < mSize.y && position.z >= 0
           && position.z < mSize.z;
}

bool BlockTransaction::shouldWrite(const Vec3i& position)
{
    return at(position)->value != 0;
}

std::deque<ChunkPos>* BlockTransaction::getAffectedChunks(Level*, const Vec3i& position)
{
    Vec3i end = position.offset(mSize);
    int minChunkX = position.x >> 4;
    int minChunkZ = position.z >> 4;
    int maxChunkX = (end.x - 1) >> 4;
    int maxChunkZ = (end.z - 1) >> 4;
    std::deque<ChunkPos>* chunks = new std::deque<ChunkPos>();
    for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
    {
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
        {
            chunks->push_back(ChunkPos(chunkX, chunkZ));
        }
    }
    return chunks;
}

void BlockTransaction::setBlock(const Vec3i& position, int block, int data)
{
    Element* target = at(position);
    target->value = (unsigned short)((block << 4) | data);
    if (block != 0)
        updateMinMax(position);
}

void BlockTransaction::setElement(const Vec3i& position, unsigned short element)
{
    Element* target = at(position);
    target->value = element;
    if (element != 0)
        updateMinMax(position);
}

void BlockTransaction::setBlock(const Vec3i& position, const BlockState* state)
{
    Block* block = state->getBlock();
    int id = Block::getId(block);
    int data = block->convertBlockStateToLegacyData(state);
    setBlock(position, id, data & 0xf);
}

int BlockTransaction::getBlock(const Vec3i& position) const
{
    return (int)(at(position)->value >> 4);
}

unsigned short BlockTransaction::getData(const Vec3i& position) const
{
    return at(position)->value & 0xf;
}

bool BlockTransaction::isEmpty(const Vec3i& position) const
{
    return getBlock(position) == 0;
}

void BlockTransaction::reduceMinMax()
{
    int minX = mMin.x;
    int maxX = mMax.x;
    int minY = mMin.y;
    int maxY = mMax.y;
    int minZ = mMin.z;
    int maxZ = mMax.z;
    mMin = mMax;
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                Vec3i position(x, y, z);
                if (!isEmpty(position))
                    updateMinMax(position);
            }
        }
    }
}

Vec3i BlockTransaction::getMin() const
{
    return Vec3i(mMin.x, mMin.y, mMin.z);
}

Vec3i BlockTransaction::getMax() const
{
    return Vec3i(mMax.x, mMax.y, mMax.z);
}
