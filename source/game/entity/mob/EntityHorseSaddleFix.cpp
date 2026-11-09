#include "game/entity/mob/EntityHorseSaddleFix.hpp"

#include "game/data/CompoundTag.hpp"

EntityHorseSaddleFix::~EntityHorseSaddleFix()
{
}

int EntityHorseSaddleFix::getVersion()
{
    return 110;
}

CompoundTag* EntityHorseSaddleFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"EntityHorse"))
    {
        if (!tag->contains(L"SaddleItem", Tag::TAG_COMPOUND) && tag->getBoolean(L"Saddle"))
        {
            CompoundTag* saddleItem = new CompoundTag();
            saddleItem->putString(L"id", L"minecraft:saddle");
            saddleItem->putByte(L"Count", 1);
            saddleItem->putShort(L"Damage", 0);
            tag->put(L"SaddleItem", saddleItem);
            tag->remove(L"Saddle");
        }
    }

    return tag;
}
