#include "game/entity/mob/EntityHealthFix.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"

static const std::wstring sEntityIDs[] = {
    L"ArmorStand",  L"Bat",           L"Blaze",     L"CaveSpider",  L"Chicken",   L"Cow",    L"Creeper",
    L"EnderDragon", L"Enderman",      L"Endermite", L"EntityHorse", L"Ghast",     L"Giant",  L"Guardian",
    L"LavaSlime",   L"MushroomCow",   L"Ozelot",    L"Pig",         L"PigZombie", L"Rabbit", L"Sheep",
    L"Shulker",     L"Silverfish",    L"Skeleton",  L"Slime",       L"SnowMan",   L"Spider", L"Squid",
    L"Villager",    L"VillagerGolem", L"Witch",     L"WitherBoss",  L"Wolf",      L"Zombie"};

EntityHealthFix::~EntityHealthFix()
{
}

int EntityHealthFix::getVersion()
{
    return 109;
}

CompoundTag* EntityHealthFix::fix(CompoundTag* tag)
{
    for (unsigned int i = 0; i < sEntityIDs->length(); ++i)
    {
        if (CheckID(tag, sEntityIDs[i]))
        {
            float health;

            if (tag->contains(L"HealF", Tag::TAG_ANY_NUMERIC))
            {
                health = tag->getFloat(L"HealF");
                tag->remove(L"HealF");
            }
            else
            {
                if (tag->contains(L"Health", Tag::TAG_ANY_NUMERIC))
                {
                    health = tag->getFloat(L"Health");
                }
                else
                {
                    break;
                }
            }

            tag->putFloat(L"Health", health);
        }
    }

    return tag;
}
