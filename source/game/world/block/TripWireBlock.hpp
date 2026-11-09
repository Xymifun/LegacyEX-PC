#ifndef GAME_WORLD_BLOCK_TRIPWIREBLOCK_HPP
#define GAME_WORLD_BLOCK_TRIPWIREBLOCK_HPP

#include "game/world/block/Block.hpp"

template <class T> class AbstractProperty;

class TripWireBlock : public Block
{
public:
    static AbstractProperty<bool>* POWERED;
    static AbstractProperty<bool>* SUSPENDED;
    static AbstractProperty<bool>* ATTACHED;
    static AbstractProperty<bool>* DISARMED;
    static AbstractProperty<bool>* NORTH;
    static AbstractProperty<bool>* EAST;
    static AbstractProperty<bool>* SOUTH;
    static AbstractProperty<bool>* WEST;

    TripWireBlock();

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    static bool shouldConnectTo(LevelSource* levelSource, const BlockPos& pos, const BlockState* state,
                                const Direction* direction);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getRenderLayer();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    void updateSource(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    void checkPressed(Level* level, const BlockPos& pos);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
