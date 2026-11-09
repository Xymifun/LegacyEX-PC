#include "game/world/block/BeaconBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/generation/Player.hpp"

BeaconBlock::BeaconBlock() : BaseEntityBlock(Material::GLASS, MaterialColor::DIAMOND)
{
    Block::DerivedInit();
    setDestroyTime(3.0f);
}

boost::shared_ptr<BlockEntity> BeaconBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<BeaconBlockEntity>(new BeaconBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

bool BeaconBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                      InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BeaconBlockEntity> beaconEntity = boost::dynamic_pointer_cast<BeaconBlockEntity>(
            level->getBlockEntity(pos));
        if (beaconEntity.get() != NULL)
            player->openContainer(beaconEntity);
    }
    return true;
}

bool BeaconBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool BeaconBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void BeaconBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                              boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BeaconBlockEntity> beaconEntity = boost::dynamic_pointer_cast<BeaconBlockEntity>(
            level->getBlockEntity(pos));
        if (beaconEntity.get() != NULL)
        {
            beaconEntity->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                        itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

void BeaconBlock::updateEntity(Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BeaconBlockEntity> beaconEntity = boost::dynamic_pointer_cast<BeaconBlockEntity>(
        level->getBlockEntity(pos));
    if (beaconEntity.get() != NULL)
    {
        beaconEntity->checkValidity();
        level->blockEvent(pos, this, 1, 0);
    }
}

void BeaconBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    updateEntity(level, pos);
}

bool BeaconBlock::TestUse()
{
    return true;
}

int BeaconBlock::getRenderLayer()
{
    return 0;
}

void BeaconBlock::updateValidityWithinBeam(Level* level, const BlockPos& pos)
{
    for (int y = pos.y - 1; y >= 0; y--)
    {
        BlockPos beamPos(pos.x, y, pos.z);
        if (!level->canSeeSky(pos))
            return;
        const BlockState* state = level->getBlockState(beamPos);
        if (state->getBlock() == Blocks::beacon)
        {
            static_cast<BeaconBlock*>(Blocks::beacon)->updateEntity(level, beamPos);
            return;
        }
    }
}

void BeaconBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
}

EBlockRenderShape BeaconBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_BEACON;
}

unsigned int BeaconBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

bool BeaconBlock::canContainLiquid(const BlockState*) const
{
    return true;
}
