#ifndef GAME_WORLD_BLOCK_STONEBUTTONBLOCK_HPP
#define GAME_WORLD_BLOCK_STONEBUTTONBLOCK_HPP

#include "game/world/block/ButtonBlock.hpp"

class StoneButtonBlock : public ButtonBlock
{
public:
    StoneButtonBlock();
    virtual void playOnSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos);
    virtual void playOffSound(Level* level, const BlockPos& pos);
};

#endif
