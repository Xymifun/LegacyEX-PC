#ifndef GAME_WORLD_BLOCK_OBSIDIANBLOCK_HPP
#define GAME_WORLD_BLOCK_OBSIDIANBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Item;
class LevelSource;
class MaterialColor;
class Random;

class ObsidianBlock : public Block
{
public:
    ObsidianBlock();

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
};

#endif
