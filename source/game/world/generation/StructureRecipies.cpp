#include "game/world/generation/StructureRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/block/QuartzBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"

void StructureRecipies::addRecipes(Recipes* recipes)
{
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone), L"ssczg", L"##", L"##", L'#',
                             new ItemInstance(Blocks::sand, 1, SandBlock::Variant::SAND->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_sandstone), L"ssczg", L"##", L"##", L'#',
                             new ItemInstance(Blocks::sand, 1, SandBlock::Variant::RED_SAND->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone, 4, SandStoneBlock::SMOOTH->getData()), L"ssczg", L"##",
                             L"##", L'#', new ItemInstance(Blocks::sandstone, 1, SandStoneBlock::SANDSTONE->getData()),
                             L'S');
    recipes->addShapedRecipe(
        new ItemInstance(Blocks::red_sandstone, 4, RedSandStoneBlock::Variant::SMOOTH->getData()), L"ssczg", L"##",
        L"##", L'#', new ItemInstance(Blocks::red_sandstone, 1, RedSandStoneBlock::Variant::DEFAULT->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::sandstone, 1, SandStoneBlock::CHISELED->getData()), L"ssczg",
                             L"#", L"#", L'#',
                             new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::SAND->getData()), L'S');
    recipes->addShapedRecipe(
        new ItemInstance(Blocks::red_sandstone, 1, RedSandStoneBlock::Variant::CHISELED->getData()), L"ssczg", L"#",
        L"#", L'#', new ItemInstance(Blocks::stone_slab2, 1, NewStoneSlabBlock::Variant::RED_SANDSTONE->getData()),
        L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block, 1, QuartzBlock::Variant::CHISELED->getData()),
                             L"ssczg", L"#", L"#", L'#',
                             new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::QUARTZ->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::quartz_block, 2, QuartzBlock::Variant::LINES_Y->getData()),
                             L"ssczg", L"#", L"#", L'#',
                             new ItemInstance(Blocks::quartz_block, 1, QuartzBlock::Variant::DEFAULT->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::crafting_table), L"ssctg", L"##", L"##", L'#', Blocks::planks,
                             L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::furnace), L"sssctg", L"###", L"# #", L"###", L'#',
                             Blocks::cobblestone, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::chest), L"sssctg", L"###", L"# #", L"###", L'#', Blocks::planks,
                             L'S');
    recipes->addShapelessRecipe(new ItemInstance(Blocks::trapped_chest), L"sttg", Blocks::chest, Blocks::tripwire_hook,
                                L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::ender_chest), L"sssctcig", L"###", L"#E#", L"###", L'#',
                             Blocks::obsidian, L'E', Items::enderEye, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::stonebrick, 4), L"ssczg", L"##", L"##", L'#',
                             new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::STONE->getData()), L'S');
    recipes->addShapedRecipe(
        new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::CHISELED_META), L"ssczg", L"#", L"#", L'#',
        new ItemInstance(Blocks::stone_slab, 1, StoneSlabBlock::Variant::SMOOTHBRICK->getData()), L'S');
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::MOSSY_META), L"ztg",
                                new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::DEFAULT_META), Blocks::vine,
                                L'S');
    recipes->addShapelessRecipe(new ItemInstance(Blocks::mossy_cobblestone, 1), L"ttg", Blocks::cobblestone,
                                Blocks::vine, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::glass_pane, 16), L"ssctg", L"###", L"###", L'#', Blocks::glass,
                             L'D');
    recipes->addShapedRecipe(new ItemInstance(Blocks::nether_brick, 1), L"sscig", L"NN", L"NN", L'N',
                             Items::netherbrick, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::red_nether_brick, 1), L"sscicig", L"NW", L"WN", L'N',
                             Items::netherbrick, L'W', Items::netherWart, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::redstone_lamp, 1), L"ssscictg", L" R ", L"RGR", L" R ", L'R',
                             Items::redstone, L'G', Blocks::glowstone, L'M');
    recipes->addShapedRecipe(new ItemInstance(Blocks::beacon, 1), L"sssctcictg", L"GGG", L"GSG", L"OOO", L'G',
                             Blocks::glass, L'S', Items::netherStar, L'O', Blocks::obsidian, L'M');
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 2, StoneBlock::Variant::DIORITE->getData()), L"ssctcig",
                             L"CQ", L"QC", L'C', Blocks::cobblestone, L'Q', Items::quartz, L'S');
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::GRANITE->getData()), L"zig",
                                new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::DIORITE->getData()),
                                Items::quartz, L'S');
    recipes->addShapelessRecipe(new ItemInstance(Blocks::stone, 2, StoneBlock::Variant::ANDESITE->getData()), L"ztg",
                                new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::DIORITE->getData()),
                                Blocks::cobblestone, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::dirt, 4, DirtBlock::Variant::COARSE_DIRT->getData()), L"ssczctg",
                             L"DG", L"GD", L'D', new ItemInstance(Blocks::dirt, 1, DirtBlock::Variant::DIRT->getData()),
                             L'G', Blocks::gravel, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 4, StoneBlock::Variant::SMOOTH_DIORITE->getData()),
                             L"ssczg", L"SS", L"SS", L'S',
                             new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::DIORITE->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 4, StoneBlock::Variant::SMOOTH_GRANITE->getData()),
                             L"ssczg", L"SS", L"SS", L'S',
                             new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::GRANITE->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::stone, 4, StoneBlock::Variant::SMOOTH_ANDESITE->getData()),
                             L"ssczg", L"SS", L"SS", L'S',
                             new ItemInstance(Blocks::stone, 1, StoneBlock::Variant::ANDESITE->getData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::ROUGH_DATA), L"sscig", L"SS",
                             L"SS", L'S', Items::prismarineShard, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::BRICKS_DATA), L"ssscig", L"SSS",
                             L"SSS", L"SSS", L'S', Items::prismarineShard, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::prismarine, 1, PrismarineBlock::DARK_DATA), L"sssciczg", L"SSS",
                             L"SIS", L"SSS", L'S', Items::prismarineShard, L'I',
                             new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()), L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::purpur_block, 4, 0), L"sscig", L"FF", L"FF", L'F',
                             Items::chorusFruitPopped, L'D');
    recipes->addShapedRecipe(new ItemInstance(Blocks::purpur_pillar, 1, 0), L"ssctg", L"#", L"#", L'#',
                             Blocks::purpur_slab, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::end_bricks, 4, 0), L"ssctg", L"##", L"##", L'#',
                             Blocks::end_stone, L'S');
    recipes->addShapedRecipe(new ItemInstance(Blocks::magma, 1, 0), L"sscig", L"##", L"##", L'#', Items::magmaCream,
                             L'D');
    recipes->addShapedRecipe(new ItemInstance(Blocks::nether_wart_block, 1, 0), L"ssscig", L"###", L"###", L"###", L'#',
                             Items::netherWart, L'D');
}
