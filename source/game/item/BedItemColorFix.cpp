#include "game/item/BedItemColorFix.hpp"

#include <string>

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"

BedItemColorFix::~BedItemColorFix()
{
}

int BedItemColorFix::getVersion()
{
    return 1125;
}

CompoundTag* BedItemColorFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"minecraft:bed") && tag->getShort(L"Damage") == 0)
    {
        tag->putShort(L"Damage", (short)DyeColor::RED->getBlockData());
    }

    return tag;
}
