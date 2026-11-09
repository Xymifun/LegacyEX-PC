#include "game/item/SimpleFoiledItem.hpp"

SimpleFoiledItem::SimpleFoiledItem() : Item()
{
}

bool SimpleFoiledItem::isFoil(not_null_ptr<ItemInstance>)
{
    return true;
}
