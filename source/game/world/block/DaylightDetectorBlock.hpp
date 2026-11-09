#ifndef GAME_WORLD_BLOCK_DAYLIGHTDETECTORBLOCK_HPP
#define GAME_WORLD_BLOCK_DAYLIGHTDETECTORBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class IconRegister;
class Item;
class Level;
class LevelSource;
class Player;
class Random;
class TextureAtlasSprite;

class DaylightDetectorBlock : public BaseEntityBlock
{
public:
    DaylightDetectorBlock(bool inverted);

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual void updateSignalStrength(Level* level, const BlockPos& pos);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isInverted();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool isSignalSource(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mInvertedTopIcon;
    TextureAtlasSprite* mSideIcon;
    bool mInverted;
};

#endif
