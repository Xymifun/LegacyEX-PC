#ifndef GAME_WORLD_BLOCK_ENDGATEWAYBLOCK_HPP
#define GAME_WORLD_BLOCK_ENDGATEWAYBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseEntityBlock.hpp"
#include "game/world/block/EntityBlock.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/AbstractProperty.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class ItemInstance;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Random;

class EndGatewayBlock : public BaseEntityBlock
{
public:
    static AbstractProperty<int>* PORTAL_NUMBER;

    EndGatewayBlock(Material* material);

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getResourceCount(Random* random);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool isWaterBlocking() const;
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
};

#endif
