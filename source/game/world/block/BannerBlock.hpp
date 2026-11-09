#ifndef GAME_WORLD_BLOCK_BANNERBLOCK_HPP
#define GAME_WORLD_BLOCK_BANNERBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class Direction;
class DirectionProperty;
class Item;
class Level;
class LevelSource;
class Random;

class BannerBlock : public BaseEntityBlock
{
public:
    BannerBlock();
    virtual ~BannerBlock() {}
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int);

    static DirectionProperty* FACING;
    static void blockStaticCtor();

    virtual std::wstring getName();
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isPossibleToRespawnInThis();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    not_null_ptr<ItemInstance> getItemInstanceFromBlockEntity(Level* level, const BlockPos& pos,
                                                              const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
};

#endif
