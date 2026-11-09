#ifndef GAME_WORLD_BLOCK_MUTABLEBLOCKPOSITERATOR_HPP
#define GAME_WORLD_BLOCK_MUTABLEBLOCKPOSITERATOR_HPP

#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"

class MutableBlockPosIterator : public Iterator<MutableBlockPos*>
{
public:
    MutableBlockPosIterator(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
    virtual ~MutableBlockPosIterator();

    virtual bool hasNext();
    virtual MutableBlockPos* next();

private:
    MutableBlockPos* currentPos;
    int minX;
    int minY;
    int minZ;
    int maxX;
    int maxY;
    int maxZ;
};

#endif
