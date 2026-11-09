#include "game/item/ItemInstanceClonerBuilder.hpp"

not_null_ptr<ItemInstance> ItemInstanceClonerBuilder::build() const
{
    return mItem.copy();
}
