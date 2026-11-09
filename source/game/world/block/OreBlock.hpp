#ifndef GAME_WORLD_BLOCK_OREBLOCK_HPP
#define GAME_WORLD_BLOCK_OREBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Level;
class MaterialColor;
class Random;

class OreBlock : public Block
{
public:
    OreBlock();
    OreBlock(const MaterialColor* materialColor);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
};

#endif
