#ifndef GAME_WORLD_BLOCK_PURPURSLABBLOCKHALF_HPP
#define GAME_WORLD_BLOCK_PURPURSLABBLOCKHALF_HPP

#include "game/world/block/PurpurSlabBlock.hpp"

class PurpurSlabBlockHalf : public PurpurSlabBlock
{
public:
    PurpurSlabBlockHalf();
    virtual bool isFullSize() const;
};

#endif
