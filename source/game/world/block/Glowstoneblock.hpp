#ifndef GAME_WORLD_BLOCK_GLOWSTONEBLOCK_HPP
#define GAME_WORLD_BLOCK_GLOWSTONEBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Item;
class LevelSource;
class Material;
class MaterialColor;
class Random;

class Glowstoneblock : public Block
{
public:
    Glowstoneblock(Material* material);

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getResourceCount(Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
};

#endif
