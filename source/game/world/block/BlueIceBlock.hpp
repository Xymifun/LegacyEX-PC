#ifndef GAME_WORLD_BLOCK_BLUEICEBLOCK_HPP
#define GAME_WORLD_BLOCK_BLUEICEBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class LevelSource;
class Material;
class Random;

class BlueIceBlock : public Block
{
public:
    BlueIceBlock(Material* material);

    virtual int getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue);
    virtual int getResourceCount(Random* random);
};

#endif
