#include "game/core/ToolRecipies.hpp"

#include <string>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

enum
{
    eToolList_Ingredients = 0,
    eToolList_Pickaxes = 1,
    eToolList_Shovels = 2,
    eToolList_Axes = 3,
    eToolList_Hoes = 4,
    eToolList_Count = 5
};

enum
{
    eToolShape_Pickaxe = 0,
    eToolShape_Shovel = 1,
    eToolShape_Axe = 2,
    eToolShape_Hoe = 3,
    eToolShape_Count = 4
};

static std::wstring toolRecipeShapes[eToolShape_Count][4] = {
    {L"XXX", L" # ", L" # "}, {L"X", L"#", L"#"}, {L"XX", L"X#", L" #"}, {L"XX", L" #", L" #"}};

void ToolRecipies::_init()
{
    mLists = new std::vector<Object*>[eToolList_Count];

    mLists[eToolList_Ingredients].push_back(new Object(Blocks::planks));
    mLists[eToolList_Ingredients].push_back(new Object(Blocks::cobblestone));
    mLists[eToolList_Ingredients].push_back(new Object(Items::ironIngot));
    mLists[eToolList_Ingredients].push_back(new Object(Items::diamond));
    mLists[eToolList_Ingredients].push_back(new Object(Items::goldIngot));

    mLists[eToolList_Pickaxes].push_back(new Object(Items::woodenPickaxe));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::stonePickaxe));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::ironPickaxe));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::diamondPickaxe));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::goldenPickaxe));

    mLists[eToolList_Shovels].push_back(new Object(Items::woodenShovel));
    mLists[eToolList_Shovels].push_back(new Object(Items::stoneShovel));
    mLists[eToolList_Shovels].push_back(new Object(Items::ironShovel));
    mLists[eToolList_Shovels].push_back(new Object(Items::diamondShovel));
    mLists[eToolList_Shovels].push_back(new Object(Items::goldenShovel));

    mLists[eToolList_Axes].push_back(new Object(Items::woodenAxe));
    mLists[eToolList_Axes].push_back(new Object(Items::stoneAxe));
    mLists[eToolList_Axes].push_back(new Object(Items::ironAxe));
    mLists[eToolList_Axes].push_back(new Object(Items::diamondAxe));
    mLists[eToolList_Axes].push_back(new Object(Items::goldenAxe));

    mLists[eToolList_Hoes].push_back(new Object(Items::woodenHoe));
    mLists[eToolList_Hoes].push_back(new Object(Items::stoneHoe));
    mLists[eToolList_Hoes].push_back(new Object(Items::ironHoe));
    mLists[eToolList_Hoes].push_back(new Object(Items::diamondHoe));
    mLists[eToolList_Hoes].push_back(new Object(Items::goldenHoe));
}

void ToolRecipies::addRecipes(Recipes* recipes)
{
    wchar_t types[7];
    types[6] = L'\0';

    for (unsigned int i = 0; i < mLists[eToolList_Ingredients].size(); ++i)
    {
        Object* ingredient = mLists[eToolList_Ingredients].at(i);

        for (int tool = eToolList_Pickaxes; tool < eToolList_Count; ++tool)
        {
            Item* target = static_cast<Item*>(mLists[tool].at(i)->object);

            types[0] = L'w';
            types[1] = L'c';
            types[2] = L'i';
            types[3] = L'c';
            types[5] = L'g';
            if (ingredient->type == Object::eType_Block)
            {
                types[4] = L't';
            }
            else
            {
                types[4] = L'i';
            }

            recipes->addShapedRecipe(new ItemInstance(target), types, toolRecipeShapes[tool - 1], L'#', Items::stick,
                                     L'X', ingredient->object, L'T');
        }
    }

    recipes->addShapedRecipe(new ItemInstance(Items::shears), L"sscig", L" #", L"# ", L'#', Items::ironIngot, L'T');
}
