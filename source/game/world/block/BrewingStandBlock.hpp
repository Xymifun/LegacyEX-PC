#ifndef GAME_WORLD_BLOCK_BREWINGSTANDBLOCK_HPP
#define GAME_WORLD_BLOCK_BREWINGSTANDBLOCK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseEntityBlock.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Entity;
class IconRegister;
class Item;
class ItemInstance;
class Level;
class LevelSource;
class LivingEntity;
class Player;
class Random;
class TextureAtlasSprite;

class BrewingStandBlock : public BaseEntityBlock
{
public:
    static AbstractProperty<bool>* HAS_BOTTLE_0;
    static AbstractProperty<bool>* HAS_BOTTLE_1;
    static AbstractProperty<bool>* HAS_BOTTLE_2;

    BrewingStandBlock();

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual std::wstring getName();
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    TextureAtlasSprite* getBaseTexture();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mBaseTexture;
};

#endif
