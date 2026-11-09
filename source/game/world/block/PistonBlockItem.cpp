#include "game/world/block/PistonBlockItem.hpp"

PistonBlockItem::PistonBlockItem(Block* block) : BlockItem(block)
{
}

int PistonBlockItem::getLevelDataForAuxValue(int)
{
    return 7;
}
