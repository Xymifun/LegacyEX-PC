#ifndef GAME_WORLD_BLOCK_ENCHANTMENTTABLEBLOCK_HPP
#define GAME_WORLD_BLOCK_ENCHANTMENTTABLEBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"
#include "game/world/block/EntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;
class Player;
class Random;
class TextureAtlasSprite;

class EnchantmentTableBlock : public BaseEntityBlock
{
public:
    EnchantmentTableBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);

private:
    TextureAtlasSprite* mTopTexture;
    TextureAtlasSprite* mBottomTexture;
};

#endif
