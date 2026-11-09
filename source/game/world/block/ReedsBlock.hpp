#ifndef GAME_WORLD_BLOCK_REEDSBLOCK_HPP
#define GAME_WORLD_BLOCK_REEDSBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class ReedsBlock : public Block, public BonemealableBlock
{
public:
    ReedsBlock();
    virtual bool isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool);
    virtual bool isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*);
    virtual void performBonemeal(Level*, Random*, const BlockPos&, const BlockState*);
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual bool mayPlace(Level*, const BlockPos&);
    virtual void neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&);
    virtual bool canSurvive(Level*, const BlockPos&, const BlockState*);
    virtual const AABB* getClipAABB(const BlockState*, LevelSource*, const BlockPos&);
    virtual Item* getResource(const BlockState*, Random*, int);
    virtual bool isSolidRender(const BlockState*) const;
    virtual bool isCubeShaped(const BlockState*);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level*, const BlockPos&, const BlockState*);
    virtual int getColor(LevelSource*, const BlockPos&, int);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState*) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*);
    virtual bool shouldBlockTick(Level*, const BlockPos&, const BlockState*);
    virtual EBlockRenderShape getRenderShape(const BlockState*);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder&);

private:
    bool checkAlive(Level*, const BlockPos&, const BlockState*);
};

#endif
