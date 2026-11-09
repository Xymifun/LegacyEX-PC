#include "game/world/block/DropperBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Container.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSourceImpl.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"

DropperBlock::DropperBlock() : DispenserBlock()
{
    Block::ClearDerivedInit();
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(TRIGGERED, false));
    mDropperBehavior = new DefaultDispenseItemBehavior();
}

boost::shared_ptr<BlockEntity> DropperBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<DropperBlockEntity>(new DropperBlockEntity());
    entity->setLevel(level);
    return entity;
}

DispenseItemBehavior* DropperBlock::getDispenseMethod(not_null_ptr<ItemInstance>)
{
    return mDropperBehavior;
}

void DropperBlock::dispenseFrom(Level* level, const BlockPos& pos)
{
    BlockSourceImpl source(level, pos);
    boost::shared_ptr<DispenserBlockEntity> dispenser = boost::dynamic_pointer_cast<DispenserBlockEntity>(
        source.getEntity());
    if (!dispenser)
    {
        return;
    }

    int slot = dispenser->getRandomSlot();
    if (slot < 0)
    {
        level->levelEvent(LevelEvent::SOUND_DISPENSER_FAIL, pos, 0);
        return;
    }

    not_null_ptr<ItemInstance> item = dispenser->getItem(static_cast<unsigned int>(slot));
    if (item->isEmpty())
        return;

    const Direction* direction = level->getBlockState(pos)->getValue(FACING);
    BlockPos destinationPos = pos.relative(direction);
    boost::shared_ptr<Container> container = HopperBlockEntity::getContainerAt(
        level, static_cast<double>(destinationPos.x), static_cast<double>(destinationPos.y),
        static_cast<double>(destinationPos.z));

    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    if (!container)
    {
        result = mDropperBehavior->dispense(&source, item);
    }
    else
    {
        result = HopperBlockEntity::addItem(dispenser.get(), container.get(), item->copy()->split(1),
                                            direction->getOpposite());
        if (result->isEmpty())
        {
            result = item->copy();
            result->shrink(1);
        }
        else
        {
            result = item->copy();
        }
    }
    dispenser->setItem(static_cast<unsigned int>(slot), result);
}

void DropperBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"furnace_side");
    mTopIcon = iconRegister->registerIcon(L"furnace_top");
    mFrontIcon = iconRegister->registerIcon(getIconName() + L"_front_horizontal");
    mFrontVerticalIcon = iconRegister->registerIcon(getIconName() + L"_front_vertical");
}

unsigned int DropperBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_OPEN;
    return Block::GetInteractTooltip(tooltipData);
}
