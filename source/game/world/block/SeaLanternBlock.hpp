#ifndef GAME_WORLD_BLOCK_SEALANTERNBLOCK_HPP
#define GAME_WORLD_BLOCK_SEALANTERNBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Item;
class LevelSource;
class Material;
class MaterialColor;
class Random;

class SeaLanternBlock : public Block
{
public:
    SeaLanternBlock(Material* material);
    virtual int getResourceCount(Random* random);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSilkTouchable();
};

#endif
