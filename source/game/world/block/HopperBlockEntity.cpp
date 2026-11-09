#include "game/world/block/HopperBlockEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include "game/core/Mth.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/entity/ai/ContainerEntitySelector.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/HopperBlock.hpp"

HopperBlockEntity::HopperBlockEntity()
    : RandomizableLockableContainerBlockEntity(),
      Hopper(),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY)),
      mCooldown(-1),
      mLastTick(0)
{
}

HopperBlockEntity::~HopperBlockEntity()
{
    delete[] mItems.items.data;
}

BlockEntity* HopperBlockEntity::create()
{
    return new HopperBlockEntity();
}

std::wstring HopperBlockEntity::getName()
{
    return hasCustomName() ? mCustomName->get() : std::wstring(L"");
}

std::wstring HopperBlockEntity::getCustomName()
{
    return hasCustomName() ? mCustomName->get() : std::wstring(L"");
}

unsigned int HopperBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool HopperBlockEntity::inventoryEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
        if (!mItems[i]->isEmpty())
            return false;
    return true;
}

bool HopperBlockEntity::isEmpty()
{
    return inventoryEmpty();
}

not_null_ptr<ItemInstance> HopperBlockEntity::removeItem(unsigned int slot, int count)
{
    unpackLootTable(boost::shared_ptr<Player>());
    return ContainerHelper::removeItem(mItems, slot, count);
}

void HopperBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    unpackLootTable(boost::shared_ptr<Player>());
    getItems()[slot] = itemInstance;
    if (itemInstance->getCount() > getMaxStackSize())
        itemInstance->setCount(getMaxStackSize());
}

int HopperBlockEntity::getMaxStackSize()
{
    return 64;
}

void HopperBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

AbstractContainerMenu* HopperBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                     boost::shared_ptr<Player> player)
{
    unpackLootTable(player);
    return new HopperMenu(inventory, boost::static_pointer_cast<HopperBlockEntity>(shared_from_this()), player);
}

int HopperBlockEntity::getTypeId(bool)
{
    return ClientboundContainerOpenPacket::HOPPER;
}

Level* HopperBlockEntity::getLevel()
{
    return BlockEntity::getLevel();
}

double HopperBlockEntity::getLevelX()
{
    return static_cast<double>(mPos.x) + 0.5;
}

double HopperBlockEntity::getLevelY()
{
    return static_cast<double>(mPos.y) + 0.5;
}

double HopperBlockEntity::getLevelZ()
{
    return static_cast<double>(mPos.z) + 0.5;
}

void HopperBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::HOPPER, L"Items"));
}

boost::shared_ptr<BlockEntity> HopperBlockEntity::clone()
{
    boost::shared_ptr<HopperBlockEntity> result(new HopperBlockEntity());
    BlockEntity::clone(result);
    result->mCustomName->set(mCustomName->raw(), std::wstring(mCustomName->getUUID()), mCustomName->isChatCensored(),
                             mCustomName->isUUIDLocal());
    result->mCooldown = mCooldown;
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
            result->mItems.set(i, mItems[i]->copy());
    }
    return result;
}

void HopperBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag))
        ContainerHelper::loadAllItems(tag, mItems);
    mCustomName->loadFromTag(tag);
    mCooldown = tag->getInt(L"TransferCooldown");
}

CompoundTag* HopperBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
    tag->putInt(L"TransferCooldown", mCooldown);
    return tag;
}

void HopperBlockEntity::tick()
{
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        --mCooldown;
        mLastTick = mLevel->getGameTime();
        if (!isOnCooldown())
        {
            setCooldown(0);
            tryMoveItems();
        }
    }
}

bool HopperBlockEntity::inventoryFull()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
        if (mItems[i]->isEmpty() || mItems[i]->getCount() != mItems[i]->getMaxStackSize())
            return false;
    return true;
}

std::vector<boost::shared_ptr<Entity> >* HopperBlockEntity::getItemsAtAndAbove(Level* level, double x, double y,
                                                                               double z)
{
    return level->getEntitiesOfClass(typeid(ItemEntity), AABB::newTemp(x - 0.5, y, z - 0.5, x + 0.5, y + 1.5, z + 0.5),
                                     EntitySelector::ENTITY_STILL_ALIVE, false);
}

boost::shared_ptr<Container> HopperBlockEntity::getContainerAt(Level* level, double x, double y, double z)
{
    boost::shared_ptr<Container> container;
    int blockX = Mth::floor(x);
    int blockY = Mth::floor(y);
    int blockZ = Mth::floor(z);
    BlockPos pos(blockX, blockY, blockZ);
    container = boost::dynamic_pointer_cast<Container>(level->getBlockEntity(pos));
    if (container)
    {
        if (boost::dynamic_pointer_cast<ChestBlockEntity>(container))
        {
            Block* block = level->getBlockState(pos)->getBlock();
            if (block != NULL && dynamic_cast<ChestBlock*>(block) != NULL)
                container = static_cast<ChestBlock*>(block)->getContainer(level, pos, true);
        }
    }
    if (!container)
    {
        ContainerEntitySelector selector;
        AABB* box = AABB::newTemp(x - 0.5, y - 0.5, z - 0.5, x + 0.5, y + 0.5, z + 0.5);
        std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(boost::shared_ptr<Entity>(), box,
                                                                               &selector, NULL);
        if (entities != NULL && !entities->empty())
            container = boost::dynamic_pointer_cast<Container>(
                entities->at(level->mRandom->nextInt(static_cast<int>(entities->size()))));
    }
    return container;
}

boost::shared_ptr<Container> HopperBlockEntity::getSourceContainer(Hopper* hopper)
{
    return getContainerAt(hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY() + 1.0, hopper->getLevelZ());
}

bool HopperBlockEntity::isEmptyContainer(boost::shared_ptr<Container> container, const Direction* direction)
{
    boost::shared_ptr<WorldlyContainer> worldly = boost::dynamic_pointer_cast<WorldlyContainer>(container);
    if (worldly)
    {
        arrayWithLength<int> slots = worldly->getSlotsForFace(direction);
        for (unsigned int i = 0; i < slots.length; ++i)
        {
            if (!worldly->getItem(slots.data[i])->isEmpty())
                return false;
        }
    }
    else
    {
        int size = container->getContainerSize();
        for (int i = 0; i < size; ++i)
        {
            if (!container->getItem(i)->isEmpty())
                return false;
        }
    }
    return true;
}

bool HopperBlockEntity::canTakeItemFromContainer(Container* container, not_null_ptr<ItemInstance> item, int slot,
                                                 const Direction* direction)
{
    if (dynamic_cast<WorldlyContainer*>(container) != NULL
        && !dynamic_cast<WorldlyContainer*>(container)->canTakeItemThroughFace(slot, item, direction))
        return false;
    return true;
}

bool HopperBlockEntity::canPlaceItemInContainer(Container* container, not_null_ptr<ItemInstance> item, int slot,
                                                const Direction* direction)
{
    if (!container->canPlaceItem(slot, item))
        return false;
    if (dynamic_cast<WorldlyContainer*>(container) != NULL
        && !dynamic_cast<WorldlyContainer*>(container)->canPlaceItemThroughFace(slot, item, direction))
        return false;
    return true;
}

bool HopperBlockEntity::canMergeItems(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second)
{
    if (first->getItem() != second->getItem())
        return false;
    if (first->getAuxValue() != second->getAuxValue())
        return false;
    if (first->getCount() > first->getMaxStackSize())
        return false;
    if (!ItemInstance::tagMatches(first, second))
        return false;
    return true;
}

not_null_ptr<ItemInstance> HopperBlockEntity::tryMoveInItem(Container* source, Container* destination,
                                                            not_null_ptr<ItemInstance> item, int slot,
                                                            const Direction* direction)
{
    not_null_ptr<ItemInstance> existing = destination->getItem(slot);
    if (canPlaceItemInContainer(destination, item, slot, direction))
    {
        bool changed = false;
        bool wasEmpty = destination->isEmpty();
        if (existing->isEmpty())
        {
            destination->setItem(slot, item);
            item = ItemInstance::EMPTY;
            changed = true;
        }
        else if (canMergeItems(existing, item))
        {
            int space = item->getMaxStackSize() - existing->getCount();
            int count = item->getCount();
            int amount = count < space ? count : space;
            item->shrink(amount);
            existing->grow(amount);
            changed = amount > 0;
        }
        if (changed)
        {
            HopperBlockEntity* destinationHopper = dynamic_cast<HopperBlockEntity*>(destination);
            if (wasEmpty && destinationHopper != NULL && !destinationHopper->isOnCustomCooldown())
            {
                int reduction = 0;
                if (source != NULL && dynamic_cast<HopperBlockEntity*>(source) != NULL)
                    reduction = 1;
                destinationHopper->setCooldown(8 - reduction);
            }
            destination->setChanged();
        }
    }
    return item;
}

not_null_ptr<ItemInstance> HopperBlockEntity::addItem(Container* source, Container* destination,
                                                      not_null_ptr<ItemInstance> item, const Direction* direction)
{
    if (destination != NULL && dynamic_cast<WorldlyContainer*>(destination) != NULL && direction != NULL)
    {
        WorldlyContainer* worldly = dynamic_cast<WorldlyContainer*>(destination);
        arrayWithLength<int> slots = worldly->getSlotsForFace(direction);
        for (unsigned int i = 0; i < slots.length && !item->isEmpty(); ++i)
        {
            item = tryMoveInItem(source, destination, item, slots.data[i], direction);
        }
    }
    else
    {
        int size = destination->getContainerSize();
        for (int i = 0; i < size && !item->isEmpty(); ++i)
        {
            item = tryMoveInItem(source, destination, item, i, direction);
        }
    }
    return item;
}

bool HopperBlockEntity::tryTakeInItemFromSlot(Hopper* hopper, Container* container, int slot,
                                              const Direction* direction)
{
    not_null_ptr<ItemInstance> item = container->getItem(slot);
    if (!item->isEmpty() && canTakeItemFromContainer(container, item, slot, direction))
    {
        not_null_ptr<ItemInstance> original = item->copy();
        not_null_ptr<ItemInstance> remainder = addItem(container, hopper, container->removeItem(slot, 1), NULL);
        if (remainder->isEmpty())
        {
            container->setChanged();
            return true;
        }
        container->setItem(slot, original);
    }
    return false;
}

bool HopperBlockEntity::addItem(Container* source, Container* destination, const boost::shared_ptr<ItemEntity>& entity)
{
    bool added = false;
    if (entity.get() == NULL)
        return false;
    not_null_ptr<ItemInstance> copy = entity->getItem()->copy();
    not_null_ptr<ItemInstance> remaining = addItem(source, destination, copy, NULL);
    if (remaining->isEmpty())
    {
        added = true;
        entity->remove();
    }
    else
    {
        entity->setItem(remaining);
    }
    return added;
}

bool HopperBlockEntity::suckInItems(Hopper* hopper)
{
    boost::shared_ptr<Container> source = getSourceContainer(hopper);
    const Direction* direction = Direction::DOWN;
    if (!source)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = getItemsAtAndAbove(
            hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY(), hopper->getLevelZ());
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            if (addItem(NULL, hopper, boost::dynamic_pointer_cast<ItemEntity>(*it)))
            {
                delete entities;
                return true;
            }
        }
        delete entities;
    }
    else
    {
        if (isEmptyContainer(source, direction))
            return false;
        boost::shared_ptr<WorldlyContainer> worldly = boost::dynamic_pointer_cast<WorldlyContainer>(source);
        if (!worldly)
        {
            int size = source->getContainerSize();
            for (int i = 0; i < size; ++i)
                if (tryTakeInItemFromSlot(hopper, source.get(), i, direction))
                    return true;
        }
        else
        {
            arrayWithLength<int> slots = worldly->getSlotsForFace(direction);
            for (unsigned int i = 0; i < slots.length; ++i)
                if (tryTakeInItemFromSlot(hopper, source.get(), slots.data[i], direction))
                    return true;
        }
    }
    return false;
}

bool HopperBlockEntity::tryMoveItems()
{
    if (mLevel == NULL || mLevel->mIsClientSide || isOnCooldown())
        return false;
    if (!HopperBlock::isTurnedOn(getData()))
        return false;
    bool moved = false;
    if (!inventoryEmpty())
        moved = ejectItems();
    if (!inventoryFull())
        moved = suckInItems(this) || moved;
    if (moved)
    {
        setCooldown(8);
        setChanged();
        return true;
    }
    return false;
}

bool HopperBlockEntity::isFullContainer(boost::shared_ptr<Container> container, const Direction* direction)
{
    boost::shared_ptr<WorldlyContainer> worldly = boost::dynamic_pointer_cast<WorldlyContainer>(container);
    if (worldly)
    {
        arrayWithLength<int> slots = worldly->getSlotsForFace(direction);
        for (unsigned int i = 0; i < slots.length; ++i)
        {
            not_null_ptr<ItemInstance> item = worldly->getItem(slots.data[i]);
            if (item->isEmpty() || item->getCount() != item->getMaxStackSize())
                return false;
        }
    }
    else
    {
        int size = container->getContainerSize();
        for (int i = 0; i < size; ++i)
        {
            not_null_ptr<ItemInstance> item = container->getItem(i);
            if (item->isEmpty() || item->getCount() != item->getMaxStackSize())
                return false;
        }
    }
    return true;
}

bool HopperBlockEntity::ejectItems()
{
    boost::shared_ptr<Container> destination = getAttachedContainer();
    if (!destination)
        return false;
    const Direction* face = HopperBlock::getAttachedFace(getData())->getOpposite();
    if (isFullContainer(destination, face))
        return false;
    for (unsigned int i = 0; i < getContainerSize(); ++i)
    {
        if (!getItem(i)->isEmpty())
        {
            not_null_ptr<ItemInstance> original = getItem(i)->copy();
            not_null_ptr<ItemInstance> remaining = addItem(this, destination.get(), removeItem(i, 1), face);
            if (remaining->isEmpty())
            {
                destination->setChanged();
                return true;
            }
            setItem(i, original);
        }
    }
    return false;
}

boost::shared_ptr<Container> HopperBlockEntity::getAttachedContainer()
{
    const Direction* face = HopperBlock::getAttachedFace(getData());
    return getContainerAt(getLevel(), getLevelX() + face->getStepX(), getLevelY() + face->getStepY(),
                          getLevelZ() + face->getStepZ());
}

void HopperBlockEntity::setCooldown(int cooldown)
{
    mCooldown = cooldown;
}

bool HopperBlockEntity::isOnCooldown()
{
    return mCooldown > 0;
}

bool HopperBlockEntity::isOnCustomCooldown()
{
    return mCooldown > 8;
}

NonNullList<not_null_ptr<ItemInstance> > HopperBlockEntity::getItems()
{
    return mItems;
}

void HopperBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    delete[] mItems.items.data;
    mItems = items;
}

std::wstring HopperBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

unsigned long long HopperBlockEntity::GetType()
{
    return BlockEntityType::HOPPER;
}
