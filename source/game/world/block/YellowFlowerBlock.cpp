#include "game/world/block/YellowFlowerBlock.hpp"

YellowFlowerBlock::YellowFlowerBlock()
{
    DerivedInit();
}

FlowerBlock::FlowerCategory* YellowFlowerBlock::getFlowerCategory()
{
    return FlowerCategory::YELLOW;
}
