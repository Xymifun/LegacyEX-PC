#include "game/item/ItemShulkerBoxColorFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

static const std::wstring SHULKER_BOX_IDS[16] = {
    L"minecraft:white_shulker_box",      L"minecraft:orange_shulker_box", L"minecraft:magenta_shulker_box",
    L"minecraft:light_blue_shulker_box", L"minecraft:yellow_shulker_box", L"minecraft:lime_shulker_box",
    L"minecraft:pink_shulker_box",       L"minecraft:gray_shulker_box",   L"minecraft:silver_shulker_box",
    L"minecraft:cyan_shulker_box",       L"minecraft:purple_shulker_box", L"minecraft:blue_shulker_box",
    L"minecraft:brown_shulker_box",      L"minecraft:green_shulker_box",  L"minecraft:red_shulker_box",
    L"minecraft:black_shulker_box"};

ItemShulkerBoxColorFix::~ItemShulkerBoxColorFix()
{
}

int ItemShulkerBoxColorFix::getVersion()
{
    return 813;
}

CompoundTag* ItemShulkerBoxColorFix::fix(CompoundTag* tag)
{
    if (tag->getString(L"id") == L"minecraft:shulker_box")
    {
        if (tag->contains(L"tag", Tag::TAG_COMPOUND))
        {
            CompoundTag* itemTag = tag->getCompound(L"tag");

            if (itemTag->contains(L"BlockEntityTag", Tag::TAG_COMPOUND))
            {
                CompoundTag* blockEntityTag = itemTag->getCompound(L"BlockEntityTag");

                if (blockEntityTag->getList(L"Items", Tag::TAG_COMPOUND)->isEmpty())
                {
                    blockEntityTag->remove(L"Items");
                }

                int color = blockEntityTag->getInt(L"Color");
                blockEntityTag->remove(L"Color");

                if (blockEntityTag->isEmpty())
                {
                    itemTag->remove(L"BlockEntityTag");
                }

                if (itemTag->isEmpty())
                {
                    tag->remove(L"tag");
                }

                tag->putString(L"id", SHULKER_BOX_IDS[color % 16]);
            }
        }
    }

    return tag;
}
