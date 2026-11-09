#ifndef GAME_WORLD_BLOCK_PRISMARINESLABBLOCKFULL_HPP
#define GAME_WORLD_BLOCK_PRISMARINESLABBLOCKFULL_HPP

#include "game/world/block/PrismarineSlabBlock.hpp"

class PrismarineSlabBlockFull : public PrismarineSlabBlock
{
public:
    PrismarineSlabBlockFull();
    virtual bool isFullSize() const;
};

#endif
