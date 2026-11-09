#include "game/core/OreRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void OreRecipies::_init()
{
    blockPairs[0].push_back(new Object(Blocks::gold_block));
    blockPairs[0].push_back(new Object(new ItemInstance(Items::goldIngot, 9)));
    blockPairs[1].push_back(new Object(Blocks::iron_block));
    blockPairs[1].push_back(new Object(new ItemInstance(Items::ironIngot, 9)));
    blockPairs[2].push_back(new Object(Blocks::diamond_block));
    blockPairs[2].push_back(new Object(new ItemInstance(Items::diamond, 9)));
    blockPairs[3].push_back(new Object(Blocks::emerald_block));
    blockPairs[3].push_back(new Object(new ItemInstance(Items::emerald, 9)));
    blockPairs[4].push_back(new Object(Blocks::lapis_block));
    blockPairs[4].push_back(new Object(new ItemInstance(Items::dye, 9, DyeColor::BLUE->getItemData())));
    blockPairs[5].push_back(new Object(Blocks::redstone_block));
    blockPairs[5].push_back(new Object(new ItemInstance(Items::redstone, 9)));
    blockPairs[6].push_back(new Object(Blocks::coal_block));
    blockPairs[6].push_back(new Object(new ItemInstance(Items::coal, 9, 0)));
    blockPairs[7].push_back(new Object(Blocks::hay_block));
    blockPairs[7].push_back(new Object(new ItemInstance(Items::wheat, 9)));
    blockPairs[8].push_back(new Object(Blocks::slime));
    blockPairs[8].push_back(new Object(new ItemInstance(Items::slimeBall, 9)));
}

void OreRecipies::addRecipes(Recipes* recipes)
{
    for (int i = 0; i < 9; ++i)
    {
        Block* block = static_cast<Block*>(blockPairs[i].at(0)->object);
        ItemInstance* ingot = static_cast<ItemInstance*>(blockPairs[i].at(1)->object);

        recipes->addShapedRecipe(new ItemInstance(block), L"sssczg", L"###", L"###", L"###", L'#', ingot, L'D');
        recipes->addShapedRecipe(ingot, L"sctg", L"#", L'#', block, L'D');
    }

    recipes->addShapedRecipe(new ItemInstance(Items::string, 9), L"sctbg", L"#", L'#', Blocks::web, 0x1f, L'D');

    recipes->addShapedRecipe(new ItemInstance(Items::goldIngot), L"ssscig", L"###", L"###", L"###", L'#',
                             Items::goldNugget, L'D');
    recipes->addShapedRecipe(new ItemInstance(Items::goldNugget, 9), L"scig", L"#", L'#', Items::goldIngot, L'D');

    recipes->addShapedRecipe(new ItemInstance(Items::ironIngot), L"ssscig", L"###", L"###", L"###", L'#',
                             Items::ironNugget, L'D');
    recipes->addShapedRecipe(new ItemInstance(Items::ironNugget, 9), L"scig", L"#", L'#', Items::ironIngot, L'D');
}
