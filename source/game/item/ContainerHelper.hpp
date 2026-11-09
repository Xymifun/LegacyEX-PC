#ifndef GAME_ITEM_CONTAINERHELPER_HPP
#define GAME_ITEM_CONTAINERHELPER_HPP

#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class ItemInstance;

class ContainerHelper
{
public:
    static not_null_ptr<ItemInstance> takeItem(NonNullList<not_null_ptr<ItemInstance> > items, unsigned int slot);
    static not_null_ptr<ItemInstance> removeItem(NonNullList<not_null_ptr<ItemInstance> > items, unsigned int slot,
                                                 unsigned int count);
    static CompoundTag* saveAllItems(CompoundTag* tag, NonNullList<not_null_ptr<ItemInstance> > items, bool saveEmpty);
    static CompoundTag* saveAllItems(CompoundTag* tag, NonNullList<not_null_ptr<ItemInstance> > items);
    static void loadAllItems(CompoundTag* tag, NonNullList<not_null_ptr<ItemInstance> > items);
};

#endif
