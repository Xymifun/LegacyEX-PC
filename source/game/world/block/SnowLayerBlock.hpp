#ifndef GAME_WORLD_BLOCK_SNOWLAYERBLOCK_HPP
#define GAME_WORLD_BLOCK_SNOWLAYERBLOCK_HPP

#include "game/world/block/FallingBlock.hpp"
#include "game/item/not_null_ptr.hpp"

template <class T> class AbstractProperty;
class AABB;
class BlockStateDefinition;
class Item;
class ItemInstance;
class Player;
class BlockEntity;

#include <boost/shared_ptr.hpp>

class SnowLayerBlock : public FallingBlock
{
public:
    static AbstractProperty<int>* LAYERS;
    static const AABB* SNOW_AABB[9];

    SnowLayerBlock();
    static const BlockState* buildSnowBlock(Level* level, const BlockPos& pos, int layers, int addLayers);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isTopSolidBlocking(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    static double getHeight(Level* level, int x, int y, int z);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool isWaterBlocking() const;
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    int melt(Level* level, const BlockPos& pos, int amount);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual const BlockState* getBlockState(int data);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual bool hasInHandRenderOffset();
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
};

#endif
