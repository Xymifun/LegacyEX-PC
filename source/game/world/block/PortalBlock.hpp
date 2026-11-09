#ifndef GAME_WORLD_BLOCK_PORTALBLOCK_HPP
#define GAME_WORLD_BLOCK_PORTALBLOCK_HPP

#include "game/world/block/HalfTransparentBlock.hpp"
#include "game/world/block/BlockPattern.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Level;
class LevelSource;
class Random;
class Rotation;
class ItemInstance;
class Entity;
template <typename T> class EnumProperty;

class PortalBlock : public HalfTransparentBlock
{
public:
    class PortalShape
    {
    public:
        PortalShape(Level* level, const BlockPos& pos, Direction::Axis* axis);

        int getHeight();
        bool isEmptyBlock(Block* block);
        int getDistanceUntilEdge(const BlockPos& pos, const Direction* direction);
        int calculatePortalHeight();
        bool isValid();
        int getWidth();
        void createPortalBlocks();

        Level* mLevel;
        Direction::Axis* mAxis;
        const Direction* mRightDir;
        const Direction* mLeftDir;
        int mNumPortalBlocks;
        BlockPos* mBottomLeft;
        int mHeight;
        int mWidth;
    };

    PortalBlock();

    static EnumProperty<Direction::Axis*>* AXIS;

    static void blockStaticCtor();
    BlockPattern::BlockPatternMatch* getPortalShape(Level* level, const BlockPos& pos);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    static int getDataForAxis(Direction::Axis* axis);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool trySpawnPortal(Level* level, const BlockPos& pos, bool create);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual int getResourceCount(Random* random);
    virtual int getRenderLayer();
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isWaterBlocking() const;
    virtual bool waterSpreadCausesSpawn();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool mayPick();
};

#endif
