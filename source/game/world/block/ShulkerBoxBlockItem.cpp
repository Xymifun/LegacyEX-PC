#include "game/world/block/ShulkerBoxBlockItem.hpp"

ShulkerBoxBlockItem::ShulkerBoxBlockItem(Block* block) : BlockItem(block)
{
    setMaxStackSize(1);
}
