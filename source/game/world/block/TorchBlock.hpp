#ifndef GAME_WORLD_BLOCK_TORCHBLOCK_HPP
#define GAME_WORLD_BLOCK_TORCHBLOCK_HPP

#include "game/world/block/Block.hpp"

class DirectionProperty;

class TorchBlock : public Block
{
public:
    TorchBlock();
    virtual ~TorchBlock() {}

    static void blockStaticCtor();
    static bool isTorch(int data);

    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    bool canStandOn(Level* level, const BlockPos& pos);
    bool canBePlacedOn(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool checkDoPop(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

    static DirectionProperty* FACING;
};

#endif
