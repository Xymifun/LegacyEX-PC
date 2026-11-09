#ifndef GAME_WORLD_BLOCK_HARDENEDCLAYBLOCK_HPP
#define GAME_WORLD_BLOCK_HARDENEDCLAYBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class LevelSource;
class MaterialColor;

class HardenedClayBlock : public Block
{
public:
    HardenedClayBlock();

    virtual const MaterialColor* getMapColor(const BlockState* blockState, LevelSource* levelSource,
                                             const BlockPos& pos);
};

#endif
