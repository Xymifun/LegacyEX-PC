#include "game/item/ItemSpawnEggFix.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"

ItemSpawnEggFix::ItemSpawnEggFix()
{
    mEntityNames[1] = L"Item";
    mEntityNames[2] = L"XPOrb";
    mEntityNames[7] = L"ThrownEgg";
    mEntityNames[8] = L"LeashKnot";
    mEntityNames[9] = L"Painting";
    mEntityNames[10] = L"Arrow";
    mEntityNames[11] = L"Snowball";
    mEntityNames[12] = L"Fireball";
    mEntityNames[13] = L"SmallFireball";
    mEntityNames[14] = L"ThrownEnderpearl";
    mEntityNames[15] = L"EyeOfEnderSignal";
    mEntityNames[16] = L"ThrownPotion";
    mEntityNames[17] = L"ThrownExpBottle";
    mEntityNames[18] = L"ItemFrame";
    mEntityNames[19] = L"WitherSkull";
    mEntityNames[20] = L"PrimedTnt";
    mEntityNames[21] = L"FallingSand";
    mEntityNames[22] = L"FireworksRocketEntity";
    mEntityNames[23] = L"TippedArrow";
    mEntityNames[24] = L"SpectralArrow";
    mEntityNames[25] = L"ShulkerBullet";
    mEntityNames[26] = L"DragonFireball";
    mEntityNames[30] = L"ArmorStand";
    mEntityNames[41] = L"Boat";
    mEntityNames[42] = L"MinecartRideable";
    mEntityNames[43] = L"MinecartChest";
    mEntityNames[44] = L"MinecartFurnace";
    mEntityNames[45] = L"MinecartTNT";
    mEntityNames[46] = L"MinecartHopper";
    mEntityNames[47] = L"MinecartSpawner";
    mEntityNames[40] = L"MinecartCommandBlock";
    mEntityNames[48] = L"Mob";
    mEntityNames[49] = L"Monster";
    mEntityNames[50] = L"Creeper";
    mEntityNames[51] = L"Skeleton";
    mEntityNames[52] = L"Spider";
    mEntityNames[53] = L"Giant";
    mEntityNames[54] = L"Zombie";
    mEntityNames[55] = L"Slime";
    mEntityNames[56] = L"Ghast";
    mEntityNames[57] = L"PigZombie";
    mEntityNames[58] = L"Enderman";
    mEntityNames[59] = L"CaveSpider";
    mEntityNames[60] = L"Silverfish";
    mEntityNames[61] = L"Blaze";
    mEntityNames[62] = L"LavaSlime";
    mEntityNames[63] = L"EnderDragon";
    mEntityNames[64] = L"WitherBoss";
    mEntityNames[65] = L"Bat";
    mEntityNames[66] = L"Witch";
    mEntityNames[67] = L"Endermite";
    mEntityNames[68] = L"Guardian";
    mEntityNames[69] = L"Shulker";
    mEntityNames[90] = L"Pig";
    mEntityNames[91] = L"Sheep";
    mEntityNames[92] = L"Cow";
    mEntityNames[93] = L"Chicken";
    mEntityNames[94] = L"Squid";
    mEntityNames[95] = L"Wolf";
    mEntityNames[96] = L"MushroomCow";
    mEntityNames[97] = L"SnowMan";
    mEntityNames[98] = L"Ozelot";
    mEntityNames[99] = L"VillagerGolem";
    mEntityNames[100] = L"EntityHorse";
    mEntityNames[101] = L"Rabbit";
    mEntityNames[120] = L"Villager";
    mEntityNames[200] = L"EnderCrystal";
    mEntityNames[102] = L"PolarBear";
    mEntityNames[0x2064] = L"Donkey";
    mEntityNames[0x3064] = L"Mule";
}

int ItemSpawnEggFix::getVersion()
{
    return 105;
}

CompoundTag* ItemSpawnEggFix::fix(CompoundTag* item)
{
    if (CheckID(item, L"minecraft:spawn_egg"))
    {
        CompoundTag* itemTag = item->getCompound(L"tag");
        CompoundTag* entityTag = itemTag->getCompound(L"EntityTag");
        short damage = item->getShort(L"Damage");

        if (!entityTag->contains(L"id", Tag::TAG_STRING))
        {
            boost::unordered_map<int, std::wstring>::iterator entity = mEntityNames.find(damage);

            if (entity == mEntityNames.end())
            {
                entity = mEntityNames.find(damage & 0xFF);
            }

            if (entity != mEntityNames.end())
            {
                entityTag->putString(L"id", entity->second);
                itemTag->put(L"EntityTag", entityTag);
                item->put(L"tag", itemTag);
            }
        }

        if (damage != 0)
        {
            item->putShort(L"Damage", 0);
        }
    }

    return item;
}
