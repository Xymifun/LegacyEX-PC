#ifndef GAME_WORLD_BLOCK_WEBBLOCK_HPP
#define GAME_WORLD_BLOCK_WEBBLOCK_HPP

#include "game/world/block/Block.hpp"

class WebBlock : public Block
{
public:
    WebBlock();

    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool isSilkTouchable();
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual int getRenderLayer();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
};

#endif
