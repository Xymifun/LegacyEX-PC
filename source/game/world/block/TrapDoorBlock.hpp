#ifndef GAME_WORLD_BLOCK_TRAPDOORBLOCK_HPP
#define GAME_WORLD_BLOCK_TRAPDOORBLOCK_HPP

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
class Material;
class MaterialColor;
class Mirror;
class Player;
class Rotation;
class LivingEntity;

template <class T> class AbstractProperty;

class TrapDoorBlock : public Block
{
public:
    class Half : public StringRepresentable
    {
    public:
        Half(const std::wstring& name) : mName(name) {}
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring mName;
    };

    static AbstractProperty<Half*>* HALF;
    static AbstractProperty<bool>* OPEN;
    static AbstractProperty<const Direction*>* FACING;
    static Half* TOP;
    static Half* BOTTOM;
    static Half* HALVES[2];
    static const AABB* EAST_OPEN_AABB;
    static const AABB* WEST_OPEN_AABB;
    static const AABB* SOUTH_OPEN_AABB;
    static const AABB* NORTH_OPEN_AABB;
    static const AABB* BOTTOM_AABB;
    static const AABB* TOP_AABB;

    TrapDoorBlock(Material* material);
    TrapDoorBlock(Material* material, const MaterialColor* materialColor);
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool TestUse();
    void playSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos, bool open);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    static const Direction* getFacingFromData(int data);
    static unsigned char getDataFromFacing(const Direction* direction);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
