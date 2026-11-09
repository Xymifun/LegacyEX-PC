#include "game/core/DefaultPresets.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/SuperflatConfig.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"

void DefaultPresets::makePreset(SuperflatConfig* config, Preset preset)
{
    FlatGeneratorInfo info(config);
    switch (preset)
    {
    case CLASSIC:
    {
        info.setDisplayname(IDS_PRESET_CLASSIC_FLAT);
        info.setBiome(Biome::PLAINS->id);
        info.setVillage(true);
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(2, Blocks::dirt));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case TUNNELERS_DREAM:
    {
        info.setDisplayname(IDS_PRESET_TUNNELERS_DREAM);
        info.setBiome(Biome::EXTREME_HILLS->id);
        info.setDungeons(true);
        info.setDecorations(true);
        info.setStronghold(true);
        info.setMineshaft(true);
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt));
        info.addLayer(FlatLayerInfo(100, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case WATER_WORLD:
    {
        info.setDisplayname(IDS_PRESET_WATER_WORLD);
        info.setBiome(Biome::DEEP_OCEAN->id);
        info.addLayer(FlatLayerInfo(90, Blocks::water));
        info.addLayer(FlatLayerInfo(5, Blocks::sand));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt));
        info.addLayer(FlatLayerInfo(5, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case OVERWORLD:
    {
        info.setDisplayname(IDS_PRESET_OVERWORLD);
        info.setBiome(Biome::PLAINS->id);
        info.setVillage(true);
        info.setDecorations(true);
        info.setStronghold(true);
        info.setMineshaft(true);
        info.setDungeons(true);
        info.setLakes(true);
        info.setLavaLakes(true);
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(3, Blocks::dirt));
        info.addLayer(FlatLayerInfo(59, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case SNOWY_KINGDOM:
    {
        info.setDisplayname(IDS_PRESET_SNOWY_KINGDOM);
        info.setBiome(Biome::ICE_PLAINS->id);
        info.setVillage(true);
        info.setBiomeSpecific(true);
        info.addLayer(FlatLayerInfo(1, Blocks::snow_layer));
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(3, Blocks::dirt));
        info.addLayer(FlatLayerInfo(59, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case BOTTOMLESS_PIT:
    {
        info.setDisplayname(IDS_PRESET_BOTTOMLESS_PIT);
        info.setBiome(Biome::PLAINS->id);
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(3, Blocks::dirt));
        info.addLayer(FlatLayerInfo(2, Blocks::stonebrick));
    }
    break;
    case DESERT:
    {
        info.setDisplayname(IDS_PRESET_DESERT);
        info.setBiome(Biome::DESERT->id);
        info.setVillage(true);
        info.setBiomeSpecific(true);
        info.setDecorations(true);
        info.setStronghold(true);
        info.setMineshaft(true);
        info.addLayer(FlatLayerInfo(8, Blocks::sand));
        info.addLayer(FlatLayerInfo(52, Blocks::sandstone));
        info.addLayer(FlatLayerInfo(3, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case REDSTONE_READY:
    {
        info.setDisplayname(IDS_PRESET_REDSTONE_READY);
        info.setBiome(Biome::DESERT->id);
        info.addLayer(FlatLayerInfo(52, Blocks::sandstone));
        info.addLayer(FlatLayerInfo(3, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case THE_VOID:
        break;
    case DEBUG_JUNGLE:
    {
        info.setDisplayname(L"DebugPreset::Jungle");
        info.setBiome(Biome::JUNGLE->id);
        info.setBiomeSpecific(true);
        info.setDecorations(true);
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(4, Blocks::dirt));
        info.addLayer(FlatLayerInfo(50, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case DEBUG_WOODLAND_MANSION:
    {
        info.setDisplayname(L"DebugPreset::WoodlandMansion");
        info.setBiome(Biome::ROOFED_FOREST->id);
        info.setDecorations(true);
        info.setWoodlandMansion(32, 15);
        info.addLayer(FlatLayerInfo(1, Blocks::grass));
        info.addLayer(FlatLayerInfo(4, Blocks::dirt));
        info.addLayer(FlatLayerInfo(50, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case DEBUG_OCEAN_MONUMENT:
    {
        info.setDisplayname(L"DebugPreset::OceanMonument");
        info.setBiome(Biome::DEEP_OCEAN->id);
        info.setOceanMonument(32, 15);
        info.addLayer(FlatLayerInfo(90, Blocks::water));
        info.addLayer(FlatLayerInfo(5, Blocks::sand));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt));
        info.addLayer(FlatLayerInfo(5, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    case DEBUG_WARM_OCEAN:
    {
        info.setDisplayname(L"DebugPreset:WarmOcean");
        info.setBiome(Biome::WARM_OCEAN->id);
        info.setDecorations(true);
        info.setShipwrecks(true);
        info.addLayer(FlatLayerInfo(20, Blocks::water));
        info.addLayer(FlatLayerInfo(5, Blocks::sand));
        info.addLayer(FlatLayerInfo(5, Blocks::dirt));
        info.addLayer(FlatLayerInfo(40, Blocks::stone));
        info.addLayer(FlatLayerInfo(1, Blocks::bedrock));
    }
    break;
    }
}
