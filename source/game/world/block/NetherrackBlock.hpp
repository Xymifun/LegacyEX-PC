#ifndef GAME_WORLD_BLOCK_NETHERRACKBLOCK_HPP
#define GAME_WORLD_BLOCK_NETHERRACKBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class LevelSource;
class MaterialColor;

class NetherrackBlock : public Block
{
public:
    NetherrackBlock();

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
};

#endif
