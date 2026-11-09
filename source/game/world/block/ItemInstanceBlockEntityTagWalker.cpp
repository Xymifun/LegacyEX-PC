#include "game/world/block/ItemInstanceBlockEntityTagWalker.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/util/ResourceLocation.hpp"


std::wstring fjSafeGet(const boost::unordered_map<std::wstring, std::wstring>& map, const std::wstring& key);

std::wstring fjSafeGet(const boost::unordered_map<std::wstring, std::wstring>& map, const std::wstring& key)
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator it = map.find(key);
    if (it != map.end())
    {
        return it->second;
    }

    return std::wstring();
}

static boost::unordered_map<std::wstring, std::wstring> sOldBlockEntityIds;
static boost::unordered_map<std::wstring, std::wstring> sNewBlockEntityIds;

ItemInstanceBlockEntityTagWalker::~ItemInstanceBlockEntityTagWalker()
{
}

void ItemInstanceBlockEntityTagWalker::staticCtor()
{
    sOldBlockEntityIds[L"minecraft:furnace"] = L"Furnace";
    sOldBlockEntityIds[L"minecraft:lit_furnace"] = L"Furnace";
    sOldBlockEntityIds[L"minecraft:chest"] = L"Chest";
    sOldBlockEntityIds[L"minecraft:trapped_chest"] = L"Chest";
    sOldBlockEntityIds[L"minecraft:ender_chest"] = L"EnderChest";
    sOldBlockEntityIds[L"minecraft:jukebox"] = L"RecordPlayer";
    sOldBlockEntityIds[L"minecraft:dispenser"] = L"Trap";
    sOldBlockEntityIds[L"minecraft:dropper"] = L"Dropper";
    sOldBlockEntityIds[L"minecraft:sign"] = L"Sign";
    sOldBlockEntityIds[L"minecraft:mob_spawner"] = L"MobSpawner";
    sOldBlockEntityIds[L"minecraft:noteblock"] = L"Music";
    sOldBlockEntityIds[L"minecraft:brewing_stand"] = L"Cauldron";
    sOldBlockEntityIds[L"minecraft:enhanting_table"] = L"EnchantTable";
    sOldBlockEntityIds[L"minecraft:command_block"] = L"CommandBlock";
    sOldBlockEntityIds[L"minecraft:beacon"] = L"Beacon";
    sOldBlockEntityIds[L"minecraft:skull"] = L"Skull";
    sOldBlockEntityIds[L"minecraft:daylight_detector"] = L"DLDetector";
    sOldBlockEntityIds[L"minecraft:hopper"] = L"Hopper";
    sOldBlockEntityIds[L"minecraft:banner"] = L"Banner";
    sOldBlockEntityIds[L"minecraft:flower_pot"] = L"FlowerPot";
    sOldBlockEntityIds[L"minecraft:repeating_command_block"] = L"CommandBlock";
    sOldBlockEntityIds[L"minecraft:chain_command_block"] = L"CommandBlock";
    sOldBlockEntityIds[L"minecraft:standing_sign"] = L"Sign";
    sOldBlockEntityIds[L"minecraft:wall_sign"] = L"Sign";
    sOldBlockEntityIds[L"minecraft:piston_head"] = L"Piston";
    sOldBlockEntityIds[L"minecraft:daylight_detector_inverted"] = L"DLDetector";
    sOldBlockEntityIds[L"minecraft:unpowered_comparator"] = L"Comparator";
    sOldBlockEntityIds[L"minecraft:powered_comparator"] = L"Comparator";
    sOldBlockEntityIds[L"minecraft:wall_banner"] = L"Banner";
    sOldBlockEntityIds[L"minecraft:standing_banner"] = L"Banner";
    sOldBlockEntityIds[L"minecraft:structure_block"] = L"Structure";
    sOldBlockEntityIds[L"minecraft:end_portal"] = L"Airportal";
    sOldBlockEntityIds[L"minecraft:end_gateway"] = L"EndGateway";
    sOldBlockEntityIds[L"minecraft:shield"] = L"Shield";

    sNewBlockEntityIds[L"minecraft:furnace"] = L"minecraft:furnace";
    sNewBlockEntityIds[L"minecraft:lit_furnace"] = L"minecraft:furnace";
    sNewBlockEntityIds[L"minecraft:chest"] = L"minecraft:chest";
    sNewBlockEntityIds[L"minecraft:trapped_chest"] = L"minecraft:chest";
    sNewBlockEntityIds[L"minecraft:ender_chest"] = L"minecraft:enderchest";
    sNewBlockEntityIds[L"minecraft:jukebox"] = L"minecraft:jukebox";
    sNewBlockEntityIds[L"minecraft:dispenser"] = L"minecraft:dispenser";
    sNewBlockEntityIds[L"minecraft:dropper"] = L"minecraft:dropper";
    sNewBlockEntityIds[L"minecraft:sign"] = L"minecraft:sign";
    sNewBlockEntityIds[L"minecraft:mob_spawner"] = L"minecraft:mob_spawner";
    sNewBlockEntityIds[L"minecraft:noteblock"] = L"minecraft:noteblock";
    sNewBlockEntityIds[L"minecraft:brewing_stand"] = L"minecraft:brewing_stand";
    sNewBlockEntityIds[L"minecraft:enhanting_table"] = L"minecraft:enchanting_table";
    sNewBlockEntityIds[L"minecraft:command_block"] = L"minecraft:command_block";
    sNewBlockEntityIds[L"minecraft:beacon"] = L"minecraft:beacon";
    sNewBlockEntityIds[L"minecraft:skull"] = L"minecraft:skull";
    sNewBlockEntityIds[L"minecraft:daylight_detector"] = L"minecraft:daylight_detector";
    sNewBlockEntityIds[L"minecraft:hopper"] = L"minecraft:hopper";
    sNewBlockEntityIds[L"minecraft:banner"] = L"minecraft:banner";
    sNewBlockEntityIds[L"minecraft:flower_pot"] = L"minecraft:flower_pot";
    sNewBlockEntityIds[L"minecraft:repeating_command_block"] = L"minecraft:command_block";
    sNewBlockEntityIds[L"minecraft:chain_command_block"] = L"minecraft:command_block";
    sNewBlockEntityIds[L"minecraft:shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:white_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:orange_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:magenta_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:light_blue_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:yellow_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:lime_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:pink_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:gray_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:silver_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:cyan_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:purple_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:blue_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:brown_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:green_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:red_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:black_shulker_box"] = L"minecraft:shulker_box";
    sNewBlockEntityIds[L"minecraft:bed"] = L"minecraft:bed";
    sNewBlockEntityIds[L"minecraft:standing_sign"] = L"minecraft:sign";
    sNewBlockEntityIds[L"minecraft:wall_sign"] = L"minecraft:sign";
    sNewBlockEntityIds[L"minecraft:piston_head"] = L"minecraft:piston";
    sNewBlockEntityIds[L"minecraft:daylight_detector_inverted"] = L"minecraft:daylight_detector";
    sNewBlockEntityIds[L"minecraft:unpowered_comparator"] = L"minecraft:comparator";
    sNewBlockEntityIds[L"minecraft:powered_comparator"] = L"minecraft:comparator";
    sNewBlockEntityIds[L"minecraft:wall_banner"] = L"minecraft:banner";
    sNewBlockEntityIds[L"minecraft:standing_banner"] = L"minecraft:banner";
    sNewBlockEntityIds[L"minecraft:structure_block"] = L"minecraft:structure_block";
    sNewBlockEntityIds[L"minecraft:end_portal"] = L"minecraft:end_portal";
    sNewBlockEntityIds[L"minecraft:end_gateway"] = L"minecraft:end_gateway";
    sNewBlockEntityIds[L"minecraft:shield"] = L"minecraft:shield";
}

std::wstring ItemInstanceBlockEntityTagWalker::getBlockEntityId(int version, const std::wstring& itemId)
{
    if (version < 515)
    {
        return fjSafeGet(sOldBlockEntityIds, ResourceLocation(itemId).toString());
    }
    else
    {
        return fjSafeGet(sNewBlockEntityIds, ResourceLocation(itemId).toString());
    }
}

CompoundTag* ItemInstanceBlockEntityTagWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (!tag->contains(L"tag", Tag::TAG_COMPOUND))
    {
        return tag;
    }

    CompoundTag* itemTag = tag->getCompound(L"tag");

    if (itemTag->contains(L"BlockEntityTag", Tag::TAG_COMPOUND))
    {
        CompoundTag* blockEntityTag = itemTag->getCompound(L"BlockEntityTag");
        std::wstring itemId = tag->getString(L"id");
        std::wstring blockEntityId = getBlockEntityId(version, itemId);
        bool removeId;

        if (blockEntityId.empty())
        {
            removeId = false;
        }
        else
        {
            removeId = !blockEntityTag->contains(L"id");
            blockEntityTag->putString(L"id", blockEntityId);
        }

        fixer->update(DataFixTypes::eDataFixTypes_BlockEntity, blockEntityTag, version);

        if (removeId)
        {
            blockEntityTag->remove(L"id");
        }
    }

    return tag;
}
