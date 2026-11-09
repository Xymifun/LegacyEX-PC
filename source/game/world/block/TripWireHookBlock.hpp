#ifndef GAME_WORLD_BLOCK_TRIPWIREHOOKBLOCK_HPP
#define GAME_WORLD_BLOCK_TRIPWIREHOOKBLOCK_HPP

#include "game/world/block/Block.hpp"

template <class T> class AbstractProperty;
class DirectionProperty;

class TripWireHookBlock : public Block
{
public:
    static DirectionProperty* FACING;
    static AbstractProperty<bool>* POWERED;
    static AbstractProperty<bool>* ATTACHED;
    static AbstractProperty<bool>* SUSPENDED;
    static const AABB* AABB_DOWN;
    static const AABB* AABB_UP;
    static const AABB* AABB_NORTH;
    static const AABB* AABB_SOUTH;
    static const AABB* AABB_WEST;
    static const AABB* AABB_EAST;

    TripWireHookBlock();

    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    void notifyNeighbors(Level* level, const BlockPos& pos, const Direction* direction);
    void playSound(Level* level, const BlockPos& pos, bool attached, bool powered, bool oldAttached, bool oldPowered);
    void calculateState(Level* level, const BlockPos& pos, const BlockState* state, bool isBeingDestroyed,
                        bool canUpdate, int wireSource, const BlockState* wireSourceState);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    bool checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual bool isSignalSource(const BlockState* state);
    virtual int getRenderLayer();
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
