#include "game/world/block/RedFlowerBlock.hpp"

RedFlowerBlock::RedFlowerBlock()
{
    DerivedInit();
}

FlowerBlock::FlowerCategory* RedFlowerBlock::getFlowerCategory()
{
    return FlowerCategory::RED;
}
