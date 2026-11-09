#include "game/core/FoodRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void FoodRecipies::addRecipes(Recipes* recipes)
{
    recipes->addShapedRecipe(new ItemInstance(Items::goldenApple), L"ssscicig", L"###", L"#X#", L"###", L'#',
                             Items::goldIngot, L'X', Items::apple, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::goldenApple, 1, 1), L"sssctcig", L"###", L"#X#", L"###", L'#',
                             Blocks::gold_block, L'X', Items::apple, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::speckledMelon, 1), L"ssscicig", L"###", L"#X#", L"###", L'#',
                             Items::goldNugget, L'X', Items::melon, L'F');
    recipes->addShapelessRecipe(new ItemInstance(Items::mushroomStew), L"ttig", Blocks::brown_mushroom,
                                Blocks::red_mushroom, Items::bowl, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::cookie, 8), L"sczcig", L"#X#", L'X',
                             new ItemInstance(Items::dye, 1, DyeColor::BROWN->getItemData()), L'#', Items::wheat, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::rabbitStew), L"sssczcicictcig", L" R ", L"CPM", L" B ", L'R',
                             new ItemInstance(Items::cookedRabbit), L'C', Items::carrot, L'P', Items::bakedPotato, L'M',
                             Blocks::brown_mushroom, L'B', Items::bowl, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::rabbitStew), L"sssczcicictcig", L" R ", L"CPD", L" B ", L'R',
                             new ItemInstance(Items::cookedRabbit), L'C', Items::carrot, L'P', Items::bakedPotato, L'D',
                             Blocks::red_mushroom, L'B', Items::bowl, L'F');
    recipes->addShapedRecipe(new ItemInstance(Blocks::melon_block), L"ssscig", L"MMM", L"MMM", L"MMM", L'M',
                             Items::melon, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::beetrootSoup), L"ssscicig", L"OOO", L"OOO", L" B ", L'O',
                             Items::beetroot, L'B', Items::bowl, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::melonSeeds), L"scig", L"M", L'M', Items::melon, L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::pumpkinSeeds, 4), L"sctg", L"M", L'M', Blocks::pumpkin, L'F');
    recipes->addShapelessRecipe(new ItemInstance(Items::pumpkinPie), L"tiig", Blocks::pumpkin, Items::sugar, Items::egg,
                                L'F');
    recipes->addShapedRecipe(new ItemInstance(Items::goldenCarrot), L"ssscicig", L"###", L"#X#", L"###", L'#',
                             Items::goldNugget, L'X', Items::carrot, L'F');
    recipes->addShapelessRecipe(new ItemInstance(Items::fermentedSpiderEye), L"itig", Items::spiderEye,
                                Blocks::brown_mushroom, Items::sugar, L'F');
    recipes->addShapelessRecipe(new ItemInstance(Items::blazePowder, 2), L"ig", Items::blazeRod, L'F');
    recipes->addShapelessRecipe(new ItemInstance(Items::magmaCream), L"iig", Items::blazePowder, Items::slimeBall,
                                L'F');
}
