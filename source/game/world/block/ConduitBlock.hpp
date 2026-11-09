#ifndef GAME_WORLD_BLOCK_CONDUITBLOCK_HPP
#define GAME_WORLD_BLOCK_CONDUITBLOCK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class Direction;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;

class ConduitBlock : public BaseEntityBlock
{
public:
    ConduitBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool canContainLiquid(const BlockState* state) const;
};

#endif
