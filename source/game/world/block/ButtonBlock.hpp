#ifndef GAME_WORLD_BLOCK_BUTTONBLOCK_HPP
#define GAME_WORLD_BLOCK_BUTTONBLOCK_HPP

#include "game/world/block/Block.hpp"

template <class T> class AbstractProperty;
class DirectionProperty;

class ButtonBlock : public Block
{
public:
    static DirectionProperty* FACING;
    static AbstractProperty<bool>* POWERED;
    static const AABB* AABB_DOWN_OFF;
    static const AABB* AABB_UP_OFF;
    static const AABB* AABB_NORTH_OFF;
    static const AABB* AABB_SOUTH_OFF;
    static const AABB* AABB_WEST_OFF;
    static const AABB* AABB_EAST_OFF;
    static const AABB* AABB_DOWN_ON;
    static const AABB* AABB_UP_ON;
    static const AABB* AABB_NORTH_ON;
    static const AABB* AABB_SOUTH_ON;
    static const AABB* AABB_WEST_ON;
    static const AABB* AABB_EAST_ON;

    ButtonBlock(bool wooden, Material* material);
    static void blockStaticCtor();
    static bool canAttachTo(Level* level, const BlockPos& pos, const Direction* direction);
    bool checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    void updateNeighbours(Level* level, const BlockPos& pos, const Direction* direction);
    void checkPressed(const BlockState* state, Level* level, const BlockPos& pos);

    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual ~ButtonBlock() {}
    virtual void DerivedInit();
    virtual bool isCubeShaped(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getTickDelay(Level* level);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual bool isSignalSource(const BlockState* state);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void playOnSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos) = 0;
    virtual void playOffSound(Level* level, const BlockPos& pos) = 0;

private:
    bool mWooden;
};

#endif
