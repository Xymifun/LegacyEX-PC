#include "game/item/ContainerHelper.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"

not_null_ptr<ItemInstance> ContainerHelper::takeItem(NonNullList<not_null_ptr<ItemInstance> > items, unsigned int slot)
{
    if (slot >= items.size())
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> item = items[slot];
    items.set(slot, ItemInstance::EMPTY);
    return item;
}

not_null_ptr<ItemInstance> ContainerHelper::removeItem(NonNullList<not_null_ptr<ItemInstance> > items,
                                                       unsigned int slot, unsigned int count)
{
    if (slot >= items.size() || items[slot]->isEmpty())
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> split = items[slot]->split((int)count);
    return split;
}

CompoundTag* ContainerHelper::saveAllItems(CompoundTag* tag, NonNullList<not_null_ptr<ItemInstance> > items,
                                           bool saveEmpty)
{
    ListTag* list = new ListTag();
    for (unsigned int slot = 0; slot < items.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = items[slot];
        if (!item->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", (unsigned char)slot);
            item->save(itemTag);
            list->add(itemTag);
        }
    }
    if (!list->isEmpty() || saveEmpty)
    {
        tag->put(L"Items", list);
    }
    return tag;
}

CompoundTag* ContainerHelper::saveAllItems(CompoundTag* tag, NonNullList<not_null_ptr<ItemInstance> > items)
{
    return saveAllItems(tag, items, true);
}

void ContainerHelper::loadAllItems(CompoundTag* tag, NonNullList<not_null_ptr<ItemInstance> > items)
{
    ListTag* list = tag->getList(L"Items", Tag::TAG_COMPOUND);
    for (int index = 0; index < list->size(); ++index)
    {
        CompoundTag* itemTag = list->getCompound(index);
        unsigned int slot = (unsigned int)itemTag->getByte(L"Slot");
        if (slot < items.size())
        {
            ItemInstance* item = new ItemInstance(itemTag);
            items.set(slot, not_null_ptr<ItemInstance>(item));
        }
    }
}
