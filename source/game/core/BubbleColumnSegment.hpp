#ifndef GAME_CORE_BUBBLECOLUMNSEGMENT_HPP
#define GAME_CORE_BUBBLECOLUMNSEGMENT_HPP

#include "game/world/block/Block.hpp"

template <class T> class AbstractProperty;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Entity;
class Level;
class LevelSource;
class Material;
class Random;

class BubbleColumnSegment : public Block
{
public:
    static AbstractProperty<bool>* DRAG;

    BubbleColumnSegment(Material* material);

    virtual bool isCubeShaped(const BlockState* state);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    static bool getFlowDownward(const BlockState* state);
    bool isValidBubbleColumnLocation(Level* level, const BlockPos& pos);
    bool addBubbleColumnSegment(Level* level, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    static void RectifyBubbleFlow(Level* level, const BlockPos& pos, bool down);
    virtual int getTickDelay(Level* level);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canInstantlyTick();
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool mayPick();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual int getRenderLayer();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();

private:
    int mUnknown;
};

#endif
