#include "game/item/EmptyLootItem.hpp"

uint32_t EmptyLootItem::_createItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                                    LootTableContext& context)
{
    return 0;
}
