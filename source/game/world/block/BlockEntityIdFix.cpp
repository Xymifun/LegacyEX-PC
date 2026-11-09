#include "game/world/block/BlockEntityIdFix.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"

boost::unordered_map<std::wstring, std::wstring> BlockEntityIdFix::ID_MAP;

BlockEntityIdFix::~BlockEntityIdFix()
{
}

void BlockEntityIdFix::staticCtor()
{
    ID_MAP[L"Airportal"] = L"minecraft:end_portal";
    ID_MAP[L"Banner"] = L"minecraft:banner";
    ID_MAP[L"Beacon"] = L"minecraft:beacon";
    ID_MAP[L"Cauldron"] = L"minecraft:brewing_stand";
    ID_MAP[L"Chest"] = L"minecraft:chest";
    ID_MAP[L"Comparator"] = L"minecraft:comparator";
    ID_MAP[L"Control"] = L"minecraft:command_block";
    ID_MAP[L"DLDetector"] = L"minecraft:daylight_detector";
    ID_MAP[L"Dropper"] = L"minecraft:dropper";
    ID_MAP[L"EnchantTable"] = L"minecraft:enchanting_table";
    ID_MAP[L"EndGateway"] = L"minecraft:end_gateway";
    ID_MAP[L"EnderChest"] = L"minecraft:ender_chest";
    ID_MAP[L"FlowerPot"] = L"minecraft:flower_pot";
    ID_MAP[L"Furnace"] = L"minecraft:furnace";
    ID_MAP[L"Hopper"] = L"minecraft:hopper";
    ID_MAP[L"MobSpawner"] = L"minecraft:mob_spawner";
    ID_MAP[L"Music"] = L"minecraft:noteblock";
    ID_MAP[L"Piston"] = L"minecraft:piston";
    ID_MAP[L"RecordPlayer"] = L"minecraft:jukebox";
    ID_MAP[L"Sign"] = L"minecraft:sign";
    ID_MAP[L"Skull"] = L"minecraft:skull";
    ID_MAP[L"Structure"] = L"minecraft:structure_block";
    ID_MAP[L"Trap"] = L"minecraft:dispenser";
    ID_MAP[L"shulker_box"] = L"minecraft:shulker_box";
}

int BlockEntityIdFix::getVersion()
{
    return 704;
}

CompoundTag* BlockEntityIdFix::fix(CompoundTag* tag)
{
    boost::unordered_map<std::wstring, std::wstring>::iterator entry = ID_MAP.find(tag->getString(L"id"));

    if (entry != ID_MAP.end())
    {
        tag->putString(L"id", entry->second);
    }

    return tag;
}
