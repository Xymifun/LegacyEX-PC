#ifndef GAME_WORLD_BLOCK_BLOCKPOSITERATOR_HPP
#define GAME_WORLD_BLOCK_BLOCKPOSITERATOR_HPP

#include "game/world/block/Iterator.hpp"
#include "game/world/block/BlockPos.hpp"

class BlockPosIterator : public Iterator<BlockPos*>
{
public:
    BlockPosIterator(const BlockPos& from, const BlockPos& to);
    virtual ~BlockPosIterator();

    virtual bool hasNext();
    virtual BlockPos* next();

private:
    BlockPos* currentPos;
    BlockPos* minPos;
    BlockPos* maxPos;
};

#endif
