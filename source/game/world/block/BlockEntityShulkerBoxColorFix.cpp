#include "game/world/block/BlockEntityShulkerBoxColorFix.hpp"

BlockEntityShulkerBoxColorFix::~BlockEntityShulkerBoxColorFix()
{
}

int BlockEntityShulkerBoxColorFix::getVersion()
{
    return 813;
}

CompoundTag* BlockEntityShulkerBoxColorFix::fix(CompoundTag* tag)
{
    if (tag->getString(L"id") == L"minecraft:shulker")
    {
        tag->remove(L"Color");
    }

    return tag;
}
