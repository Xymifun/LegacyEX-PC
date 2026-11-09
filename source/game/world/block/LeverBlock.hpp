#ifndef GAME_WORLD_BLOCK_LEVERBLOCK_HPP
#define GAME_WORLD_BLOCK_LEVERBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/core/Direction.hpp"
#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Level;
class LevelSource;
class LivingEntity;
class Mirror;
class Player;
class Rotation;
template <class T> class AbstractProperty;

class LeverBlock : public Block
{
public:
    class LeverDirection : public StringRepresentable
    {
    public:
        static LeverDirection* DOWN_X;
        static LeverDirection* EAST;
        static LeverDirection* WEST;
        static LeverDirection* SOUTH;
        static LeverDirection* NORTH;
        static LeverDirection* UP_Z;
        static LeverDirection* UP_X;
        static LeverDirection* DOWN_Z;
        static LeverDirection* BY_DATA[8];

        LeverDirection(int data, const std::wstring& name, const Direction* direction);
        int getData();
        static void staticCtor();
        static LeverDirection* fromFacing(const Direction* face, const Direction* direction);
        const Direction* getDirection();
        static LeverDirection* byData(int data);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
        const Direction* mDirection;
    };

    static AbstractProperty<LeverDirection*>* FACING;
    static AbstractProperty<bool>* POWERED;
    static const AABB* AABB_NORTH;
    static const AABB* AABB_SOUTH;
    static const AABB* AABB_WEST;
    static const AABB* AABB_EAST;
    static const AABB* AABB_UP;
    static const AABB* AABB_DOWN;

    LeverBlock();
    static void blockStaticCtor();
    static bool canAttachTo(Level* level, const BlockPos& pos, const Direction* direction);
    static int getLeverFacing(const Direction* direction);

    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual bool isCubeShaped(const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
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
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
};

#endif
