#ifndef GAME_WORLD_BLOCK_BLOCKCOLORS_HPP
#define GAME_WORLD_BLOCK_BLOCKCOLORS_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/IdMapper.hpp"

class Block;
class BlockPos;
class BlockState;
class LevelSource;

class BlockColor
{
public:
    virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex) = 0;
};

class BlockColors
{
public:
    BlockColors();

    void registerColor(BlockColor* color, arrayWithLength<Block*> blocks);

    static BlockColors* createDefault();

    int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex);

private:
    IdMapper<BlockColor*>* mBlockColorIds;
};

#endif
