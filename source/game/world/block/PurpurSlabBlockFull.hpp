#ifndef GAME_WORLD_BLOCK_PURPURSLABBLOCKFULL_HPP
#define GAME_WORLD_BLOCK_PURPURSLABBLOCKFULL_HPP

#include "game/world/block/PurpurSlabBlock.hpp"

class PurpurSlabBlockFull : public PurpurSlabBlock
{
public:
    PurpurSlabBlockFull();
    virtual bool isFullSize() const;
};

#endif
