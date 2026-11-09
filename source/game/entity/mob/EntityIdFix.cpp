#include "game/entity/mob/EntityIdFix.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"

boost::unordered_map<std::wstring, std::wstring> makeIdMap()
{
    boost::unordered_map<std::wstring, std::wstring> idMap;
    idMap[L"AreaEffectCloud"] = L"minecraft:area_effect_cloud";
    idMap[L"ArmorStand"] = L"minecraft:armor_stand";
    idMap[L"Arrow"] = L"minecraft:arrow";
    idMap[L"Bat"] = L"minecraft:bat";
    idMap[L"Blaze"] = L"minecraft:blaze";
    idMap[L"Boat"] = L"minecraft:boat";
    idMap[L"CaveSpider"] = L"minecraft:cave_spider";
    idMap[L"Chicken"] = L"minecraft:chicken";
    idMap[L"Cow"] = L"minecraft:cow";
    idMap[L"Creeper"] = L"minecraft:creeper";
    idMap[L"Donkey"] = L"minecraft:donkey";
    idMap[L"DragonFireball"] = L"minecraft:dragon_fireball";
    idMap[L"ElderGuardian"] = L"minecraft:elder_guardian";
    idMap[L"EnderCrystal"] = L"minecraft:ender_crystal";
    idMap[L"EnderDragon"] = L"minecraft:ender_dragon";
    idMap[L"Enderman"] = L"minecraft:enderman";
    idMap[L"Endermite"] = L"minecraft:endermite";
    idMap[L"EyeOfEnderSignal"] = L"minecraft:eye_of_ender_signal";
    idMap[L"FallingSand"] = L"minecraft:falling_block";
    idMap[L"Fireball"] = L"minecraft:fireball";
    idMap[L"FireworksRocketEntity"] = L"minecraft:fireworks_rocket";
    idMap[L"Ghast"] = L"minecraft:ghast";
    idMap[L"Giant"] = L"minecraft:giant";
    idMap[L"Guardian"] = L"minecraft:guardian";
    idMap[L"Horse"] = L"minecraft:horse";
    idMap[L"Husk"] = L"minecraft:husk";
    idMap[L"Item"] = L"minecraft:item";
    idMap[L"ItemFrame"] = L"minecraft:item_frame";
    idMap[L"LavaSlime"] = L"minecraft:magma_cube";
    idMap[L"LeashKnot"] = L"minecraft:leash_knot";
    idMap[L"MinecartChest"] = L"minecraft:chest_minecart";
    idMap[L"MinecartCommandBlock"] = L"minecraft:commandblock_minecart";
    idMap[L"MinecartFurnace"] = L"minecraft:furnace_minecart";
    idMap[L"MinecartHopper"] = L"minecraft:hopper_minecart";
    idMap[L"MinecartRideable"] = L"minecraft:minecart";
    idMap[L"MinecartSpawner"] = L"minecraft:spawner_minecart";
    idMap[L"MinecartTNT"] = L"minecraft:tnt_minecart";
    idMap[L"Mule"] = L"minecraft:mule";
    idMap[L"MushroomCow"] = L"minecraft:mooshroom";
    idMap[L"Ozelot"] = L"minecraft:ocelot";
    idMap[L"Painting"] = L"minecraft:painting";
    idMap[L"Pig"] = L"minecraft:pig";
    idMap[L"PigZombie"] = L"minecraft:zombie_pigman";
    idMap[L"PolarBear"] = L"minecraft:polar_bear";
    idMap[L"PrimedTnt"] = L"minecraft:tnt";
    idMap[L"Rabbit"] = L"minecraft:rabbit";
    idMap[L"Sheep"] = L"minecraft:sheep";
    idMap[L"Shulker"] = L"minecraft:shulker";
    idMap[L"ShulkerBullet"] = L"minecraft:shulker_bullet";
    idMap[L"Silverfish"] = L"minecraft:silverfish";
    idMap[L"Skeleton"] = L"minecraft:skeleton";
    idMap[L"SkeletonHorse"] = L"minecraft:skeleton_horse";
    idMap[L"Slime"] = L"minecraft:slime";
    idMap[L"SmallFireball"] = L"minecraft:small_fireball";
    idMap[L"SnowMan"] = L"minecraft:snowman";
    idMap[L"Snowball"] = L"minecraft:snowball";
    idMap[L"SpectralArrow"] = L"minecraft:spectral_arrow";
    idMap[L"Spider"] = L"minecraft:spider";
    idMap[L"Squid"] = L"minecraft:squid";
    idMap[L"Stray"] = L"minecraft:stray";
    idMap[L"ThrownEgg"] = L"minecraft:egg";
    idMap[L"ThrownEnderpearl"] = L"minecraft:ender_pearl";
    idMap[L"ThrownExpBottle"] = L"minecraft:xp_bottle";
    idMap[L"ThrownPotion"] = L"minecraft:potion";
    idMap[L"Villager"] = L"minecraft:villager";
    idMap[L"VillagerGolem"] = L"minecraft:villager_golem";
    idMap[L"Witch"] = L"minecraft:witch";
    idMap[L"WitherBoss"] = L"minecraft:wither";
    idMap[L"WitherSkeleton"] = L"minecraft:wither_skeleton";
    idMap[L"WitherSkull"] = L"minecraft:wither_skull";
    idMap[L"Wolf"] = L"minecraft:wolf";
    idMap[L"XPOrb"] = L"minecraft:xp_orb";
    idMap[L"Zombie"] = L"minecraft:zombie";
    idMap[L"ZombieHorse"] = L"minecraft:zombie_horse";
    idMap[L"ZombieVillager"] = L"minecraft:zombie_villager";
    return idMap;
}

static boost::unordered_map<std::wstring, std::wstring> sEntityIdMap = makeIdMap();

EntityIdFix::~EntityIdFix()
{
}

int EntityIdFix::getVersion()
{
    return 704;
}

CompoundTag* EntityIdFix::fix(CompoundTag* tag)
{
    const boost::unordered_map<std::wstring, std::wstring>& idMap = sEntityIdMap;
    boost::unordered_map<std::wstring, std::wstring>::const_iterator entry = idMap.find(tag->getString(L"id"));

    if (entry != idMap.end())
    {
        tag->putString(L"id", entry->second);
    }

    return tag;
}
