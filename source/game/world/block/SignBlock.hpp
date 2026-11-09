#ifndef GAME_WORLD_BLOCK_SIGNBLOCK_HPP
#define GAME_WORLD_BLOCK_SIGNBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Item;
class Level;
class LevelSource;
class Random;
class Player;
class TextureAtlasSprite;

class SignBlock : public BaseEntityBlock
{
public:
    SignBlock();
    virtual ~SignBlock() {}

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isPossibleToRespawnInThis();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
