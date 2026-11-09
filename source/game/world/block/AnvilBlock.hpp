#ifndef GAME_WORLD_BLOCK_ANVILBLOCK_HPP
#define GAME_WORLD_BLOCK_ANVILBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/BlockPos.hpp"

template <class T> class AbstractProperty;
class DirectionProperty;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class FallingBlockEntity;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;
class Player;
class Rotation;
class Inventory;
class AbstractContainerMenu;
class TextureAtlasSprite;

class AnvilBlock : public FallingBlock
{
public:
    class Anvil : public InteractionObject
    {
    public:
        Anvil(Level* level, const BlockPos& pos);

        virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory,
                                                  boost::shared_ptr<Player> player);
        virtual int getTypeId(bool large);
        virtual std::wstring getName();
        virtual std::wstring getCustomName();
        virtual bool hasCustomName();
        virtual std::wstring getDisplayName();

    private:
        Level* mLevel;
        BlockPos mBlockPos;
    };

    static DirectionProperty* FACING;
    static AbstractProperty<int>* DAMAGE;

    AnvilBlock();
    static void blockStaticCtor();
    virtual bool isCubeShaped(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual void falling(boost::shared_ptr<FallingBlockEntity> entity);
    virtual void onLand(Level* level, const BlockPos& pos, const BlockState* state, const BlockState* replacedState);
    virtual void onBroken(Level* level, const BlockPos& pos);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    friend class BlockRenderer;

    int mTextureType;
    TextureAtlasSprite* mTopIcons[3];
};

#endif
