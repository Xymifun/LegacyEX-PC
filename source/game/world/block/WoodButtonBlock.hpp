#ifndef GAME_WORLD_BLOCK_WOODBUTTONBLOCK_HPP
#define GAME_WORLD_BLOCK_WOODBUTTONBLOCK_HPP

#include "game/world/block/ButtonBlock.hpp"

class WoodButtonBlock : public ButtonBlock
{
public:
    WoodButtonBlock();
    virtual void playOnSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos);
    virtual void playOffSound(Level* level, const BlockPos& pos);
};

#endif
