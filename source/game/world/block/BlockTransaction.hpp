#ifndef GAME_WORLD_BLOCK_BLOCKTRANSACTION_HPP
#define GAME_WORLD_BLOCK_BLOCKTRANSACTION_HPP

#include <deque>

#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockPos.hpp"

class BlockState;
class ChunkPos;
class Level;

class BlockTransaction
{
public:
    struct Element
    {
        unsigned short value;
    };

    Vec3i mSize;
    unsigned int mElementCount;
    Vec3i mMin;
    Vec3i mMax;
    Element* mElements;

    BlockTransaction(const BlockPos& min, const BlockPos& max, const Vec3i& size, Element* elements,
                     unsigned int elementCount);
    ~BlockTransaction();

    static BlockTransaction* CreateTransaction(const BlockPos& min, const BlockPos& max);
    void readFrom(Level* level, const Vec3i& position);
    void updateMinMax(const Vec3i& position);
    Element* at(const Vec3i& position) const;
    bool inBounds(const Vec3i& position) const;
    bool shouldWrite(const Vec3i& position);
    std::deque<ChunkPos>* getAffectedChunks(Level* level, const Vec3i& position);
    void setBlock(const Vec3i& position, int block, int data);
    void setElement(const Vec3i& position, unsigned short element);
    void setBlock(const Vec3i& position, const BlockState* state);
    int getBlock(const Vec3i& position) const;
    unsigned short getData(const Vec3i& position) const;
    bool isEmpty(const Vec3i& position) const;
    void reduceMinMax();
    Vec3i getMin() const;
    Vec3i getMax() const;
    bool isEmpty() const;
};

#endif
