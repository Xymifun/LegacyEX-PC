#include "game/world/block/MutableBlockPosIterator.hpp"

#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"

MutableBlockPosIterator::MutableBlockPosIterator(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    this->minX = minX;
    this->minY = minY;
    this->minZ = minZ;
    this->maxX = maxX;
    this->maxY = maxY;
    this->maxZ = maxZ;
    this->currentPos = NULL;
}

MutableBlockPosIterator::~MutableBlockPosIterator()
{
    delete this->currentPos;
}

bool MutableBlockPosIterator::hasNext()
{
    return this->currentPos == NULL || !this->currentPos->equals(BlockPos(this->maxX, this->maxY, this->maxZ));
}

MutableBlockPos* MutableBlockPosIterator::next()
{
    if (this->currentPos == NULL)
    {
        this->currentPos = new MutableBlockPos(this->minX, this->minY, this->minZ);
    }
    else if (this->currentPos->x < this->maxX)
    {
        this->currentPos->x = this->currentPos->x + 1;
    }
    else if (this->currentPos->y < this->maxY)
    {
        this->currentPos->x = this->minX;
        this->currentPos->y = this->currentPos->y + 1;
    }
    else if (this->currentPos->z < this->maxZ)
    {
        this->currentPos->x = this->minX;
        this->currentPos->y = this->minY;
        this->currentPos->z = this->currentPos->z + 1;
    }

    return this->currentPos;
}
