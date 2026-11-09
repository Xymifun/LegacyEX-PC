#include "game/entity/mob/EntityZombieSplitFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"

EntityZombieSplitFix::~EntityZombieSplitFix()
{
}

int EntityZombieSplitFix::getVersion()
{
    return 702;
}

CompoundTag* EntityZombieSplitFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"Zombie"))
    {
        int zombieType = tag->getInt(L"ZombieType");

        switch (zombieType)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            tag->putString(L"id", L"ZombieVillager");
            tag->putInt(L"Profession", zombieType - 1);
            break;
        case 6:
            tag->putString(L"id", L"Husk");
            break;
        }

        delete tag->get(L"ZombieType");

        tag->remove(L"ZombieType");
    }

    return tag;
}
