#include "game/item/ItemWaterPotionFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

ItemWaterPotionFix::~ItemWaterPotionFix()
{
}

int ItemWaterPotionFix::getVersion()
{
    return 806;
}

CompoundTag* ItemWaterPotionFix::fix(CompoundTag* tag)
{
    const std::wstring id = tag->getString(L"id");

    if (id == L"minecraft:potion" || id == L"minecraft:splash_potion" || id == L"minecraft:lingering_potion"
        || id == L"minecraft:tipped_arrow")
    {
        CompoundTag* itemTag = tag->getCompound(L"tag");

        if (!itemTag->contains(L"Potion", Tag::TAG_STRING))
        {
            itemTag->putString(L"Potion", L"minecraft:water");
        }

        if (!tag->contains(L"tag", Tag::TAG_COMPOUND))
        {
            tag->put(L"tag", itemTag);
        }
    }

    return tag;
}
