#include "game/item/ClothDyeRecipes.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/FlowerBlock.hpp"

void ClothDyeRecipes::addRecipes(Recipes* recipes)
{
    for (int color = 0; color < 16; ++color)
    {
        if (color != DyeColor::WHITE->getBlockData())
        {
            recipes->addShapelessRecipe(new ItemInstance(Blocks::wool, 1, color), L"zzg",
                                        new ItemInstance(Items::dye, 1, 15 - color),
                                        new ItemInstance(Item::byBlock(Blocks::wool)), L'D');
        }
        recipes->addShapedRecipe(new ItemInstance(Blocks::stained_hardened_clay, 8, 15 - color), L"sssczczg", L"###",
                                 L"#X#", L"###", L'#', new ItemInstance(Blocks::hardened_clay), L'X',
                                 new ItemInstance(Items::dye, 1, color), L'D');
        recipes->addShapelessRecipe(
            new ItemInstance(Blocks::concrete_powder, 8, 15 - color), L"zzzzzzzzzg",
            new ItemInstance(Items::dye, 1, color), new ItemInstance(Blocks::gravel), new ItemInstance(Blocks::gravel),
            new ItemInstance(Blocks::gravel), new ItemInstance(Blocks::gravel), new ItemInstance(Blocks::sand),
            new ItemInstance(Blocks::sand), new ItemInstance(Blocks::sand), new ItemInstance(Blocks::sand), L'D');
        recipes->addShapedRecipe(new ItemInstance(Blocks::stained_glass, 8, 15 - color), L"sssczczg", L"###", L"#X#",
                                 L"###", L'#', new ItemInstance(Blocks::glass), L'X',
                                 new ItemInstance(Items::dye, 1, color), L'D');
        recipes->addShapedRecipe(new ItemInstance(Blocks::stained_glass_pane, 16, 15 - color), L"ssczg", L"###", L"###",
                                 L'#', new ItemInstance(Blocks::stained_glass, 1, 15 - color), L'D');
    }

    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::YELLOW->getItemData()), L"zg",
                                new ItemInstance(Blocks::yellow_flower, 1, FlowerBlock::Flower::DANDELION->getData()),
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::POPPY->getData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 3, DyeColor::WHITE->getItemData()), L"ig", Items::bone,
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 9, DyeColor::WHITE->getItemData()), L"tg",
                                Blocks::bone_block, L'D');

    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::PINK->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::ORANGE->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::YELLOW->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::LIME->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::GREEN->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::GRAY->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::SILVER->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::GRAY->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 3, DyeColor::SILVER->getItemData()), L"zzzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLACK->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::LIGHT_BLUE->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::CYAN->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::GREEN->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::PURPLE->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::MAGENTA->getItemData()), L"zzg",
                                new ItemInstance(Items::dye, 1, DyeColor::PURPLE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::PINK->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 3, DyeColor::MAGENTA->getItemData()), L"zzzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::PINK->getItemData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 4, DyeColor::MAGENTA->getItemData()), L"zzzzg",
                                new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()),
                                new ItemInstance(Items::dye, 1, DyeColor::WHITE->getItemData()), L"D");

    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::LIGHT_BLUE->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::BLUE_ORCHID->getData()),
                                L"D");
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::MAGENTA->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::ALLIUM->getData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::SILVER->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::HOUSTONIA->getData()),
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::RED_TULIP->getData()),
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::ORANGE->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::ORANGE_TULIP->getData()),
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::SILVER->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::WHITE_TULIP->getData()),
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::PINK->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::PINK_TULIP->getData()),
                                L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::SILVER->getItemData()), L"zg",
                                new ItemInstance(Blocks::red_flower, 1, FlowerBlock::Flower::OXEYE_DAISY->getData()),
                                L'D');
    recipes->addShapelessRecipe(
        new ItemInstance(Items::dye, 2, DyeColor::YELLOW->getItemData()), L"zg",
        new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::Variant::SUNFLOWER->getData()), L'D');
    recipes->addShapelessRecipe(
        new ItemInstance(Items::dye, 2, DyeColor::MAGENTA->getItemData()), L"zg",
        new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::Variant::SYRINGA->getData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 2, DyeColor::RED->getItemData()), L"zg",
                                new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::Variant::ROSE->getData()),
                                L'D');
    recipes->addShapelessRecipe(
        new ItemInstance(Items::dye, 2, DyeColor::PINK->getItemData()), L"zg",
        new ItemInstance(Blocks::double_plant, 1, DoublePlantBlock::Variant::PAEONIA->getData()), L'D');
    recipes->addShapelessRecipe(new ItemInstance(Items::dye, 1, DyeColor::RED->getItemData()), L"ig", Items::beetroot,
                                L'D');
    for (int color = 0; color < 16; ++color)
    {
        recipes->addShapedRecipe(new ItemInstance(Blocks::carpet, 3, color), L"sczg", L"##", L'#',
                                 new ItemInstance(Blocks::wool, 1, color), L'D');
    }
    for (int color = 0; color < 16; ++color)
    {
        recipes->addShapelessRecipe(new ItemInstance(Items::bed, 1, color), L"zzgv",
                                    new ItemInstance(Items::dye, 1, 15 - color), new ItemInstance(Items::bed, 1, -1),
                                    L'X', eAward_ChangeOfSheets);
    }
}
