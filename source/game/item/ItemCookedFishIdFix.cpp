#include "game/item/ItemCookedFishIdFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/util/ResourceLocation.hpp"


static const ResourceLocation COOKED_FISH_ID(L"cooked_fish");

ItemCookedFishIdFix::~ItemCookedFishIdFix()
{
}

int ItemCookedFishIdFix::getVersion()
{
    return 502;
}

CompoundTag* ItemCookedFishIdFix::fix(CompoundTag* tag)
{
    if (tag->contains(L"id", Tag::TAG_STRING))
    {
        ResourceLocation id(tag->getString(L"id"));

        if (COOKED_FISH_ID.equals(&id))
        {
            tag->putString(L"id", L"minecraft:cooked_fish");
        }
    }

    return tag;
}
