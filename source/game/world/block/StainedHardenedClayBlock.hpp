#ifndef GAME_WORLD_BLOCK_STAINEDHARDENEDCLAYBLOCK_HPP
#define GAME_WORLD_BLOCK_STAINEDHARDENEDCLAYBLOCK_HPP

#include "game/world/block/ColoredBlock.hpp"

class BlockPos;
class BlockState;
class LevelSource;
class MaterialColor;

class StainedHardenedClayBlock : public ColoredBlock
{
public:
    StainedHardenedClayBlock();

    static void staticCtor();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
};

#endif
