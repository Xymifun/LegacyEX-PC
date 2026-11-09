#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/ui/DispenserMenu.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockEntity.hpp"

namespace
{
Random gDispenserRandom;
}

DispenserBlockEntity::DispenserBlockEntity()
    : mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(9, ItemInstance::EMPTY))
{
}

DispenserBlockEntity::~DispenserBlockEntity()
{
    delete[] mItems.items.data;
}

BlockEntity* DispenserBlockEntity::create()
{
    return new DispenserBlockEntity();
}

std::wstring DispenserBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"");
}

std::wstring DispenserBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"");
}

unsigned int DispenserBlockEntity::getContainerSize()
{
    return 9;
}

bool DispenserBlockEntity::isEmpty()
{
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = mItems[slot];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int DispenserBlockEntity::getMaxStackSize()
{
    return 64;
}

void DispenserBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

AbstractContainerMenu* DispenserBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                        boost::shared_ptr<Player> player)
{
    unpackLootTable(player);
    return new DispenserMenu(inventory, boost::static_pointer_cast<DispenserBlockEntity>(shared_from_this()));
}

int DispenserBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::TRAP;
}

int DispenserBlockEntity::getRandomSlot()
{
    {
        boost::shared_ptr<Player> player;
        unpackLootTable(player);
    }

    int selected = -1;
    int count = 1;
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (!mItems[slot]->isEmpty() && gDispenserRandom.nextInt(count++) == 0)
        {
            selected = static_cast<int>(slot);
        }
    }
    return selected;
}

int DispenserBlockEntity::addItem(not_null_ptr<ItemInstance> item)
{
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (mItems[slot]->isEmpty())
        {
            setItem(slot, item);
            return static_cast<int>(slot);
        }
    }
    return -1;
}

void DispenserBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::DISPENSER, L"Items"));
}

void DispenserBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag))
    {
        ContainerHelper::loadAllItems(tag, mItems);
    }
    mCustomName->loadFromTag(tag);
}

CompoundTag* DispenserBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
    mCustomName->saveToTag(tag);
    return tag;
}

NonNullList<not_null_ptr<ItemInstance> > DispenserBlockEntity::getItems()
{
    return mItems;
}

void DispenserBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    delete[] mItems.items.data;
    mItems = items;
}

boost::shared_ptr<BlockEntity> DispenserBlockEntity::clone()
{
    boost::shared_ptr<DispenserBlockEntity> result(new DispenserBlockEntity());
    BlockEntity::clone(result);
    result->mCustomName->set(mCustomName->raw(), std::wstring(mCustomName->getUUID()), mCustomName->isChatCensored(),
                             mCustomName->isUUIDLocal());
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (!mItems[slot]->isEmpty())
        {
            result->mItems.set(slot, mItems[slot]->copy());
        }
    }
    return result;
}

unsigned long long DispenserBlockEntity::GetType()
{
    return BlockEntityType::DISPENSER;
}
