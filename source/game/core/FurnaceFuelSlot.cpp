#include "game/core/FurnaceFuelSlot.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"

FurnaceFuelSlot::FurnaceFuelSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

FurnaceFuelSlot::~FurnaceFuelSlot()
{
}

bool FurnaceFuelSlot::isBucket(not_null_ptr<ItemInstance> item)
{
    return item->getItem() == Items::bucket;
}

bool FurnaceFuelSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return FurnaceBlockEntity::isFuel(item) || isBucket(item);
}

int FurnaceFuelSlot::getMaxStackSize(not_null_ptr<ItemInstance> item)
{
    return isBucket(item) ? 1 : Slot::getMaxStackSize();
}
