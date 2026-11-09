#ifndef GAME_WORLD_BLOCK_ENDERCHESTBLOCK_HPP
#define GAME_WORLD_BLOCK_ENDERCHESTBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseEntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class DirectionProperty;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;
class Mirror;
class Player;
class Random;
class Rotation;
class Item;
class ItemInstance;

class EnderChestBlock : public BaseEntityBlock
{
public:
    static DirectionProperty* FACING;

    EnderChestBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual bool isSilkTouchable();
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
