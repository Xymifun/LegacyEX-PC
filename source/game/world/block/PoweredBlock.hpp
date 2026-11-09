#ifndef GAME_WORLD_BLOCK_POWEREDBLOCK_HPP
#define GAME_WORLD_BLOCK_POWEREDBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Direction;
class LevelSource;
class Material;
class MaterialColor;

class PoweredBlock : public Block
{
public:
    PoweredBlock(Material* material, const MaterialColor* materialColor);

    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual bool isSignalSource(const BlockState* state);
};

#endif
