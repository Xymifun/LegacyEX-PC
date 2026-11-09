#include "game/entity/mob/EntityArmorStandSilentFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

EntityArmorStandSilentFix::~EntityArmorStandSilentFix()
{
}

int EntityArmorStandSilentFix::getVersion()
{
    return 147;
}

CompoundTag* EntityArmorStandSilentFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"ArmorStand"))
    {
        if (tag->getBoolean(L"Silent") && !tag->getBoolean(L"Marker"))
        {
            tag->remove(L"Silent");
        }
    }

    return tag;
}
