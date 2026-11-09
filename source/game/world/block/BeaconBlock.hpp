#ifndef GAME_WORLD_BLOCK_BEACONBLOCK_HPP
#define GAME_WORLD_BLOCK_BEACONBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseEntityBlock.hpp"
#include "game/core/InteractionHand.hpp"

class BeaconBlockEntity;
class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LivingEntity;
class Player;
class ItemInstance;

class BeaconBlock : public BaseEntityBlock
{
public:
    BeaconBlock();

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    void updateEntity(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool TestUse();
    virtual int getRenderLayer();
    static void updateValidityWithinBeam(Level* level, const BlockPos& pos);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool canContainLiquid(const BlockState* state) const;
    bool blocksLight();
};

#endif
