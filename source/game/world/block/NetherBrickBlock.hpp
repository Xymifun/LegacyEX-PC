#ifndef GAME_WORLD_BLOCK_NETHERBRICKBLOCK_HPP
#define GAME_WORLD_BLOCK_NETHERBRICKBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class LevelSource;
class MaterialColor;

class NetherBrickBlock : public Block
{
public:
    NetherBrickBlock();

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
};

#endif
