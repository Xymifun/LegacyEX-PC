#include "game/core/ColourTable.hpp"

#include <boost/unordered_map.hpp>
#include <cwchar>
#include <sstream>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/TerracottaLayerTheme.hpp"
#include "game/core/XMem.hpp"


typedef boost::unordered_map<std::wstring, eMinecraftColour> ColourNameMap;
typedef boost::unordered_map<int, ColourTable::BiomeColourInfo> BiomeColourInfoMap;

static const int DEFAULT_BIOME_ID = -1;

static const wchar_t* COLOUR_NAMES[ColourTable::NUM_COLOURS] = {L"NOTSET",
                                                                L"Foliage_Evergreen",
                                                                L"Foliage_Birch",
                                                                L"Foliage_Default",
                                                                L"Foliage_Mesa",
                                                                L"Foliage_Swampland",
                                                                L"Grass_Common",
                                                                L"Grass_Swamp1",
                                                                L"Grass_Swamp2",
                                                                L"Grass_Mesa",
                                                                L"Grass_Roofed_Forest_Modifier",
                                                                L"Water_Ocean",
                                                                L"Water_Plains",
                                                                L"Water_Desert",
                                                                L"Water_ExtremeHills",
                                                                L"Water_Forest",
                                                                L"Water_Taiga",
                                                                L"Water_Swampland",
                                                                L"Water_River",
                                                                L"Water_Hell",
                                                                L"Water_Sky",
                                                                L"Water_FrozenOcean",
                                                                L"Water_FrozenRiver",
                                                                L"Water_IcePlains",
                                                                L"Water_IceMountains",
                                                                L"Water_MushroomIsland",
                                                                L"Water_MushroomIslandShore",
                                                                L"Water_Beach",
                                                                L"Water_DesertHills",
                                                                L"Water_ForestHills",
                                                                L"Water_TaigaHills",
                                                                L"Water_ExtremeHillsEdge",
                                                                L"Water_Jungle",
                                                                L"Water_JungleHills",
                                                                L"Water_JungleEdge",
                                                                L"Water_DeepOcean",
                                                                L"Water_StoneBeach",
                                                                L"Water_ColdBeach",
                                                                L"Water_BirchForest",
                                                                L"Water_BirchForestHills",
                                                                L"Water_RoofedForest",
                                                                L"Water_ColdTaiga",
                                                                L"Water_ColdTaigaHills",
                                                                L"Water_MegaTaiga",
                                                                L"Water_MegaTaigaHills",
                                                                L"Water_ExtremeHillsPlus",
                                                                L"Water_Savanna",
                                                                L"Water_SavannaPlateau",
                                                                L"Water_Mesa",
                                                                L"Water_MesaPlateauF",
                                                                L"Water_MesaPlateau",
                                                                L"Sky_Ocean",
                                                                L"Sky_Plains",
                                                                L"Sky_Desert",
                                                                L"Sky_ExtremeHills",
                                                                L"Sky_Forest",
                                                                L"Sky_Taiga",
                                                                L"Sky_Swampland",
                                                                L"Sky_River",
                                                                L"Sky_Hell",
                                                                L"Sky_Sky",
                                                                L"Sky_FrozenOcean",
                                                                L"Sky_FrozenRiver",
                                                                L"Sky_IcePlains",
                                                                L"Sky_IceMountains",
                                                                L"Sky_MushroomIsland",
                                                                L"Sky_MushroomIslandShore",
                                                                L"Sky_Beach",
                                                                L"Sky_DesertHills",
                                                                L"Sky_ForestHills",
                                                                L"Sky_TaigaHills",
                                                                L"Sky_ExtremeHillsEdge",
                                                                L"Sky_Jungle",
                                                                L"Sky_JungleHills",
                                                                L"Sky_JungleEdge",
                                                                L"Sky_DeepOcean",
                                                                L"Sky_StoneBeach",
                                                                L"Sky_ColdBeach",
                                                                L"Sky_BirchForest",
                                                                L"Sky_BirchForestHills",
                                                                L"Sky_RoofedForest",
                                                                L"Sky_ColdTaiga",
                                                                L"Sky_ColdTaigaHills",
                                                                L"Sky_MegaTaiga",
                                                                L"Sky_MegaTaigaHills",
                                                                L"Sky_ExtremeHillsPlus",
                                                                L"Sky_Savanna",
                                                                L"Sky_SavannaPlateau",
                                                                L"Sky_Mesa",
                                                                L"Sky_MesaPlateauF",
                                                                L"Sky_MesaPlateau",
                                                                L"Preview_Ocean",
                                                                L"Preview_Plains",
                                                                L"Preview_Desert",
                                                                L"Preview_ExtremeHills",
                                                                L"Preview_Forest",
                                                                L"Preview_Taiga",
                                                                L"Preview_Swampland",
                                                                L"Preview_River",
                                                                L"Preview_Hell",
                                                                L"Preview_Sky",
                                                                L"Preview_FrozenOcean",
                                                                L"Preview_FrozenRiver",
                                                                L"Preview_IcePlains",
                                                                L"Preview_IceMountains",
                                                                L"Preview_MushroomIsland",
                                                                L"Preview_MushroomIslandShore",
                                                                L"Preview_Beach",
                                                                L"Preview_DesertHills",
                                                                L"Preview_ForestHills",
                                                                L"Preview_TaigaHills",
                                                                L"Preview_ExtremeHillsEdge",
                                                                L"Preview_Jungle",
                                                                L"Preview_JungleHills",
                                                                L"Preview_JungleEdge",
                                                                L"Preview_DeepOcean",
                                                                L"Preview_StoneBeach",
                                                                L"Preview_ColdBeach",
                                                                L"Preview_BirchForest",
                                                                L"Preview_BirchForestHills",
                                                                L"Preview_RoofedForest",
                                                                L"Preview_ColdTaiga",
                                                                L"Preview_ColdTaigaHills",
                                                                L"Preview_MegaTaiga",
                                                                L"Preview_MegaTaigaHills",
                                                                L"Preview_ExtremeHillsPlus",
                                                                L"Preview_Savanna",
                                                                L"Preview_SavannaPlateau",
                                                                L"Preview_Mesa",
                                                                L"Preview_MesaPlateauF",
                                                                L"Preview_MesaPlateau",
                                                                L"Preview_Plains_Mutated",
                                                                L"Preview_Desert_Mutated",
                                                                L"Preview_ExtremeHills_Mutated",
                                                                L"Preview_Forest_Mutated",
                                                                L"Preview_Taiga_Mutated",
                                                                L"Preview_Swampland_Mutated",
                                                                L"Preview_IcePlains_Mutated",
                                                                L"Preview_Jungle_Mutated",
                                                                L"Preview_JungleEdge_Mutated",
                                                                L"Preview_BirchForest_Mutated",
                                                                L"Preview_BirchForestHills_Mutated",
                                                                L"Preview_RoofedForest_Mutated",
                                                                L"Preview_ColdTaiga_Mutated",
                                                                L"Preview_MegaTaiga_Mutated",
                                                                L"Preview_MegaTaigaHills_Mutated",
                                                                L"Preview_ExtremeHillsPlus_Mutated",
                                                                L"Preview_Savanna_Mutated",
                                                                L"Preview_SavannaPlateau_Mutated",
                                                                L"Preview_Mesa_Mutated",
                                                                L"Preview_MesaPlateauF_Mutated",
                                                                L"Preview_MesaPlateau_Mutated",
                                                                L"Tile_RedstoneDust",
                                                                L"Tile_RedstoneDustUnlit",
                                                                L"Tile_RedstoneDustLitMin",
                                                                L"Tile_RedstoneDustLitMax",
                                                                L"Tile_StemMin",
                                                                L"Tile_StemMax",
                                                                L"Tile_WaterLily",
                                                                L"Sky_Dawn_Dark",
                                                                L"Sky_Dawn_Bright",
                                                                L"Material_None",
                                                                L"Material_Grass",
                                                                L"Material_Sand",
                                                                L"Material_Cloth",
                                                                L"Material_Fire",
                                                                L"Material_Ice",
                                                                L"Material_Metal",
                                                                L"Material_Plant",
                                                                L"Material_Snow",
                                                                L"Material_Clay",
                                                                L"Material_Dirt",
                                                                L"Material_Stone",
                                                                L"Material_Water",
                                                                L"Material_Wood",
                                                                L"Material_Emerald",
                                                                L"Material_Quartz",
                                                                L"Material_Gold",
                                                                L"Material_Diamond",
                                                                L"Material_Lapis",
                                                                L"Material_Podzol",
                                                                L"Material_Nether",
                                                                L"Material_Color_Orange",
                                                                L"Material_Color_Magenta",
                                                                L"Material_Color_Light_Blue",
                                                                L"Material_Color_Yellow",
                                                                L"Material_Color_Light_Green",
                                                                L"Material_Color_Pink",
                                                                L"Material_Color_Grey",
                                                                L"Material_Color_Silver",
                                                                L"Material_Color_Cyan",
                                                                L"Material_Color_Purple",
                                                                L"Material_Color_Blue",
                                                                L"Material_Color_Brown",
                                                                L"Material_Color_Green",
                                                                L"Material_Color_Red",
                                                                L"Material_Color_Black",
                                                                L"Material_Terracotta_White",
                                                                L"Material_Terracotta_Orange",
                                                                L"Material_Terracotta_Magenta",
                                                                L"Material_Terracotta_Light_Blue",
                                                                L"Material_Terracotta_Yellow",
                                                                L"Material_Terracotta_Light_Green",
                                                                L"Material_Terracotta_Pink",
                                                                L"Material_Terracotta_Grey",
                                                                L"Material_Terracotta_Silver",
                                                                L"Material_Terracotta_Cyan",
                                                                L"Material_Terracotta_Purple",
                                                                L"Material_Terracotta_Blue",
                                                                L"Material_Terracotta_Brown",
                                                                L"Material_Terracotta_Green",
                                                                L"Material_Terracotta_Red",
                                                                L"Material_Terracotta_Black",
                                                                L"Particle_Note_00",
                                                                L"Particle_Note_01",
                                                                L"Particle_Note_02",
                                                                L"Particle_Note_03",
                                                                L"Particle_Note_04",
                                                                L"Particle_Note_05",
                                                                L"Particle_Note_06",
                                                                L"Particle_Note_07",
                                                                L"Particle_Note_08",
                                                                L"Particle_Note_09",
                                                                L"Particle_Note_10",
                                                                L"Particle_Note_11",
                                                                L"Particle_Note_12",
                                                                L"Particle_Note_13",
                                                                L"Particle_Note_14",
                                                                L"Particle_Note_15",
                                                                L"Particle_Note_16",
                                                                L"Particle_Note_17",
                                                                L"Particle_Note_18",
                                                                L"Particle_Note_19",
                                                                L"Particle_Note_20",
                                                                L"Particle_Note_21",
                                                                L"Particle_Note_22",
                                                                L"Particle_Note_23",
                                                                L"Particle_Note_24",
                                                                L"Particle_NetherPortal",
                                                                L"Particle_EnderPortal",
                                                                L"Particle_Smoke",
                                                                L"Particle_Ender",
                                                                L"Particle_Explode",
                                                                L"Particle_HugeExplosion",
                                                                L"Particle_DripWater",
                                                                L"Particle_DripLavaStart",
                                                                L"Particle_DripLavaEnd",
                                                                L"Particle_EnchantmentTable",
                                                                L"Particle_DragonBreathMin",
                                                                L"Particle_DragonBreathMax",
                                                                L"Particle_Suspend",
                                                                L"Particle_CritStart",
                                                                L"Particle_CritEnd",
                                                                L"Effect_MovementSpeed",
                                                                L"Effect_MovementSlowDown",
                                                                L"Effect_DigSpeed",
                                                                L"Effect_DigSlowdown",
                                                                L"Effect_DamageBoost",
                                                                L"Effect_Heal",
                                                                L"Effect_Harm",
                                                                L"Effect_Jump",
                                                                L"Effect_Confusion",
                                                                L"Effect_Regeneration",
                                                                L"Effect_DamageResistance",
                                                                L"Effect_FireResistance",
                                                                L"Effect_WaterBreathing",
                                                                L"Effect_Invisiblity",
                                                                L"Effect_Blindness",
                                                                L"Effect_NightVision",
                                                                L"Effect_Hunger",
                                                                L"Effect_Weakness",
                                                                L"Effect_Poison",
                                                                L"Effect_Wither",
                                                                L"Effect_HealthBoost",
                                                                L"Effect_Absorption",
                                                                L"Effect_Saturation",
                                                                L"Effect_Glowing",
                                                                L"Effect_Levitation",
                                                                L"Effect_Luck",
                                                                L"Effect_BadLuck",
                                                                L"Effect_TurtleMaster",
                                                                L"Effect_SlowFall",
                                                                L"Effect_ConduitPower",
                                                                L"Potion_BaseColour",
                                                                L"Mob_Creeper_Colour1",
                                                                L"Mob_Creeper_Colour2",
                                                                L"Mob_Skeleton_Colour1",
                                                                L"Mob_Skeleton_Colour2",
                                                                L"Mob_Spider_Colour1",
                                                                L"Mob_Spider_Colour2",
                                                                L"Mob_Zombie_Colour1",
                                                                L"Mob_Zombie_Colour2",
                                                                L"Mob_Slime_Colour1",
                                                                L"Mob_Slime_Colour2",
                                                                L"Mob_Ghast_Colour1",
                                                                L"Mob_Ghast_Colour2",
                                                                L"Mob_PigZombie_Colour1",
                                                                L"Mob_PigZombie_Colour2",
                                                                L"Mob_Enderman_Colour1",
                                                                L"Mob_Enderman_Colour2",
                                                                L"Mob_CaveSpider_Colour1",
                                                                L"Mob_CaveSpider_Colour2",
                                                                L"Mob_Silverfish_Colour1",
                                                                L"Mob_Silverfish_Colour2",
                                                                L"Mob_Blaze_Colour1",
                                                                L"Mob_Blaze_Colour2",
                                                                L"Mob_LavaSlime_Colour1",
                                                                L"Mob_LavaSlime_Colour2",
                                                                L"Mob_Pig_Colour1",
                                                                L"Mob_Pig_Colour2",
                                                                L"Mob_Sheep_Colour1",
                                                                L"Mob_Sheep_Colour2",
                                                                L"Mob_Cow_Colour1",
                                                                L"Mob_Cow_Colour2",
                                                                L"Mob_Chicken_Colour1",
                                                                L"Mob_Chicken_Colour2",
                                                                L"Mob_Squid_Colour1",
                                                                L"Mob_Squid_Colour2",
                                                                L"Mob_Wolf_Colour1",
                                                                L"Mob_Wolf_Colour2",
                                                                L"Mob_MushroomCow_Colour1",
                                                                L"Mob_MushroomCow_Colour2",
                                                                L"Mob_Ocelot_Colour1",
                                                                L"Mob_Ocelot_Colour2",
                                                                L"Mob_Villager_Colour1",
                                                                L"Mob_Villager_Colour2",
                                                                L"Mob_Bat_Colour1",
                                                                L"Mob_Bat_Colour2",
                                                                L"Mob_Witch_Colour1",
                                                                L"Mob_Witch_Colour2",
                                                                L"Mob_Horse_Colour1",
                                                                L"Mob_Horse_Colour2",
                                                                L"Mob_Endermite_Color1",
                                                                L"Mob_Endermite_Color2",
                                                                L"Mob_Guardian_Color1",
                                                                L"Mob_Guardian_Color2",
                                                                L"Mob_Rabbit_Colour1",
                                                                L"Mob_Rabbit_Colour2",
                                                                L"Mob_PolarBear_Colour1",
                                                                L"Mob_PolarBear_Colour2",
                                                                L"Mob_Shulker_Colour1",
                                                                L"Mob_Shulker_Colour2",
                                                                L"Mob_Elder_Guardian_Colour1",
                                                                L"Mob_Elder_Guardian_Colour2",
                                                                L"Mob_Evocation_Illager_Colour1",
                                                                L"Mob_Evocation_Illager_Colour2",
                                                                L"Mob_Llama_Colour1",
                                                                L"Mob_Llama_Colour2",
                                                                L"Mob_Donkey_Colour1",
                                                                L"Mob_Donkey_Colour2",
                                                                L"Mob_Skeleton_Horse_Colour1",
                                                                L"Mob_Skeleton_Horse_Colour2",
                                                                L"Mob_Zombie_Horse_Colour1",
                                                                L"Mob_Zombie_Horse_Colour2",
                                                                L"Mob_Mule_Colour1",
                                                                L"Mob_Mule_Colour2",
                                                                L"Mob_Stray_Colour1",
                                                                L"Mob_Stray_Colour2",
                                                                L"Mob_Husk_Colour1",
                                                                L"Mob_Husk_Colour2",
                                                                L"Mob_Vex_Colour1",
                                                                L"Mob_Vex_Colour2",
                                                                L"Mob_Vindication_Illager_Colour1",
                                                                L"Mob_Vindication_Illager_Colour2",
                                                                L"Mob_Zombie_Villager_Colour1",
                                                                L"Mob_Zombie_Villager_Colour2",
                                                                L"Mob_Parrot_Colour1",
                                                                L"Mob_Parrot_Colour2",
                                                                L"Mob_Wither_Skeleton_Colour1",
                                                                L"Mob_Wither_Skeleton_Colour2",
                                                                L"Mob_Turtle_Colour1",
                                                                L"Mob_Turtle_Colour2",
                                                                L"Mob_Dolphin_Colour1",
                                                                L"Mob_Dolphin_Colour2",
                                                                L"Mob_Tropical_Colour1",
                                                                L"Mob_Tropical_Colour2",
                                                                L"Mob_Cod_Colour1",
                                                                L"Mob_Cod_Colour2",
                                                                L"Mob_Pufferfish_Colour1",
                                                                L"Mob_Pufferfish_Colour2",
                                                                L"Mob_Salmon_Colour1",
                                                                L"Mob_Salmon_Colour2",
                                                                L"Mob_Drowned_Colour1",
                                                                L"Mob_Drowned_Colour2",
                                                                L"Mob_Phantom_Colour1",
                                                                L"Mob_Phantom_Colour2",
                                                                L"Armour_Default_Leather_Colour",
                                                                L"Under_Water_Clear_Colour",
                                                                L"Under_Lava_Clear_Colour",
                                                                L"In_Cloud_Base_Colour",
                                                                L"Under_Water_Fog_Colour",
                                                                L"Under_Lava_Fog_Colour",
                                                                L"In_Cloud_Fog_Colour",
                                                                L"Default_Fog_Colour",
                                                                L"Nether_Fog_Colour",
                                                                L"End_Fog_Colour",
                                                                L"Sign_Text",
                                                                L"Map_Text",
                                                                L"Leash_Light_Colour",
                                                                L"Leash_Dark_Colour",
                                                                L"Fire_Overlay",
                                                                L"HTMLColor_0",
                                                                L"HTMLColor_1",
                                                                L"HTMLColor_2",
                                                                L"HTMLColor_3",
                                                                L"HTMLColor_4",
                                                                L"HTMLColor_5",
                                                                L"HTMLColor_6",
                                                                L"HTMLColor_7",
                                                                L"HTMLColor_8",
                                                                L"HTMLColor_9",
                                                                L"HTMLColor_a",
                                                                L"HTMLColor_b",
                                                                L"HTMLColor_c",
                                                                L"HTMLColor_d",
                                                                L"HTMLColor_e",
                                                                L"HTMLColor_f",
                                                                L"HTMLColor_dark_0",
                                                                L"HTMLColor_dark_1",
                                                                L"HTMLColor_dark_2",
                                                                L"HTMLColor_dark_3",
                                                                L"HTMLColor_dark_4",
                                                                L"HTMLColor_dark_5",
                                                                L"HTMLColor_dark_6",
                                                                L"HTMLColor_dark_7",
                                                                L"HTMLColor_dark_8",
                                                                L"HTMLColor_dark_9",
                                                                L"HTMLColor_dark_a",
                                                                L"HTMLColor_dark_b",
                                                                L"HTMLColor_dark_c",
                                                                L"HTMLColor_dark_d",
                                                                L"HTMLColor_dark_e",
                                                                L"HTMLColor_dark_f",
                                                                L"HTMLColor_T1",
                                                                L"HTMLColor_T2",
                                                                L"HTMLColor_T3",
                                                                L"HTMLColor_Black",
                                                                L"HTMLColor_White",
                                                                L"Color_EnchantText",
                                                                L"Color_EnchantTextFocus",
                                                                L"Color_EnchantTextDisabled",
                                                                L"Color_RenamedItemTitle",
                                                                L"Color_ItemCount",
                                                                L"BlockLight_OverworldDimension",
                                                                L"BlockLight_HellDimension",
                                                                L"BlockLight_EndDimension",
                                                                L"Banner_White",
                                                                L"Banner_Orange",
                                                                L"Banner_Magenta",
                                                                L"Banner_Light_Blue",
                                                                L"Banner_Yellow",
                                                                L"Banner_Lime",
                                                                L"Banner_Pink",
                                                                L"Banner_Gray",
                                                                L"Banner_Silver",
                                                                L"Banner_Cyan",
                                                                L"Banner_Purple",
                                                                L"Banner_Blue",
                                                                L"Banner_Brown",
                                                                L"Banner_Green",
                                                                L"Banner_Red",
                                                                L"Banner_Black",
                                                                L"Thermal_Lift",
                                                                L"Thermal_Constant",
                                                                L"Thermal_ConstantToHeight",
                                                                L"Thermal_Speedboost",
                                                                L"Thermal_Speedboost2",
                                                                L"Particle_ScoreRing_Large",
                                                                L"Particle_ScoreRing_Medium",
                                                                L"Particle_ScoreRing_Small",
                                                                L"Shulker_Box_White",
                                                                L"Shulker_Box_Orange",
                                                                L"Shulker_Box_Magenta",
                                                                L"Shulker_Box_Light_Blue",
                                                                L"Shulker_Box_Yellow",
                                                                L"Shulker_Box_Light_Green",
                                                                L"Shulker_Box_Pink",
                                                                L"Shulker_Box_Grey",
                                                                L"Shulker_Box_Silver",
                                                                L"Shulker_Box_Cyan",
                                                                L"Shulker_Box_Purple",
                                                                L"Shulker_Box_Blue",
                                                                L"Shulker_Box_Brown",
                                                                L"Shulker_Box_Green",
                                                                L"Shulker_Box_Red",
                                                                L"Shulker_Box_Black",
                                                                L"Bed_White",
                                                                L"Bed_Orange",
                                                                L"Bed_Magenta",
                                                                L"Bed_Light_Blue",
                                                                L"Bed_Yellow",
                                                                L"Bed_Lime",
                                                                L"Bed_Pink",
                                                                L"Bed_Grey",
                                                                L"Bed_Silver",
                                                                L"Bed_Cyan",
                                                                L"Bed_Purple",
                                                                L"Bed_Blue",
                                                                L"Bed_Brown",
                                                                L"Bed_Green",
                                                                L"Bed_Red",
                                                                L"Bed_Black",
                                                                L"Glide_Ghost_Slow",
                                                                L"Glide_Ghost_Medium",
                                                                L"Glide_Ghost_Fast",
                                                                L"Cauldron_Water"};

static ColourNameMap sColourIndicesByName;
static BiomeColourInfoMap sBiomeColourInfo;
static const std::wstring DEFAULT_BIOME_NAME = L"default";

Block* getColour(unsigned char colour)
{
    static Block* colours[16];
    static int initialized;
    int index = colour & 0xf;

    if (!initialized)
    {
        initialized = 1;
        colours[0] = Blocks::black_glazed_terracotta;
        colours[1] = Blocks::blue_glazed_terracotta;
        colours[2] = Blocks::light_blue_glazed_terracotta;
        colours[3] = Blocks::cyan_glazed_terracotta;
        colours[4] = Blocks::green_glazed_terracotta;
        colours[5] = Blocks::lime_glazed_terracotta;
        colours[6] = Blocks::pink_glazed_terracotta;
        colours[7] = Blocks::magenta_glazed_terracotta;
        colours[8] = Blocks::purple_glazed_terracotta;
        colours[9] = Blocks::red_glazed_terracotta;
        colours[10] = Blocks::brown_glazed_terracotta;
        colours[11] = Blocks::orange_glazed_terracotta;
        colours[12] = Blocks::yellow_glazed_terracotta;
        colours[13] = Blocks::white_glazed_terracotta;
        colours[14] = Blocks::silver_glazed_terracotta;
        colours[15] = Blocks::gray_glazed_terracotta;
    }

    return colours[index];
}

TerracottaLayerThemeFunction getColourFunc(const std::wstring& function)
{
    if (function.compare(0, function.size(), L"CONCENTRIC_RINGS", wcslen(L"CONCENTRIC_RINGS")) == 0)
    {
        return reinterpret_cast<TerracottaLayerThemeFunction>(&TerracottaLayerTheme::CONCENTRIC_RINGS);
    }
    if (function.compare(0, function.size(), L"OFFSET_RINGS", wcslen(L"OFFSET_RINGS")) == 0)
    {
        return reinterpret_cast<TerracottaLayerThemeFunction>(&TerracottaLayerTheme::OFFSET_RINGS);
    }
    if (function.compare(0, function.size(), L"HASHED", wcslen(L"HASHED")) == 0)
    {
        return reinterpret_cast<TerracottaLayerThemeFunction>(&TerracottaLayerTheme::HASHED);
    }
    if (function.compare(0, function.size(), L"TARTAN", wcslen(L"TARTAN")) == 0)
    {
        return &TerracottaLayerTheme::TARTAN;
    }
    if (function.compare(0, function.size(), L"UNIFORM", wcslen(L"UNIFORM")) == 0)
    {
        return reinterpret_cast<TerracottaLayerThemeFunction>(&TerracottaLayerTheme::UNIFORM);
    }
    if (function.compare(0, function.size(), L"STRIPES", wcslen(L"STRIPES")) == 0)
    {
        return reinterpret_cast<TerracottaLayerThemeFunction>(&TerracottaLayerTheme::STRIPES);
    }
    return 0;
}

void ColourTable::staticCtor()
{
    for (int index = 0; index < NUM_COLOURS; index++)
    {
        sColourIndicesByName.insert(std::make_pair(COLOUR_NAMES[index], (eMinecraftColour)index));
    }
}

void ColourTable::setColour(const std::wstring& name, int colour)
{
    ColourNameMap::const_iterator entry = sColourIndicesByName.find(name);

    if (entry != sColourIndicesByName.end())
    {
        mColours[entry->second] = colour;
    }
}

void ColourTable::setColour(const std::wstring& name, const std::wstring& colour)
{
    unsigned int value = 0;
    if (swscanf(colour.c_str(), L"%x", &value) == 1)
    {
        setColour(name, (int)value);
    }
}

void ColourTable::setColour(eMinecraftColour colour, int value)
{
    mColours[(int)colour] = value;
}

void ColourTable::loadColoursFromTextData(const std::wstring& text)
{
    std::wistringstream input(text);
    std::wstring line;
    while (std::getline(input, line))
    {
        std::wstring::size_type separator = line.find(L'=');
        if (separator == std::wstring::npos)
        {
            separator = line.find(L' ');
        }
        if (separator != std::wstring::npos)
        {
            std::wstring name = line.substr(0, separator);
            std::wstring value = line.substr(separator + 1);
            if (!name.empty() && !value.empty())
            {
                setColour(name, value);
            }
        }
    }
}

arrayWithLength<unsigned char> ColourTable::getBinaryData()
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    stream.writeInt(1);
    stream.writeInt(NUM_COLOURS);
    for (int i = 0; i < NUM_COLOURS; ++i)
    {
        stream.writeUTF(COLOUR_NAMES[i]);
        stream.writeInt(mColours[i]);
    }
    stream.writeInt((int)sBiomeColourInfo.size());
    BiomeColourInfoMap::const_iterator entry = sBiomeColourInfo.begin();
    for (; entry != sBiomeColourInfo.end(); ++entry)
    {
        std::wstringstream name;
        if (entry->first == DEFAULT_BIOME_ID)
        {
            name << DEFAULT_BIOME_NAME;
        }
        else
        {
            name << entry->first;
        }
        stream.writeUTF(name.str());
        stream.writeInt((int)entry->second.waterSurfaceColour);
        stream.writeInt((int)entry->second.waterFogColour);
        stream.writeInt((int)entry->second.waterFogDistance);
    }
    return byteStream.toByteArray();
}

std::wstring ColourTable::getTextData()
{
    std::wstringstream output;
    for (int i = 0; i < NUM_COLOURS; ++i)
    {
        output << COLOUR_NAMES[i] << L"=" << std::hex << (unsigned int)mColours[i] << L"\n";
    }
    return output.str();
}

void ColourTable::loadColoursFromData(unsigned char* data, unsigned int length)
{
    arrayWithLength<unsigned char> buffer(data, length);

    ByteArrayInputStream byteStream(buffer);
    DataInputStream stream(&byteStream);

    stream.readInt();

    int colourCount = stream.readInt();

    for (int colourIndex = 0; colourIndex < colourCount; colourIndex++)
    {
        std::wstring name = stream.readUTF();
        int colour = stream.readInt();

        setColour(name, colour);
    }

    int biomeCount = stream.readInt();

    if (biomeCount > 0)
    {
        sBiomeColourInfo.clear();
    }

    for (int biomeIndex = 0; biomeIndex < biomeCount; biomeIndex++)
    {
        std::wstring biomeName = stream.readUTF();

        BiomeColourInfo info;
        info.waterSurfaceColour = stream.readInt();
        info.waterFogColour = stream.readInt();
        info.waterFogDistance = stream.readInt();

        if (biomeName == DEFAULT_BIOME_NAME)
        {
            sBiomeColourInfo[DEFAULT_BIOME_ID] = info;
        }
        else
        {
            Biome* biome = Biome::byNameId(biomeName);
#ifdef MINECRAFT_PC
            if (biome == NULL)
                continue;
#endif
            sBiomeColourInfo[biome->id] = info;
        }
    }

    byteStream.reset();
}

ColourTable::ColourTable(unsigned char* data, unsigned int length)
{
    loadColoursFromData(data, length);
}

ColourTable::ColourTable(ColourTable* source, unsigned char* data, unsigned int length)
{
    XMemCpy(this, source, sizeof(ColourTable));

    loadColoursFromData(data, length);
}

unsigned int ColourTable::getColour(eMinecraftColour colour)
{
    return mColours[colour];
}

const ColourTable::BiomeColourInfo& ColourTable::getBiomeColourInfo(int biomeId, int baseBiomeId) const
{
    BiomeColourInfoMap::const_iterator entry = sBiomeColourInfo.find(biomeId);

    if (entry != sBiomeColourInfo.end())
    {
        return entry->second;
    }

    BiomeColourInfoMap::const_iterator baseEntry = sBiomeColourInfo.find(baseBiomeId);

    if (baseEntry != sBiomeColourInfo.end())
    {
        return baseEntry->second;
    }

    return sBiomeColourInfo.at(DEFAULT_BIOME_ID);
}

unsigned int ColourTable::getWaterSurfaceColor(int biomeId, int baseBiomeId)
{
    return getBiomeColourInfo(biomeId, baseBiomeId).waterSurfaceColour;
}

unsigned int ColourTable::getWaterFogColor(int biomeId, int baseBiomeId)
{
    return getBiomeColourInfo(biomeId, baseBiomeId).waterFogColour;
}

unsigned int ColourTable::getWaterFogDistance(int biomeId, int baseBiomeId)
{
    return getBiomeColourInfo(biomeId, baseBiomeId).waterFogDistance;
}

unsigned int ColourTable::LinearBlend(int startColour, int middleColour, int endColour, float fraction)
{
    int startRed = (startColour >> 16) & 0xff;
    int middleRed = (middleColour >> 16) & 0xff;
    int endRed = (endColour >> 16) & 0xff;

    int red;

    if (fraction < 0.5f)
    {
        red = (int)((float)startRed + (fraction * 2.0f) * (float)(middleRed - startRed));
    }
    else
    {
        red = (int)((float)middleRed + ((fraction - 0.5f) * 2.0f) * (float)(endRed - middleRed));
    }

    int startGreen = (startColour >> 8) & 0xff;
    int middleGreen = (middleColour >> 8) & 0xff;
    int endGreen = (endColour >> 8) & 0xff;

    int green;

    if (fraction < 0.5f)
    {
        green = (int)((float)startGreen + (fraction * 2.0f) * (float)(middleGreen - startGreen));
    }
    else
    {
        green = (int)((float)middleGreen + ((fraction - 0.5f) * 2.0f) * (float)(endGreen - middleGreen));
    }

    int startBlue = startColour & 0xff;
    int middleBlue = middleColour & 0xff;
    int endBlue = endColour & 0xff;

    int blue;

    if (fraction < 0.5f)
    {
        blue = (int)((float)startBlue + (fraction * 2.0f) * (float)(middleBlue - startBlue));
    }
    else
    {
        blue = (int)((float)middleBlue + ((fraction - 0.5f) * 2.0f) * (float)(endBlue - middleBlue));
    }

    int redClamped = Mth::clamp(red, 0, 255);
    int greenClamped = Mth::clamp(green, 0, 255);
    int blueClamped = Mth::clamp(blue, 0, 255);
    int alphaClamped = Mth::clamp(255, 0, 255);
    return (redClamped << 16) | (greenClamped << 8) | blueClamped | (alphaClamped << 24);
}

unsigned int ColourTable::LinearBlend(int startColour, int endColour, float fraction)
{
    int red = (int)((float)((startColour >> 16) & 0xff)
                    + fraction * (float)(((endColour >> 16) & 0xff) - ((startColour >> 16) & 0xff)));
    int green = (int)((float)((startColour >> 8) & 0xff)
                      + fraction * (float)(((endColour >> 8) & 0xff) - ((startColour >> 8) & 0xff)));
    int blue = (int)((float)(startColour & 0xff) + fraction * (float)((endColour & 0xff) - (startColour & 0xff)));
    return (Mth::clamp(255, 0, 255) << 24) | (Mth::clamp(red, 0, 255) << 16) | (Mth::clamp(green, 0, 255) << 8)
           | Mth::clamp(blue, 0, 255);
}
