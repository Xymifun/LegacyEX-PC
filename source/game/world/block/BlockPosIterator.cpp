#include "game/world/block/BlockPosIterator.hpp"

#include <algorithm>

#include "game/world/block/BlockPos.hpp"

BlockPosIterator::BlockPosIterator(const BlockPos& from, const BlockPos& to)
{
    this->currentPos = NULL;
    this->minPos = new BlockPos(std::min(from.x, to.x), std::min(from.y, to.y), std::min(from.z, to.z));
    this->maxPos = new BlockPos(std::max(from.x, to.x), std::max(from.y, to.y), std::max(from.z, to.z));
}

BlockPosIterator::~BlockPosIterator()
{
    delete this->currentPos;
    delete this->minPos;
    delete this->maxPos;
}

bool BlockPosIterator::hasNext()
{
    return this->currentPos == NULL || !this->currentPos->equals(this->maxPos);
}

BlockPos* BlockPosIterator::next()
{
    if (this->currentPos == NULL)
    {
        this->currentPos = new BlockPos(*this->minPos);
    }
    else
    {
        int x = this->currentPos->x;
        int y = this->currentPos->y;
        int z = this->currentPos->z;
        if (x < this->maxPos->x)
        {
            x = x + 1;
        }
        else if (y < this->maxPos->y)
        {
            x = this->minPos->x;
            y = y + 1;
        }
        else if (z < this->maxPos->z)
        {
            x = this->minPos->x;
            y = this->minPos->y;
            z = z + 1;
        }
        const BlockPos& moved = BlockPos(x, y, z);
        *this->currentPos = moved;
    }

    return this->currentPos;
}
