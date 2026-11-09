#ifndef GAME_WORLD_BLOCK_PRISMARINESLABBLOCKHALF_HPP
#define GAME_WORLD_BLOCK_PRISMARINESLABBLOCKHALF_HPP

#include "game/world/block/PrismarineSlabBlock.hpp"

class PrismarineSlabBlockHalf : public PrismarineSlabBlock
{
public:
    PrismarineSlabBlockHalf();
    virtual bool isFullSize() const;
};

#endif
