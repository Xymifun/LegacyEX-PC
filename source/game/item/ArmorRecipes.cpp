#include "game/item/ArmorRecipes.hpp"

#include <string>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"

enum
{
    eArmorList_Ingredients = 0,
    eArmorList_Helmets = 1,
    eArmorList_Chestplates = 2,
    eArmorList_Leggings = 3,
    eArmorList_Boots = 4,
    eArmorList_Count = 5
};

enum
{
    eArmorShape_Helmet = 0,
    eArmorShape_Chestplate = 1,
    eArmorShape_Leggings = 2,
    eArmorShape_Boots = 3,
    eArmorShape_Count = 4
};

static std::wstring armorRecipeShapes[eArmorShape_Count][4] = {
    {L"XXX", L"X X", L""}, {L"X X", L"XXX", L"XXX", L""}, {L"XXX", L"X X", L"X X", L""}, {L"X X", L"X X", L""}};

void ArmorRecipes::_init()
{
    mLists = new std::vector<Object*>[eArmorList_Count];

    mLists[eArmorList_Ingredients].push_back(new Object(Items::leather));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::ironIngot));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::diamond));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::goldIngot));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::turtleShellPiece));

    mLists[eArmorList_Helmets].push_back(new Object(Items::leatherHelmet));
    mLists[eArmorList_Helmets].push_back(new Object(Items::ironHelmet));
    mLists[eArmorList_Helmets].push_back(new Object(Items::diamondHelmet));
    mLists[eArmorList_Helmets].push_back(new Object(Items::goldenHelmet));
    mLists[eArmorList_Helmets].push_back(new Object(Items::turtleHelmet));

    mLists[eArmorList_Chestplates].push_back(new Object(Items::leatherChestplate));
    mLists[eArmorList_Chestplates].push_back(new Object(Items::ironChestplate));
    mLists[eArmorList_Chestplates].push_back(new Object(Items::diamondChestplate));
    mLists[eArmorList_Chestplates].push_back(new Object(Items::goldenChestplate));

    mLists[eArmorList_Leggings].push_back(new Object(Items::leatherLeggings));
    mLists[eArmorList_Leggings].push_back(new Object(Items::ironLeggings));
    mLists[eArmorList_Leggings].push_back(new Object(Items::diamondLeggings));
    mLists[eArmorList_Leggings].push_back(new Object(Items::goldenLeggings));

    mLists[eArmorList_Boots].push_back(new Object(Items::leatherBoots));
    mLists[eArmorList_Boots].push_back(new Object(Items::ironBoots));
    mLists[eArmorList_Boots].push_back(new Object(Items::diamondBoots));
    mLists[eArmorList_Boots].push_back(new Object(Items::goldenBoots));
}

void ArmorRecipes::addRecipes(Recipes* recipes)
{
    wchar_t types[5];
    types[4] = L'\0';

    unsigned int count = mLists[eArmorList_Ingredients].size();
    for (unsigned int i = 0; i < count; ++i)
    {
        Object* ingredient = mLists[eArmorList_Ingredients].at(i);

        for (int armor = eArmorList_Helmets; armor < eArmorList_Count; ++armor)
        {
            if (i < mLists[armor].size())
            {
                Item* target = static_cast<Item*>(mLists[armor].at(i)->object);

                types[0] = L'w';
                types[1] = L'c';
                types[3] = L'g';
                if (ingredient->type == Object::eType_Block)
                {
                    types[2] = L't';
                }
                else
                {
                    types[2] = L'i';
                }

                recipes->addShapedRecipe(new ItemInstance(target), types, armorRecipeShapes[armor - 1], L'X',
                                         ingredient->object, L'A');
            }
        }
    }

    recipes->addShapedRecipe(new ItemInstance(Items::leatherHorseArmor), L"ssscig", L"~ ~", L"~~~", L"~ ~", L'~',
                             Items::leather, L'A');
}

int ArmorRecipes::GetArmorType(Item* item)
{
    return item->GetArmorType();
}
