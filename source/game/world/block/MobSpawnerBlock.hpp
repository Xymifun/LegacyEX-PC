#ifndef GAME_WORLD_BLOCK_MOBSPAWNERBLOCK_HPP
#define GAME_WORLD_BLOCK_MOBSPAWNERBLOCK_HPP

#include "game/world/block/BaseEntityBlock.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;
class BlockPos;
class BlockState;
class Direction;
class Level;
class Random;
class ItemInstance;

class MobSpawnerBlock : public BaseEntityBlock
{
public:
    MobSpawnerBlock();

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual int getRenderLayer();
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);
};

#endif
