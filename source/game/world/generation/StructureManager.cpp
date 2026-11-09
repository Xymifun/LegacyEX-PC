#include "game/world/generation/StructureManager.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StringUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static const wchar_t* STRUCTURE_TEMPLATE_NAMES[eMinecraftStructureTemplate_Count] = {
    L"igloo/igloo_top",
    L"igloo/igloo_middle",
    L"igloo/igloo_bottom",
    L"fossils/fossil_spine_01",
    L"fossils/fossil_spine_02",
    L"fossils/fossil_spine_03",
    L"fossils/fossil_spine_04",
    L"fossils/fossil_spine_01_coal",
    L"fossils/fossil_spine_02_coal",
    L"fossils/fossil_spine_03_coal",
    L"fossils/fossil_spine_04_coal",
    L"fossils/fossil_skull_01",
    L"fossils/fossil_skull_02",
    L"fossils/fossil_skull_03",
    L"fossils/fossil_skull_04",
    L"fossils/fossil_skull_01_coal",
    L"fossils/fossil_skull_02_coal",
    L"fossils/fossil_skull_03_coal",
    L"fossils/fossil_skull_04_coal",
    L"endcity/base_floor",
    L"endcity/base_roof",
    L"endcity/bridge_end",
    L"endcity/bridge_gentle_stairs",
    L"endcity/bridge_piece",
    L"endcity/bridge_steep_stairs",
    L"endcity/fat_tower_base",
    L"endcity/fat_tower_middle",
    L"endcity/fat_tower_top",
    L"endcity/second_floor",
    L"endcity/second_floor_2",
    L"endcity/second_roof",
    L"endcity/ship",
    L"endcity/third_floor",
    L"endcity/third_floor_b",
    L"endcity/third_floor_c",
    L"endcity/third_roof",
    L"endcity/tower_base",
    L"endcity/tower_floor",
    L"endcity/tower_piece",
    L"endcity/tower_top",
    L"mansion/carpet_south",
    L"mansion/carpet_south_2",
    L"mansion/carpet_west",
    L"mansion/carpet_west_2",
    L"mansion/carpet_north",
    L"mansion/carpet_east",
    L"mansion/corridor_floor",
    L"mansion/corridor_floor_2",
    L"mansion/indoors_wall",
    L"mansion/indoors_wall_2",
    L"mansion/indoors_door",
    L"mansion/indoors_door_2",
    L"mansion/roof",
    L"mansion/roof_front",
    L"mansion/small_wall",
    L"mansion/small_wall_corner",
    L"mansion/roof_corner",
    L"mansion/roof_inner_corner",
    L"mansion/entrance",
    L"mansion/wall_corner",
    L"mansion/wall_flat",
    L"mansion/wall_window",
    L"mansion/1x1_a1",
    L"mansion/1x1_a2",
    L"mansion/1x1_a3",
    L"mansion/1x1_a4",
    L"mansion/1x1_a5",
    L"mansion/1x1_as1",
    L"mansion/1x1_as2",
    L"mansion/1x1_as3",
    L"mansion/1x1_as4",
    L"mansion/1x1_b1",
    L"mansion/1x1_b2",
    L"mansion/1x1_b3",
    L"mansion/1x1_b4",
    L"mansion/1x1_b5",
    L"mansion/1x2_a1",
    L"mansion/1x2_a2",
    L"mansion/1x2_a3",
    L"mansion/1x2_a4",
    L"mansion/1x2_a5",
    L"mansion/1x2_a6",
    L"mansion/1x2_a7",
    L"mansion/1x2_a8",
    L"mansion/1x2_a9",
    L"mansion/1x2_b1",
    L"mansion/1x2_b2",
    L"mansion/1x2_b3",
    L"mansion/1x2_b4",
    L"mansion/1x2_b5",
    L"mansion/1x2_c_stairs",
    L"mansion/1x2_c1",
    L"mansion/1x2_c2",
    L"mansion/1x2_c3",
    L"mansion/1x2_c4",
    L"mansion/1x2_d_stairs",
    L"mansion/1x2_d1",
    L"mansion/1x2_d2",
    L"mansion/1x2_d3",
    L"mansion/1x2_d4",
    L"mansion/1x2_d5",
    L"mansion/1x2_s1",
    L"mansion/1x2_s2",
    L"mansion/1x2_se1",
    L"mansion/2x2_a1",
    L"mansion/2x2_a2",
    L"mansion/2x2_a3",
    L"mansion/2x2_a4",
    L"mansion/2x2_b1",
    L"mansion/2x2_b2",
    L"mansion/2x2_b3",
    L"mansion/2x2_b4",
    L"mansion/2x2_b5",
    L"mansion/2x2_s1",
    L"shipwreck/swwithmast",
    L"shipwreck/swupsidedownfull",
    L"shipwreck/swupsidedownfronthalf",
    L"shipwreck/swupsidedownbackhalf",
    L"shipwreck/swsidewaysfull",
    L"shipwreck/swsidewaysfronthalf",
    L"shipwreck/swsidewaysbackhalf",
    L"shipwreck/swrightsideupfull",
    L"shipwreck/swrightsideupfronthalf",
    L"shipwreck/swrightsideupbackhalf",
    L"shipwreck/swwithmastdegraded",
    L"shipwreck/swupsidedownfulldegraded",
    L"shipwreck/swupsidedownfronthalfdegraded",
    L"shipwreck/swupsidedownbackhalfdegraded",
    L"shipwreck/swsidewaysfulldegraded",
    L"shipwreck/swsidewaysfronthalfdegraded",
    L"shipwreck/swsidewaysbackhalfdegraded",
    L"shipwreck/swrightsideupfulldegraded",
    L"shipwreck/swrightsideupfronthalfdegraded",
    L"shipwreck/swrightsideupbackhalfdegraded",
    L"ruin/big_ruin1_brick",
    L"ruin/big_ruin2_brick",
    L"ruin/big_ruin3_brick",
    L"ruin/big_ruin8_brick",
    L"ruin/big_ruin1_mossy",
    L"ruin/big_ruin2_mossy",
    L"ruin/big_ruin3_mossy",
    L"ruin/big_ruin8_mossy",
    L"ruin/big_ruin1_cracked",
    L"ruin/big_ruin2_cracked",
    L"ruin/big_ruin3_cracked",
    L"ruin/big_ruin8_cracked",
    L"ruin/big_ruin_warm4",
    L"ruin/big_ruin_warm5",
    L"ruin/big_ruin_warm6",
    L"ruin/big_ruin_warm7",
    L"ruin/ruin_warm1",
    L"ruin/ruin_warm2",
    L"ruin/ruin_warm3",
    L"ruin/ruin1_brick",
    L"ruin/ruin1_cracked",
    L"ruin/ruin1_mossy",
    L"ruin/ruin2_brick",
    L"ruin/ruin2_cracked",
    L"ruin/ruin2_mossy",
    L"ruin/ruin3_brick",
    L"ruin/ruin3_cracked",
    L"ruin/ruin3_mossy",
    L"ruin/ruin4_brick",
    L"ruin/ruin4_cracked",
    L"ruin/ruin4_mossy",
    L"ruin/ruin5_brick",
    L"ruin/ruin5_cracked",
    L"ruin/ruin5_mossy",
    L"ruin/ruin6_brick",
    L"ruin/ruin6_cracked",
    L"ruin/ruin6_mossy",
    L"ruin/ruin7_brick",
    L"ruin/ruin7_cracked",
    L"ruin/ruin7_mossy",
    L"ruin/ruin8_brick",
    L"ruin/ruin8_cracked",
    L"ruin/ruin8_mossy",
    L"coralcrust/crust1",
    L"coralcrust/crust2",
    L"coralcrust/crust3",
    L"coralcrust/crust4",
    L"coralcrust/crust5",
    L"coralcrust/outcropping1",
    L"coralcrust/outcropping2",
    L"coralcrust/outcropping3",
    L"coralcrust/outcropping4",
    L"coralcrust/outcropping5",
    L"coralcrust/outcropping6",
};

StructureManager::StructureManager(DataFixerUpper* fixer) : templates(11), fixerUpper(fixer)
{
}

StructureManager::~StructureManager()
{
    for (boost::unordered_map<int, const StructureTemplate*>::iterator it = templates.begin(); it != templates.end();
         ++it)
        delete it->second;
}

void StructureManager::staticCtor()
{
}

std::wstring StructureManager::fjGetStructureTemplatePath(eMinecraftStructureTemplate type) const
{
    std::wstring name = STRUCTURE_TEMPLATE_NAMES[type];
    return L"Structures\\" + replaceAll(name, L"/", L"\\");
}

void StructureManager::readStructure(eMinecraftStructureTemplate type, ByteArrayInputStream* input)
{
    CompoundTag* tag = NbtIo::readCompressed(input);
    bool hasDataVersion = tag->contains(SharedConstants::DATA_VERSION_TAG, Tag::TAG_ANY_NUMERIC);
    if (!hasDataVersion)
        tag->putInt(SharedConstants::DATA_VERSION_TAG, 500);

    StructureTemplate* structureTemplate = new StructureTemplate();
    CompoundTag* fixedTag = fixerUpper->updateData(DataFixTypes::eDataFixTypes_Structure, tag);
    structureTemplate->load(fixedTag);
    delete tag;

    boost::unordered_map<int, const StructureTemplate*>::iterator it = templates.find(type);
    if (it != templates.end())
        delete it->second;
    templates[type] = structureTemplate;
}

bool StructureManager::load(eMinecraftStructureTemplate type)
{
    std::wstring path = fjGetStructureTemplatePath(type);
    if (gApp.hasArchiveFile(path, false))
    {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(path, false);
        if (data.length != 0 && data.data != NULL)
        {
            ByteArrayInputStream input(data);
            readStructure(type, &input);
            input.reset();
            delete[] data.data;
            return true;
        }
    }
    return false;
}

const StructureTemplate* StructureManager::get(eMinecraftStructureTemplate type)
{
    if (templates.find(type) != templates.end())
    {
        return templates.find(type)->second;
    }
    else
    {
        load(type);
        if (templates.find(type) != templates.end())
        {
            return templates.find(type)->second;
        }
    }
    return new StructureTemplate();
}

const StructureTemplate* StructureManager::getOrCreate(eMinecraftStructureTemplate type)
{
    const StructureTemplate* structureTemplate = get(type);
    if (structureTemplate == NULL)
    {
        structureTemplate = new StructureTemplate();
        templates[type] = structureTemplate;
    }
    return structureTemplate;
}
