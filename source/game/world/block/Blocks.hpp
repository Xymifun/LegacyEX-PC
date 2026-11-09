#ifndef GAME_WORLD_BLOCK_BLOCKS_HPP
#define GAME_WORLD_BLOCK_BLOCKS_HPP

#include <string>

#include "game/core/Bush.hpp"
#include "game/world/block/BeaconBlock.hpp"
#include "game/world/block/CactusBlock.hpp"
#include "game/world/block/CauldronBlock.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ComparatorBlock.hpp"
#include "game/world/block/DaylightDetectorBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/DynamicLiquidBlock.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/GrassBlock.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/HopperBlock.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/MyceliumBlock.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMagicBlock.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/ReedsBlock.hpp"
#include "game/world/block/RepeaterBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/StainedGlassBlock.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"
#include "game/world/block/StaticLiquidBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TripWireHookBlock.hpp"

class Block;

class Blocks
{
public:
    static void staticInit();

    static Block* air;
    static Block* stone;
    static GrassBlock* grass;
    static Block* dirt;
    static Block* cobblestone;
    static Block* planks;
    static Block* sapling;
    static Block* bedrock;
    static DynamicLiquidBlock* flowing_water;
    static StaticLiquidBlock* water;
    static DynamicLiquidBlock* flowing_lava;
    static StaticLiquidBlock* lava;
    static SandBlock* sand;
    static Block* gravel;
    static Block* gold_ore;
    static Block* iron_ore;
    static Block* coal_ore;
    static Block* log;
    static Block* log2;
    static LeafBlock* leaves;
    static LeafBlock* leaves2;
    static Block* sponge;
    static Block* glass;
    static Block* lapis_ore;
    static Block* lapis_block;
    static Block* dispenser;
    static Block* sandstone;
    static Block* noteblock;
    static Block* bed;
    static Block* golden_rail;
    static Block* detector_rail;
    static PistonBaseBlock* sticky_piston;
    static Block* web;
    static TallGrassBlock* tallgrass;
    static Block* deadbush;
    static PistonBaseBlock* piston;
    static PistonHeadBlock* piston_head;
    static Block* wool;
    static PistonMagicBlock* piston_extension;
    static FlowerBlock* yellow_flower;
    static FlowerBlock* red_flower;
    static Bush* brown_mushroom;
    static Bush* red_mushroom;
    static Block* gold_block;
    static Block* iron_block;
    static HalfSlabBlock* double_stone_slab;
    static HalfSlabBlock* stone_slab;
    static Block* brick_block;
    static Block* tnt;
    static Block* bookshelf;
    static Block* mossy_cobblestone;
    static Block* obsidian;
    static Block* torch;
    static Block* fire;
    static Block* mob_spawner;
    static Block* oak_stairs;
    static ChestBlock* chest;
    static RedStoneWireBlock* redstone_wire;
    static Block* diamond_ore;
    static Block* diamond_block;
    static Block* crafting_table;
    static Block* wheat;
    static Block* farmland;
    static Block* furnace;
    static Block* lit_furnace;
    static Block* standing_sign;
    static Block* wooden_door;
    static Block* spruce_door;
    static Block* birch_door;
    static Block* jungle_door;
    static Block* acacia_door;
    static Block* dark_oak_door;
    static Block* ladder;
    static Block* rail;
    static Block* stone_stairs;
    static Block* wall_sign;
    static Block* lever;
    static Block* stone_pressure_plate;
    static Block* iron_door;
    static Block* wooden_pressure_plate;
    static Block* redstone_ore;
    static Block* lit_redstone_ore;
    static Block* unlit_redstone_torch;
    static Block* redstone_torch;
    static Block* stone_button;
    static Block* snow_layer;
    static Block* ice;
    static Block* snow;
    static CactusBlock* cactus;
    static Block* clay;
    static ReedsBlock* reeds;
    static Block* jukebox;
    static Block* fence;
    static Block* spruce_fence;
    static Block* birch_fence;
    static Block* jungle_fence;
    static Block* dark_oak_fence;
    static Block* acacia_fence;
    static Block* pumpkin;
    static Block* carved_pumpkin;
    static Block* netherrack;
    static Block* soul_sand;
    static Block* glowstone;
    static PortalBlock* portal;
    static Block* lit_pumpkin;
    static Block* cake;
    static RepeaterBlock* unpowered_repeater;
    static RepeaterBlock* powered_repeater;
    static Block* trapdoor;
    static Block* monster_egg;
    static Block* stonebrick;
    static Block* brown_mushroom_block;
    static Block* red_mushroom_block;
    static Block* iron_bars;
    static Block* glass_pane;
    static Block* melon_block;
    static Block* pumpkin_stem;
    static Block* melon_stem;
    static Block* vine;
    static Block* fence_gate;
    static Block* spruce_fence_gate;
    static Block* birch_fence_gate;
    static Block* jungle_fence_gate;
    static Block* dark_oak_fence_gate;
    static Block* acacia_fence_gate;
    static Block* brick_stairs;
    static Block* stone_brick_stairs;
    static MyceliumBlock* mycelium;
    static Block* waterlily;
    static Block* nether_brick;
    static Block* nether_brick_fence;
    static Block* nether_brick_stairs;
    static Block* nether_wart;
    static Block* enchanting_table;
    static Block* brewing_stand;
    static CauldronBlock* cauldron;
    static Block* end_portal;
    static Block* end_portal_frame;
    static Block* end_stone;
    static Block* dragon_egg;
    static Block* redstone_lamp;
    static Block* lit_redstone_lamp;
    static HalfSlabBlock* double_wooden_slab;
    static HalfSlabBlock* wooden_slab;
    static Block* cocoa;
    static Block* sandstone_stairs;
    static Block* emerald_ore;
    static Block* ender_chest;
    static TripWireHookBlock* tripwire_hook;
    static Block* tripwire;
    static Block* emerald_block;
    static Block* spruce_stairs;
    static Block* birch_stairs;
    static Block* jungle_stairs;
    static Block* command_block;
    static BeaconBlock* beacon;
    static Block* cobblestone_wall;
    static Block* flower_pot;
    static Block* carrots;
    static Block* potatoes;
    static Block* wooden_button;
    static SkullBlock* skull;
    static Block* anvil;
    static ChestBlock* trapped_chest;
    static Block* light_weighted_pressure_plate;
    static Block* heavy_weighted_pressure_plate;
    static ComparatorBlock* unpowered_comparator;
    static ComparatorBlock* powered_comparator;
    static DaylightDetectorBlock* daylight_detector;
    static DaylightDetectorBlock* daylight_detector_inverted;
    static Block* redstone_block;
    static Block* quartz_ore;
    static HopperBlock* hopper;
    static Block* quartz_block;
    static Block* quartz_stairs;
    static Block* activator_rail;
    static Block* dropper;
    static Block* stained_hardened_clay;
    static Block* barrier;
    static Block* iron_trapdoor;
    static Block* hay_block;
    static Block* carpet;
    static Block* hardened_clay;
    static Block* coal_block;
    static Block* packed_ice;
    static Block* acacia_stairs;
    static Block* dark_oak_stairs;
    static Block* slime;
    static DoublePlantBlock* double_plant;
    static StainedGlassBlock* stained_glass;
    static StainedGlassPaneBlock* stained_glass_pane;
    static Block* prismarine;
    static Block* sea_lantern;
    static Block* standing_banner;
    static Block* wall_banner;
    static Block* red_sandstone;
    static Block* red_sandstone_stairs;
    static HalfSlabBlock* double_stone_slab2;
    static HalfSlabBlock* stone_slab2;
    static Block* end_rod;
    static Block* chorus_plant;
    static Block* chorus_flower;
    static Block* purpur_block;
    static Block* purpur_pillar;
    static Block* purpur_stairs;
    static HalfSlabBlock* purpur_double_slab;
    static HalfSlabBlock* purpur_slab;
    static Block* end_bricks;
    static Block* beetroots;
    static Block* grass_path;
    static Block* end_gateway;
    static Block* repeating_command_block;
    static Block* chain_command_block;
    static Block* frosted_ice;
    static Block* magma;
    static Block* nether_wart_block;
    static Block* red_nether_brick;
    static Block* bone_block;
    static Block* structure_void;
    static Block* observer;
    static Block* white_shulker_box;
    static Block* orange_shulker_box;
    static Block* magenta_shulker_box;
    static Block* light_blue_shulker_box;
    static Block* yellow_shulker_box;
    static Block* lime_shulker_box;
    static Block* pink_shulker_box;
    static Block* gray_shulker_box;
    static Block* silver_shulker_box;
    static Block* cyan_shulker_box;
    static Block* purple_shulker_box;
    static Block* blue_shulker_box;
    static Block* brown_shulker_box;
    static Block* green_shulker_box;
    static Block* red_shulker_box;
    static Block* black_shulker_box;
    static Block* white_glazed_terracotta;
    static Block* orange_glazed_terracotta;
    static Block* magenta_glazed_terracotta;
    static Block* light_blue_glazed_terracotta;
    static Block* yellow_glazed_terracotta;
    static Block* lime_glazed_terracotta;
    static Block* pink_glazed_terracotta;
    static Block* gray_glazed_terracotta;
    static Block* silver_glazed_terracotta;
    static Block* cyan_glazed_terracotta;
    static Block* purple_glazed_terracotta;
    static Block* blue_glazed_terracotta;
    static Block* brown_glazed_terracotta;
    static Block* green_glazed_terracotta;
    static Block* red_glazed_terracotta;
    static Block* black_glazed_terracotta;
    static Block* concrete;
    static Block* concrete_powder;
    static Block* structure_block;
    static Block* conduit;
    static Block* kelp;
    static Block* coral_block;
    static Block* coral;
    static Block* coral_fan;
    static Block* coral_fan_dead;
    static Block* coral_fan_hang;
    static Block* coral_fan_hang2;
    static Block* coral_fan_hang3;
    static Block* dried_kelp_block;
    static Block* sea_grass;
    static Block* sea_pickle;
    static Block* bubble_column_segment;
    static Block* blue_ice;
    static Block* turtle_egg;
    static Block* spruce_trapdoor;
    static Block* birch_trapdoor;
    static Block* jungle_trapdoor;
    static Block* acacia_trapdoor;
    static Block* dark_oak_trapdoor;
    static Block* prismarine_stairs;
    static Block* prismarine_bricks_stairs;
    static Block* dark_prismarine_stairs;
    static Block* prismarine_double_slab;
    static Block* prismarine_slab;
    static Block* stripped_log_spruce;
    static Block* stripped_log_birch;
    static Block* stripped_log_jungle;
    static Block* stripped_log_acacia;
    static Block* stripped_log_dark_oak;
    static Block* stripped_log_oak;
    static Block* acacia_pressure_plate;
    static Block* birch_pressure_plate;
    static Block* dark_oak_pressure_plate;
    static Block* jungle_pressure_plate;
    static Block* spruce_pressure_plate;
    static Block* acacia_button;
    static Block* birch_button;
    static Block* dark_oak_button;
    static Block* jungle_button;
    static Block* spruce_button;
    static Block* bark_spruce;
    static Block* bark_birch;
    static Block* bark_jungle;
    static Block* bark_acacia;
    static Block* bark_dark_oak;
    static Block* bark_oak;

private:
    static Block* get(const std::wstring& name);
};

#endif
