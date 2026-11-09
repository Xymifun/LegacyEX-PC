#ifndef GAME_WORLD_BLOCK_BLOCKEVENTDATA_HPP
#define GAME_WORLD_BLOCK_BLOCKEVENTDATA_HPP

#include "game/world/block/BlockPos.hpp"

class Block;

class BlockEventData : public BlockPos
{
public:
    BlockEventData(const BlockPos& pos, Block* block, int eventId, int eventParam);
    BlockPos getPos();
    int getParamA();
    int getParamB();
    Block* getBlock();
    bool equals(BlockEventData& other);

private:
    Block* block;
    int eventId;
    int eventParam;
};

#endif
