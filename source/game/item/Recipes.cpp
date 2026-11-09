#include "game/item/Recipes.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif

#include <cwchar>
#include <string>
#include <utility>
#include <boost/unordered_map.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/ShulkerBoxColoring.hpp"
#include "game/item/ArmorDyeRecipe.hpp"
#include "game/item/BookCloningRecipe.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapCloningRecipe.hpp"
#include "game/item/RepairItemRecipe.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/core/FoodRecipies.hpp"
#include "game/core/OreRecipies.hpp"
#include "game/core/ToolRecipies.hpp"
#include "game/core/WeaponRecipies.hpp"
#include "game/entity/projectile/TippedArrowRecipe.hpp"
#include "game/item/ArmorRecipes.hpp"
#include "game/item/BannerRecipes.hpp"
#include "game/item/ClothDyeRecipes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ShapedRecipe.hpp"
#include "game/item/ShapelessRecipe.hpp"
#include "game/item/ShulkerBoxRecipes.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/StructureRecipies.hpp"

Recipes* Recipes::mInstance = NULL;
ToolRecipies* Recipes::mToolRecipies = NULL;
WeaponRecipies* Recipes::mWeaponRecipies = NULL;
OreRecipies* Recipes::mOreRecipies = NULL;
FoodRecipies* Recipes::mFoodRecipies = NULL;
StructureRecipies* Recipes::mStructureRecipies = NULL;
ArmorRecipes* Recipes::mArmorRecipes = NULL;
ClothDyeRecipes* Recipes::mClothDyeRecipes = NULL;
BannerRecipes* Recipes::mBannerRecipes = NULL;
ShulkerBoxRecipes* Recipes::mShulkerBoxRecipes = NULL;
ShulkerBoxColoring* Recipes::mShulkerBoxColoring = NULL;
FireworksRecipe* Recipes::mFireworksRecipe = NULL;
RepairItemRecipe* Recipes::mRepairItemRecipe = NULL;
BookCloningRecipe* Recipes::mBookCloningRecipe = NULL;
TippedArrowRecipe* Recipes::mTippedArrowRecipe = NULL;
ArmorDyeRecipe* Recipes::mArmorDyeRecipe = NULL;

Recipes::Recipes()
{
    mToolRecipies = new ToolRecipies();
    mWeaponRecipies = new WeaponRecipies();
    mOreRecipies = new OreRecipies();
    mFoodRecipies = new FoodRecipies;
    mStructureRecipies = new StructureRecipies;
    mArmorRecipes = new ArmorRecipes();
    mClothDyeRecipes = new ClothDyeRecipes();
    mBannerRecipes = new BannerRecipes;
    mShulkerBoxRecipes = new ShulkerBoxRecipes();
    mArmorDyeRecipe = new ArmorDyeRecipe;
    mRecipes[eRecipeCategory_Crafting].push_back(new MapCloningRecipe());
    mFireworksRecipe = new FireworksRecipe();
    mRepairItemRecipe = new RepairItemRecipe();
    mBookCloningRecipe = new BookCloningRecipe();
    mTippedArrowRecipe = new TippedArrowRecipe();
    mShulkerBoxColoring = new ShulkerBoxColoring();
    mRecipes[eRecipeCategory_Crafting].push_back(mFireworksRecipe);
    mRecipes[eRecipeCategory_Crafting].push_back(mRepairItemRecipe);
    mRecipes[eRecipeCategory_Crafting].push_back(mTippedArrowRecipe);
    mRecipes[eRecipeCategory_Crafting].push_back(mBookCloningRecipe);
    mRecipes[eRecipeCategory_Crafting].push_back(mArmorDyeRecipe);
    mRecipes[eRecipeCategory_Crafting].push_back(mShulkerBoxColoring);

    addShapedRecipe(new ItemInstance(Blocks::planks, 4, 0), L"sczg", L"#", L'#', new ItemInstance(Blocks::log, 1, 0),
                    L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::BIRCH->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::log, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::SPRUCE->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::log, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::JUNGLE->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::log, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::ACACIA->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::log2, 1, PlanksBlock::Variant::ACACIA->getData() - 4), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::DARK_OAK->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::log2, 1, PlanksBlock::Variant::DARK_OAK->getData() - 4), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, 0), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::stripped_log_oak, 1, 0), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::BIRCH->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::stripped_log_birch, 1), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::SPRUCE->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::stripped_log_spruce, 1), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::JUNGLE->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::stripped_log_jungle, 1), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::ACACIA->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::stripped_log_acacia, 1), L'S');
    addShapedRecipe(new ItemInstance(Blocks::planks, 4, PlanksBlock::Variant::DARK_OAK->getData()), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::stripped_log_dark_oak, 1), L'S');
    addShapedRecipe(new ItemInstance(Items::stick, 4), L"ssctg", L"#", L"#", L'#', Blocks::planks, L'S');
    mToolRecipies->addRecipes(this);
    mFoodRecipies->addRecipes(this);
    addShapedRecipe(new ItemInstance(Blocks::quartz_block, 1), L"sscig", L"##", L"##", L'#', Items::quartz, L'S');
    mStructureRecipies->addRecipes(this);
    mShulkerBoxRecipes->addRecipes(this);
    for (int colour = 0; colour < 16; ++colour)
    {
        addShapedRecipe(new ItemInstance(Items::bed, 1, colour), L"ssczctg", L"###", L"XXX", L'#',
                        new ItemInstance(Blocks::wool, 1, colour), L'X', Blocks::planks, L'S');
    }
    addShapedRecipe(new ItemInstance(Blocks::enchanting_table, 1), L"sssctcicig", L" B ", L"D#D", L"###", L'#',
                    Blocks::obsidian, L'B', Items::book, L'D', Items::diamond, L'S');
    addShapedRecipe(new ItemInstance(Blocks::anvil, 1), L"sssctcig", L"III", L" i ", L"iii", L'I', Blocks::iron_block,
                    L'i', Items::ironIngot, L'S');
    addShapedRecipe(new ItemInstance(Blocks::ladder, 3), L"ssscig", L"# #", L"###", L"# #", L'#', Items::stick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::fence, 3), L"ssciczg", L"W#W", L"W#W", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::birch_fence, 3), L"ssciczg", L"W#W", L"W#W", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::spruce_fence, 3), L"ssciczg", L"W#W", L"W#W", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::jungle_fence, 3), L"ssciczg", L"W#W", L"W#W", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::acacia_fence, 3), L"ssciczg", L"W#W", L"W#W", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::dark_oak_fence, 3), L"ssciczg", L"W#W", L"W#W", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::cobblestone_wall, 6, WallBlock::Variant::COBBLESTONE->getData()), L"ssctg",
                    L"###", L"###", L'#', Blocks::cobblestone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::cobblestone_wall, 6, WallBlock::Variant::MOSSY->getData()), L"ssctg",
                    L"###", L"###", L'#', Blocks::mossy_cobblestone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::nether_brick_fence, 6), L"ssctg", L"###", L"###", L'#',
                    Blocks::nether_brick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::iron_bars, 16), L"sscig", L"###", L"###", L'#', Items::ironIngot, L'S');
    addShapedRecipe(new ItemInstance(Blocks::fence_gate, 1), L"ssciczg", L"#W#", L"#W#", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::birch_fence_gate, 1), L"ssciczg", L"#W#", L"#W#", L'#', Items::stick, L'W',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::spruce_fence_gate, 1), L"ssciczg", L"#W#", L"#W#", L'#', Items::stick,
                    L'W', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::jungle_fence_gate, 1), L"ssciczg", L"#W#", L"#W#", L'#', Items::stick,
                    L'W', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::acacia_fence_gate, 1), L"ssciczg", L"#W#", L"#W#", L'#', Items::stick,
                    L'W', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::dark_oak_fence_gate, 1), L"ssciczg", L"#W#", L"#W#", L'#', Items::stick,
                    L'W', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::woodenDoor, 3), L"sssczg", L"##", L"##", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::birchDoor, 3), L"sssczg", L"##", L"##", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::spruceDoor, 3), L"sssczg", L"##", L"##", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::jungleDoor, 3), L"sssczg", L"##", L"##", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::acaciaDoor, 3), L"sssczg", L"##", L"##", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::darkOakDoor, 3), L"sssczg", L"##", L"##", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::ironDoor, 3), L"ssscig", L"##", L"##", L"##", L'#', Items::ironIngot, L'S');
    addShapedRecipe(new ItemInstance(Blocks::trapdoor, 2), L"ssczg", L"###", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::spruce_trapdoor, 2), L"ssczg", L"###", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::birch_trapdoor, 2), L"ssczg", L"###", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::jungle_trapdoor, 2), L"ssczg", L"###", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::acacia_trapdoor, 2), L"ssczg", L"###", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::dark_oak_trapdoor, 2), L"ssczg", L"###", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::iron_trapdoor, 1), L"sscig", L"##", L"##", L'#', Items::ironIngot, L'S');
    addShapedRecipe(new ItemInstance(Blocks::oak_stairs, 4), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, 0), L'S');
    addShapedRecipe(new ItemInstance(Blocks::birch_stairs, 4), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::spruce_stairs, 4), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::jungle_stairs, 4), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::acacia_stairs, 4), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::dark_oak_stairs, 4), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::cobblestone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::brick_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::brick_block, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_brick_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::stonebrick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::nether_brick_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::nether_brick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::sandstone_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::sandstone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::red_sandstone_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::red_sandstone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::quartz_stairs, 4), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::quartz_block, L'S');
    addShapedRecipe(new ItemInstance(Blocks::purpur_stairs, 4, 0), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::purpur_block, L'S');
    addShapedRecipe(new ItemInstance(Blocks::purpur_stairs, 4, 0), L"sssctg", L"#  ", L"## ", L"###", L'#',
                    Blocks::purpur_pillar, L'S');
    addShapedRecipe(new ItemInstance(Blocks::prismarine_stairs, 4, 0), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::Variant::ROUGH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::prismarine_bricks_stairs, 4, 0), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::Variant::BRICKS->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::dark_prismarine_stairs, 4, 0), L"sssczg", L"#  ", L"## ", L"###", L'#',
                    new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::Variant::DARK->getData()), L'S');
    mArmorRecipes->addRecipes(this);
    mClothDyeRecipes->addRecipes(this);
    addShapedRecipe(new ItemInstance(Blocks::snow, 1), L"sscig", L"##", L"##", L'#', Items::snowball, L'S');
    addShapedRecipe(new ItemInstance(Blocks::snow_layer, 6), L"sctg", L"###", L'#', Blocks::snow, L'S');
    addShapedRecipe(new ItemInstance(Blocks::clay, 1), L"sscig", L"##", L"##", L'#', Items::clayBall, L'S');
    addShapedRecipe(new ItemInstance(Blocks::brick_block, 1), L"sscig", L"##", L"##", L'#', Items::brick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::wool, 1), L"sscig", L"##", L"##", L'#', Items::string, L'D');
    addShapedRecipe(new ItemInstance(Blocks::tnt, 1), L"ssscictg", L"X#X", L"#X#", L"X#X", L'X', Items::gunpowder, L'#',
                    Blocks::sand, L'T');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::COBBLESTONE->getData()), L"sctg",
                    L"###", L'#', Blocks::cobblestone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::STONE->getData()), L"sctg", L"###",
                    L'#', Blocks::stone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::BRICK->getData()), L"sctg", L"###",
                    L'#', Blocks::brick_block, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::SMOOTHBRICK->getData()), L"sctg",
                    L"###", L'#', Blocks::stonebrick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::NETHERBRICK->getData()), L"sctg",
                    L"###", L'#', Blocks::nether_brick, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::SAND->getData()), L"sctg", L"###",
                    L'#', Blocks::sandstone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab2, 6, NewStoneSlabBlock::Variant::RED_SANDSTONE->getData()),
                    L"sctg", L"###", L'#', Blocks::red_sandstone, L'S');
    addShapedRecipe(new ItemInstance(Blocks::stone_slab, 6, StoneSlabBlock::Variant::QUARTZ->getData()), L"sctg",
                    L"###", L'#', Blocks::quartz_block, L'S');
    addShapedRecipe(new ItemInstance(Blocks::purpur_slab, 6, 0), L"sctg", L"###", L'#', Blocks::purpur_block, L'S');
    addShapedRecipe(new ItemInstance(Blocks::purpur_slab, 6, 0), L"sctg", L"###", L'#', Blocks::purpur_pillar, L'S');
    addShapedRecipe(new ItemInstance(Blocks::prismarine_slab, 6, PrismarineBlock::Variant::ROUGH->getData()), L"sczg",
                    L"###", L'#', new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::Variant::ROUGH->getData()),
                    L'S');
    addShapedRecipe(new ItemInstance(Blocks::prismarine_slab, 6, PrismarineBlock::Variant::BRICKS->getData()), L"sczg",
                    L"###", L'#', new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::Variant::BRICKS->getData()),
                    L'S');
    addShapedRecipe(new ItemInstance(Blocks::prismarine_slab, 6, PrismarineBlock::Variant::DARK->getData()), L"sczg",
                    L"###", L'#', new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::Variant::DARK->getData()),
                    L'S');
    addShapedRecipe(new ItemInstance(Blocks::wooden_slab, 6, 0), L"sczg", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::wooden_slab, 6, PlanksBlock::Variant::BIRCH->getData()), L"sczg", L"###",
                    L'#', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::wooden_slab, 6, PlanksBlock::Variant::SPRUCE->getData()), L"sczg", L"###",
                    L'#', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::wooden_slab, 6, PlanksBlock::Variant::JUNGLE->getData()), L"sczg", L"###",
                    L'#', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::wooden_slab, 6, PlanksBlock::Variant::ACACIA->getData()), L"sczg", L"###",
                    L'#', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'S');
    addShapedRecipe(new ItemInstance(Blocks::wooden_slab, 6, PlanksBlock::Variant::DARK_OAK->getData()), L"sczg",
                    L"###", L'#', new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'S');
    addShapedRecipe(new ItemInstance(Items::cake, 1), L"ssscicicicig", L"AAA", L"BEB", L"CCC", L'A', Items::milkBucket,
                    L'B', Items::sugar, L'C', Items::wheat, L'E', Items::egg, L'F');
    addShapedRecipe(new ItemInstance(Items::sugar, 1), L"scig", L"#", L'#', Items::reeds, L'F');
    addShapedRecipe(new ItemInstance(Blocks::rail, 16), L"ssscicig", L"X X", L"X#X", L"X X", L'X', Items::ironIngot,
                    L'#', Items::stick, L'V');
    addShapedRecipe(new ItemInstance(Blocks::golden_rail, 6), L"ssscicicig", L"X X", L"X#X", L"XRX", L'X',
                    Items::goldIngot, L'R', Items::redstone, L'#', Items::stick, L'V');
    addShapedRecipe(new ItemInstance(Blocks::activator_rail, 6), L"ssscictcig", L"XSX", L"X#X", L"XSX", L'X',
                    Items::ironIngot, L'#', Blocks::redstone_torch, L'S', Items::stick, L'V');
    addShapedRecipe(new ItemInstance(Blocks::detector_rail, 6), L"ssscicictg", L"X X", L"X#X", L"XRX", L'X',
                    Items::ironIngot, L'R', Items::redstone, L'#', Blocks::stone_pressure_plate, L'V');
    addShapedRecipe(new ItemInstance(Items::minecart, 1), L"sscig", L"# #", L"###", L'#', Items::ironIngot, L'V');
    addShapedRecipe(new ItemInstance(Items::chestMinecart, 1), L"ssctcig", L"A", L"B", L'A', Blocks::chest, L'B',
                    Items::minecart, L'V');
    addShapedRecipe(new ItemInstance(Items::furnaceMinecart, 1), L"ssctcig", L"A", L"B", L'A', Blocks::furnace, L'B',
                    Items::minecart, L'V');
    addShapedRecipe(new ItemInstance(Items::tntMinecart, 1), L"ssctcig", L"A", L"B", L'A', Blocks::tnt, L'B',
                    Items::minecart, L'V');
    addShapedRecipe(new ItemInstance(Items::hopperMinecart, 1), L"ssctcig", L"A", L"B", L'A', Blocks::hopper, L'B',
                    Items::minecart, L'V');
    addShapedRecipe(new ItemInstance(Items::boat, 1), L"ssczcig", L"#X#", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'X',
                    Items::woodenShovel, L'V');
    addShapedRecipe(new ItemInstance(Items::spruceBoat, 1), L"ssczcig", L"#X#", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'X',
                    Items::woodenShovel, L'V');
    addShapedRecipe(new ItemInstance(Items::birchBoat, 1), L"ssczcig", L"#X#", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'X',
                    Items::woodenShovel, L'V');
    addShapedRecipe(new ItemInstance(Items::jungleBoat, 1), L"ssczcig", L"#X#", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'X',
                    Items::woodenShovel, L'V');
    addShapedRecipe(new ItemInstance(Items::acaciaBoat, 1), L"ssczcig", L"#X#", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'X',
                    Items::woodenShovel, L'V');
    addShapedRecipe(new ItemInstance(Items::darkOakBoat, 1), L"ssczcig", L"#X#", L"###", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'X',
                    Items::woodenShovel, L'V');
    addShapedRecipe(new ItemInstance(Items::fishingRod, 1), L"ssscicig", L"  #", L" #X", L"# X", L'#', Items::stick,
                    L'X', Items::string, L'T');
    addShapedRecipe(new ItemInstance(Items::carrotOnAStick, 1), L"sscicig", L"# ", L" X", L'#', Items::fishingRod, L'X',
                    Items::carrot, L'T');
    addShapelessRecipe(new ItemInstance(Items::flintAndSteel, 1), L"zzg", new ItemInstance(Items::ironIngot, 1),
                       new ItemInstance(Items::flint, 1), L'T');
    addShapedRecipe(new ItemInstance(Items::bread, 1), L"scig", L"###", L'#', Items::wheat, L'F');
    addShapedRecipe(new ItemInstance(Items::bow, 1), L"ssscicig", L" #X", L"# X", L" #X", L'X', Items::string, L'#',
                    Items::stick, L'T');
    addShapedRecipe(new ItemInstance(Items::arrow, 4), L"ssscicicig", L"X", L"#", L"Y", L'Y', Items::feather, L'X',
                    Items::flint, L'#', Items::stick, L'T');
    mTippedArrowRecipe->addRecipes(this);
    mWeaponRecipies->addRecipes(this);
    addShapedRecipe(new ItemInstance(Items::bucket, 1), L"sscig", L"# #", L" # ", L'#', Items::ironIngot, L'T');
    addShapedRecipe(new ItemInstance(Items::bowl, 4), L"ssctg", L"# #", L" # ", L'#', Blocks::planks, L'T');
    addShapedRecipe(new ItemInstance(Items::glassBottle, 3), L"ssctg", L"# #", L" # ", L'#', Blocks::glass, L'T');
    addShapedRecipe(new ItemInstance(Items::flowerPot, 1), L"sscig", L"# #", L" # ", L'#', Items::brick, L'D');
    addShapedRecipe(new ItemInstance(Blocks::torch, 4), L"ssczcig", L"X", L"#", L'X',
                    new ItemInstance(Items::coal, 1, 1), L'#', Items::stick, L'T');
    addShapedRecipe(new ItemInstance(Blocks::torch, 4), L"ssczcig", L"X", L"#", L'X',
                    new ItemInstance(Items::coal, 1, 0), L'#', Items::stick, L'T');
    addShapedRecipe(new ItemInstance(Blocks::glowstone, 1), L"sscig", L"##", L"##", L'#', Items::glowstoneDust, L'T');
    addShapedRecipe(new ItemInstance(Blocks::sea_lantern, 1, 0), L"ssscicig", L"SCS", L"CCC", L"SCS", L'S',
                    Items::prismarineShard, L'C', Items::prismarineCrystals, L'T');
    addShapedRecipe(new ItemInstance(Blocks::lever, 1), L"ssctcig", L"X", L"#", L'#', Blocks::cobblestone, L'X',
                    Items::stick, L'M');
    addShapedRecipe(new ItemInstance(Blocks::tripwire_hook, 2), L"sssctcicig", L"I", L"S", L"#", L'#', Blocks::planks,
                    L'S', Items::stick, L'I', Items::ironIngot, L'M');
    addShapedRecipe(new ItemInstance(Blocks::redstone_torch, 1), L"sscicig", L"X", L"#", L'#', Items::stick, L'X',
                    Items::redstone, L'M');
    addShapedRecipe(new ItemInstance(Items::repeater, 1), L"ssctcictg", L"#X#", L"III", L'#', Blocks::redstone_torch,
                    L'X', Items::redstone, L'I', Blocks::stone, L'M');
    addShapedRecipe(new ItemInstance(Items::comparator, 1), L"sssctcictg", L" # ", L"#X#", L"III", L'#',
                    Blocks::redstone_torch, L'X', Items::quartz, L'I', Blocks::stone, L'M');
    addShapedRecipe(new ItemInstance(Blocks::daylight_detector), L"sssctcictg", L"GGG", L"QQQ", L"WWW", L'G',
                    Blocks::glass, L'Q', Items::quartz, L'W', Blocks::wooden_slab, L'M');
    addShapedRecipe(new ItemInstance(Items::endCrystal), L"sssctcicig", L"GGG", L"GEG", L"GTG", L'G', Blocks::glass,
                    L'E', Items::enderEye, L'T', Items::ghastTear, L'M');
    addShapedRecipe(new ItemInstance(Blocks::hopper), L"ssscictg", L"I I", L"ICI", L" I ", L'I', Items::ironIngot, L'C',
                    Blocks::chest, L'M');
    addShapedRecipe(new ItemInstance(Items::clock, 1), L"ssscicig", L" # ", L"#X#", L" # ", L'#', Items::goldIngot,
                    L'X', Items::redstone, L'T');
    addShapelessRecipe(new ItemInstance(Items::enderEye, 1), L"iig", Items::enderPearl, Items::blazePowder, L'T');
    addShapelessRecipe(new ItemInstance(Items::fireCharge, 3), L"iiig", Items::gunpowder, Items::blazePowder,
                       Items::coal, L'T');
    addShapelessRecipe(new ItemInstance(Items::fireCharge, 3), L"iizg", Items::gunpowder, Items::blazePowder,
                       new ItemInstance(Items::coal, 1, 1), L'T');
    addShapedRecipe(new ItemInstance(Items::armorStand, 1), L"sssciczg", L"III", L" I ", L"ICI", L'I', Items::stick,
                    L'C', new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::STONE->getData()), L'D');
    addShapedRecipe(new ItemInstance(Blocks::end_rod, 4), L"sscicig", L"/", L"#", L'/', Items::blazeRod, L'#',
                    Items::chorusFruitPopped, L'D');
    addShapedRecipe(new ItemInstance(Blocks::bone_block, 1), L"sssczg", L"XXX", L"XXX", L"XXX", L'X',
                    new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'S');
    addShapedRecipe(new ItemInstance(Items::lead, 2), L"ssscicig", L"~~ ", L"~O ", L"  ~", L'~', Items::string, L'O',
                    Items::slimeBall, L'T');
    addShapedRecipe(new ItemInstance(Items::compass, 1), L"ssscicig", L" # ", L"#X#", L" # ", L'#', Items::ironIngot,
                    L'X', Items::redstone, L'T');
    addShapedRecipe(new ItemInstance(Items::map, 1), L"ssscicig", L"###", L"#X#", L"###", L'#', Items::paper, L'X',
                    Items::compass, L'T');
    addShapedRecipe(new ItemInstance(Blocks::stone_button, 1), L"sczg", L"#", L'#', new ItemInstance(Blocks::stone, 1),
                    L'M');
    addShapedRecipe(new ItemInstance(Blocks::wooden_button, 1), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::acacia_button, 1), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::birch_button, 1), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::dark_oak_button, 1), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::jungle_button, 1), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::spruce_button, 1), L"sczg", L"#", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::wooden_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::acacia_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::birch_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::dark_oak_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::jungle_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::spruce_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::stone_pressure_plate, 1), L"sczg", L"##", L'#',
                    new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::STONE->getData()), L'M');
    addShapedRecipe(new ItemInstance(Blocks::heavy_weighted_pressure_plate, 1), L"scig", L"##", L'#', Items::ironIngot,
                    L'M');
    addShapedRecipe(new ItemInstance(Blocks::light_weighted_pressure_plate, 1), L"scig", L"##", L'#', Items::goldIngot,
                    L'M');
    addShapedRecipe(new ItemInstance(Blocks::dispenser, 1), L"sssctcicig", L"###", L"#X#", L"#R#", L'#',
                    Blocks::cobblestone, L'X', Items::bow, L'R', Items::redstone, L'M');
    addShapedRecipe(new ItemInstance(Blocks::dropper, 1), L"sssctcig", L"###", L"# #", L"#R#", L'#',
                    Blocks::cobblestone, L'R', Items::redstone, L'M');
    addShapedRecipe(new ItemInstance(Blocks::observer, 1), L"sssctcicig", L"###", L"RRQ", L"###", L'#',
                    Blocks::cobblestone, L'R', Items::redstone, L'Q', Items::quartz, L'M');
    addShapedRecipe(new ItemInstance(Items::cauldron, 1), L"ssscig", L"# #", L"# #", L"###", L'#', Items::ironIngot,
                    L'T');
    addShapedRecipe(new ItemInstance(Items::brewingStand, 1), L"ssctcig", L" B ", L"###", L'#', Blocks::cobblestone,
                    L'B', Items::blazeRod, L'S');
    addShapedRecipe(new ItemInstance(Blocks::lit_pumpkin, 1), L"ssctctg", L"A", L"B", L'A', Blocks::carved_pumpkin,
                    L'B', Blocks::torch, L'T');
    addShapedRecipe(new ItemInstance(Blocks::jukebox, 1), L"sssctcig", L"###", L"#X#", L"###", L'#', Blocks::planks,
                    L'X', Items::diamond, L'D');
    addShapedRecipe(new ItemInstance(Items::paper, 3), L"scig", L"###", L'#', Items::reeds, L'D');
    addShapelessRecipe(new ItemInstance(Items::book, 1), L"iiiig", Items::paper, Items::paper, Items::paper,
                       Items::leather, L'D');
    addShapelessRecipe(new ItemInstance(Items::leather, 1), L"iiiig", Items::rabbitHide, Items::rabbitHide,
                       Items::rabbitHide, Items::rabbitHide, L'D');
    addShapelessRecipe(new ItemInstance(Items::writableBook, 1), L"izig", Items::book,
                       new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()), Items::feather, L'D');
    addShapedRecipe(new ItemInstance(Blocks::noteblock, 1), L"sssctcig", L"###", L"#X#", L"###", L'#', Blocks::planks,
                    L'X', Items::redstone, L'M');
    addShapedRecipe(new ItemInstance(Blocks::bookshelf, 1), L"sssctcig", L"###", L"XXX", L"###", L'#', Blocks::planks,
                    L'X', Items::book, L'D');
    addShapedRecipe(new ItemInstance(Items::painting, 1), L"ssscictg", L"###", L"#X#", L"###", L'#', Items::stick, L'X',
                    Blocks::wool, L'D');
    addShapedRecipe(new ItemInstance(Items::itemFrame, 1), L"ssscicig", L"###", L"#X#", L"###", L'#', Items::stick,
                    L'X', Items::leather, L'D');
    addShapedRecipe(new ItemInstance(Blocks::dried_kelp_block, 1), L"ssscig", L"###", L"###", L"###", L'#',
                    Items::driedKelp, L'D');
    addShapedRecipe(new ItemInstance(Items::driedKelp, 9), L"sctg", L"#", L'#', Blocks::dried_kelp_block, L'D');
    addShapedRecipe(new ItemInstance(Blocks::blue_ice, 1), L"sssctg", L"###", L"###", L"###", L'#', Blocks::packed_ice,
                    L'D');
    addShapedRecipe(new ItemInstance(Blocks::conduit, 1), L"ssscicig", L"###", L"#X#", L"###", L'#', Items::nautilus,
                    L'X', Items::nautilusCore, L'M');
    mOreRecipies->addRecipes(this);
    addShapedRecipe(new ItemInstance(Items::sign, 3), L"sssctcig", L"###", L"###", L" X ", L'#', Blocks::planks, L'X',
                    Items::stick, L'D');
    addShapedRecipe(new ItemInstance(Blocks::piston, 1), L"sssctcicictg", L"TTT", L"#X#", L"#R#", L'#',
                    Blocks::cobblestone, L'X', Items::ironIngot, L'R', Items::redstone, L'T', Blocks::planks, L'M');
    addShapedRecipe(new ItemInstance(Blocks::sticky_piston, 1), L"sscictg", L"S", L"P", L'S', Items::slimeBall, L'P',
                    Blocks::piston, L'M');
    addShapedRecipe(new ItemInstance(Items::fireworkCharge, 1), 2, L"ssczcig", L" D", L" G", L'D',
                    new ItemInstance(Items::dye, 1, -1), L'G', Items::gunpowder, L'S');
    addShapedRecipe(new ItemInstance(Items::fireworkCharge, 1), 2, L"ssczcig", L" D", L" C", L'D',
                    new ItemInstance(Items::dye, 1, -1), L'C', Items::fireworkCharge, L'S');
    addShapedRecipe(new ItemInstance(Items::fireworks, 3), 2, L"sscicig", L" P", L" G", L'P', Items::paper, L'G',
                    Items::gunpowder, L'S');
    addShapedRecipe(new ItemInstance(Items::leatherHelmet, 1), eRecipeCategory_Dyeing, L"ssczcig", L" D", L" G", L'D',
                    new ItemInstance(Items::dye, 1, -1), L'G', Items::leatherHelmet, L'0');
    addShapedRecipe(new ItemInstance(Items::leatherChestplate, 1), eRecipeCategory_Dyeing, L"ssczcig", L" D", L" G",
                    L'D', new ItemInstance(Items::dye, 1, -1), L'G', Items::leatherChestplate, L'0');
    addShapedRecipe(new ItemInstance(Items::leatherLeggings, 1), eRecipeCategory_Dyeing, L"ssczcig", L" D", L" G", L'D',
                    new ItemInstance(Items::dye, 1, -1), L'G', Items::leatherLeggings, L'0');
    addShapedRecipe(new ItemInstance(Items::leatherBoots, 1), eRecipeCategory_Dyeing, L"ssczcig", L" D", L" G", L'D',
                    new ItemInstance(Items::dye, 1, -1), L'G', Items::leatherBoots, L'0');
    addShapedRecipe(new ItemInstance(Items::leatherHorseArmor, 1), eRecipeCategory_Dyeing, L"ssczcig", L" D", L" G",
                    L'D', new ItemInstance(Items::dye, 1, -1), L'G', Items::leatherHorseArmor, L'0');
    for (int colour = 0; colour < 16; ++colour)
    {
        addShapedRecipe(new ItemInstance(Items::bed, 1, colour), eRecipeCategory_Dyeing, L"ssczczg", L" D", L" G", L'D',
                        new ItemInstance(Items::dye, 1, 15 - colour), L'G', new ItemInstance(Items::bed, 1, -1), L'0');
    }
    Block* shulkerBoxes[16] = {Blocks::white_shulker_box,      Blocks::orange_shulker_box, Blocks::magenta_shulker_box,
                               Blocks::light_blue_shulker_box, Blocks::yellow_shulker_box, Blocks::lime_shulker_box,
                               Blocks::pink_shulker_box,       Blocks::gray_shulker_box,   Blocks::silver_shulker_box,
                               Blocks::cyan_shulker_box,       Blocks::purple_shulker_box, Blocks::blue_shulker_box,
                               Blocks::brown_shulker_box,      Blocks::green_shulker_box,  Blocks::red_shulker_box,
                               Blocks::black_shulker_box};
    for (int colour = 0; colour < 16; ++colour)
    {
        addShapedRecipe(new ItemInstance(shulkerBoxes[colour], 1, colour), eRecipeCategory_Dyeing, L"ssczczg", L" D",
                        L" G", L'D', new ItemInstance(Items::dye, 1, 15 - colour), L'G',
                        new ItemInstance(Blocks::purple_shulker_box, 1, -1), L'0');
    }
    mBannerRecipes->addRecipes(this);
#ifdef MINECRAFT_PC
    PcModContentAddRecipes(this);
#endif
    buildRecipeIngredientsArray();
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, int category, va_list arguments)
{
    std::wstring pattern(L"");
    std::wstring row;
    int width = 0;
    int height = 0;
    wchar_t ingredientKey = 0;
    int achievementId = -1;
    Recipe::_eGroupType group = Recipe::eGroupType_Miscellaneous;
    int itemId = Item::getId(result->getItem());
    int baseItemType;

    if (static_cast<unsigned int>(itemId - 256) < 768)
    {
        baseItemType = result->getItem()->getBaseItemType();
    }
    else
    {
        baseItemType = Block::byItem(result->getItem())->getBaseItemType();
    }

    boost::unordered_map<wchar_t, ItemInstance*> ingredientMap;
    const wchar_t* format = va_arg(arguments, const wchar_t*);
    for (const wchar_t* type = format; *type != 0; ++type)
    {
        switch (*type)
        {
        case L'a':
        case L's':
            row = va_arg(arguments, const wchar_t*);
            width = static_cast<int>(row.size());
            ++height;
            pattern.append(row);
            break;
        case L'b':
            baseItemType = va_arg(arguments, int);
            break;
        case L'c':
            ingredientKey = static_cast<wchar_t>(va_arg(arguments, int));
            break;
        case L'g':
            ingredientKey = static_cast<wchar_t>(va_arg(arguments, int));
            if (ingredientKey >= L'0' && ingredientKey <= L'5')
            {
                group = static_cast<Recipe::_eGroupType>(ingredientKey - L'0');
            }
            else
            {
                switch (ingredientKey)
                {
                case L'S':
                    group = Recipe::eGroupType_Structures;
                    break;
                case L'T':
                    group = Recipe::eGroupType_Tools;
                    break;
                case L'F':
                    group = Recipe::eGroupType_Food;
                    break;
                case L'A':
                    group = Recipe::eGroupType_Armour;
                    break;
                case L'M':
                    group = Recipe::eGroupType_Mechanisms;
                    break;
                case L'V':
                    group = Recipe::eGroupType_Vehicles;
                    break;
                case L'X':
                    group = Recipe::eGroupType_Excluded;
                    break;
                default:
                    group = Recipe::eGroupType_Miscellaneous;
                    break;
                }
            }
            break;
        case L'i':
            ingredientMap.insert(std::make_pair(ingredientKey, new ItemInstance(va_arg(arguments, Item*))));
            break;
        case L't':
            ingredientMap.insert(std::make_pair(ingredientKey, new ItemInstance(va_arg(arguments, Block*), 1, -1)));
            break;
        case L'v':
            achievementId = va_arg(arguments, int);
            break;
        case L'w':
        {
            std::wstring* rows = va_arg(arguments, std::wstring*);
            while (true)
            {
                row = *rows;
                ++rows;
                if (row.empty())
                {
                    break;
                }
                ++height;
                pattern.append(row);
                width = static_cast<int>(row.size());
            }
            break;
        }
        case L'z':
            ingredientMap.insert(std::make_pair(ingredientKey, va_arg(arguments, ItemInstance*)));
            break;
        default:
            break;
        }
    }

    int ingredientCount = width * height;
    ItemInstance** ingredients = new ItemInstance*[ingredientCount];
    for (int index = 0; index < ingredientCount; ++index)
    {
        boost::unordered_map<wchar_t, ItemInstance*>::iterator ingredient = ingredientMap.find(pattern[index]);
        ingredients[index] = ingredient == ingredientMap.end() ? ItemInstance::EMPTY.get() : ingredient->second;
    }

    ShapedRecipe* recipe = new ShapedRecipe(width, height, ingredients, result, group);
    recipe->mAchievementId = achievementId;
    recipe->mBaseItemType = baseItemType;
    mRecipes[category].push_back(recipe);
    return recipe;
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, ...)
{
    va_list arguments;
    va_start(arguments, result);
    Recipe* recipe = addShapedRecipe(result, 0, arguments);
    va_end(arguments);
    return recipe;
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, int category, ...)
{
    va_list arguments;
    va_start(arguments, category);
    Recipe* recipe = addShapedRecipe(result, category, arguments);
    va_end(arguments);
    return recipe;
}

void Recipes::addShapelessRecipe(ItemInstance* result, int category, va_list arguments)
{
    Recipe::_eGroupType group = Recipe::eGroupType_Miscellaneous;
    std::vector<ItemInstance*>* ingredients = new std::vector<ItemInstance*>();
    int itemId = Item::getId(result->getItem());
    int baseItemType;

    if (itemId < 256)
    {
        baseItemType = Block::byItem(result->getItem())->getBaseItemType();
    }
    else
    {
        baseItemType = result->getItem()->getBaseItemType();
    }
    int achievementId = -1;

    const wchar_t* format = va_arg(arguments, const wchar_t*);
    for (const wchar_t* type = format; *type != 0; ++type)
    {
        switch (*type)
        {
        case L'z':
            ingredients->push_back(va_arg(arguments, ItemInstance*)->copy_not_shared());
            break;
        case L'i':
            ingredients->push_back(new ItemInstance(va_arg(arguments, Item*)));
            break;
        case L't':
            ingredients->push_back(new ItemInstance(va_arg(arguments, Block*), 1, -1));
            break;
        case L'b':
            baseItemType = va_arg(arguments, int);
            break;
        case L'v':
            achievementId = va_arg(arguments, int);
            break;
        case L'g':
        {
            wchar_t groupCode = static_cast<wchar_t>(va_arg(arguments, int));
            if (groupCode >= L'0' && groupCode <= L'5')
            {
                group = static_cast<Recipe::_eGroupType>(groupCode - L'0');
            }
            else
            {
                switch (groupCode)
                {
                case L'S':
                    group = Recipe::eGroupType_Structures;
                    break;
                case L'T':
                    group = Recipe::eGroupType_Tools;
                    break;
                case L'F':
                    group = Recipe::eGroupType_Food;
                    break;
                case L'A':
                    group = Recipe::eGroupType_Armour;
                    break;
                case L'M':
                    group = Recipe::eGroupType_Mechanisms;
                    break;
                case L'V':
                    group = Recipe::eGroupType_Vehicles;
                    break;
                case L'X':
                    group = Recipe::eGroupType_Excluded;
                    break;
                default:
                    group = Recipe::eGroupType_Miscellaneous;
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    ShapelessRecipe* recipe = new ShapelessRecipe(result, ingredients, group);
    recipe->mAchievementId = achievementId;
    recipe->mBaseItemType = baseItemType;
    mRecipes[category].push_back(recipe);
}

void Recipes::addShapelessRecipe(ItemInstance* result, ...)
{
    va_list arguments;
    va_start(arguments, result);
    addShapelessRecipe(result, 0, arguments);
    va_end(arguments);
}

void Recipes::addShapelessRecipe(ItemInstance* result, int category, ...)
{
    va_list arguments;
    va_start(arguments, category);
    addShapelessRecipe(result, category, arguments);
    va_end(arguments);
}

void Recipes::buildRecipeIngredientsArray()
{
    for (int category = 0; category < eRecipeCategory_Count; ++category)
    {
        mRecipeIngredients[category] = new Recipe::INGREDIENTS_REQUIRED[mRecipes[category].size()];
        int recipeIndex = 0;
        std::vector<Recipe*>::iterator recipesEnd = mRecipes[category].end();
        for (std::vector<Recipe*>::iterator recipe = mRecipes[category].begin(); recipe != recipesEnd; recipe++)
        {
            (*recipe)->recipeRequires(&mRecipeIngredients[category][recipeIndex++]);
        }
    }
}

void Recipes::staticCtor()
{
    mInstance = new Recipes();
}

void Recipes::addCustomRecipe(Recipe* recipe, ERecipeCategory category)
{
    mRecipes[category].push_back(recipe);
}

not_null_ptr<ItemInstance> Recipes::getItemFor(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level,
                                               Recipe* recipe, ERecipeCategory category)
{
    int itemCount = 0;
    not_null_ptr<ItemInstance> first = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> second = ItemInstance::EMPTY;
    for (unsigned int slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (!item->isEmpty())
        {
            if (itemCount == 0)
                first = item;
            if (itemCount == 1)
                second = item;
            ++itemCount;
        }
    }

    if (itemCount == 2 && first->getItem() == second->getItem() && first->getCount() == 1 && second->getCount() == 1
        && first->getItem()->canBeDepleted())
    {
        Item* item = first->getItem();
        int remaining1 = item->getMaxDamage() - first->getDamageValue();
        int remaining2 = item->getMaxDamage() - second->getDamageValue();
        int remaining = remaining1 + remaining2 + item->getMaxDamage() * 5 / 100;
        int resultDamage = item->getMaxDamage() - remaining;
        if (resultDamage < 0)
            resultDamage = 0;
        return not_null_ptr<ItemInstance>(new ItemInstance(first->getItem(), 1, resultDamage));
    }

    if (recipe != NULL)
    {
        if (recipe->matches(craftingContainer, level))
            return recipe->assemble(craftingContainer);
    }
    else
    {
        std::vector<Recipe*>::iterator end = mRecipes[category].end();
        for (std::vector<Recipe*>::iterator current = mRecipes[category].begin(); current != end; ++current)
        {
            Recipe* candidate = *current;
            if (candidate->matches(craftingContainer, level))
                return candidate->assemble(craftingContainer);
        }
    }
    return ItemInstance::EMPTY;
}

eAward Recipes::hasAchievementAttachedToRecipe(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level,
                                               Recipe* recipe, ERecipeCategory category)
{
    not_null_ptr<ItemInstance> first = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> second = ItemInstance::EMPTY;
    int itemCount = 0;
    for (unsigned int slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (!item->isEmpty())
        {
            if (itemCount == 0)
                first = item;
            if (itemCount == 1)
                second = item;
            ++itemCount;
        }
    }

    if (itemCount == 2 && first->getItem() == second->getItem() && first->getCount() == 1 && second->getCount() == 1
        && first->getItem()->canBeDepleted())
        return eAward_None;

    if (recipe == NULL)
    {
        std::vector<Recipe*>& recipes = mRecipes[category];
        for (std::vector<Recipe*>::iterator current = recipes.begin(); current != recipes.end(); ++current)
        {
            if ((*current)->matches(craftingContainer, level))
                return static_cast<eAward>((*current)->mAchievementId);
        }
    }
    else if (recipe->matches(craftingContainer, level))
    {
        return static_cast<eAward>(recipe->mAchievementId);
    }
    return eAward_None;
}

NonNullList<not_null_ptr<ItemInstance> > Recipes::getRemainingItemsFor(
    boost::shared_ptr<CraftingContainer> craftingContainer, Level* level, ERecipeCategory category)
{
    for (std::vector<Recipe*>::iterator current = mRecipes[category].begin(); current != mRecipes[category].end();
         ++current)
    {
        Recipe* recipe = *current;
        if (recipe->matches(craftingContainer, level))
            return recipe->getRemainingItems(craftingContainer);
    }

    NonNullList<not_null_ptr<ItemInstance> > remaining = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (unsigned int slot = 0; slot < remaining.size(); ++slot)
    {
        remaining.set(slot, craftingContainer->getItem(slot));
    }
    return remaining;
}

std::vector<Recipe*>* Recipes::getRecipes(ERecipeCategory category)
{
    return &mRecipes[category];
}

not_null_ptr<ItemInstance> Recipes::getItemForRecipe(Recipe* recipe)
{
    return recipe->assemble(boost::shared_ptr<CraftingContainer>());
}

Recipe::INGREDIENTS_REQUIRED* Recipes::getRecipeIngredientsArray(ERecipeCategory category)
{
    return mRecipeIngredients[category];
}
