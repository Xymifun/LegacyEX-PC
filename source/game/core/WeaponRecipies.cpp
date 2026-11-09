#include "game/core/WeaponRecipies.hpp"

#include <string>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

std::wstring SWORD_PATTERN[4] = {L"X", L"X", L"#", L""};

void WeaponRecipies::_init()
{
    mLists = new std::vector<Object*>[2];

    mLists[0].push_back(new Object(Blocks::planks));
    mLists[0].push_back(new Object(Blocks::cobblestone));
    mLists[0].push_back(new Object(Items::ironIngot));
    mLists[0].push_back(new Object(Items::diamond));
    mLists[0].push_back(new Object(Items::goldIngot));

    mLists[1].push_back(new Object(Items::woodenSword));
    mLists[1].push_back(new Object(Items::stoneSword));
    mLists[1].push_back(new Object(Items::ironSword));
    mLists[1].push_back(new Object(Items::diamondSword));
    mLists[1].push_back(new Object(Items::goldenSword));
}

void WeaponRecipies::addRecipes(Recipes* recipes)
{
    wchar_t types[7];
    types[6] = L'\0';

    for (unsigned int i = 0; i < mLists[0].size(); ++i)
    {
        Object* ingredient = mLists[0].at(i);
        Item* weapon = static_cast<Item*>(mLists[1].at(i)->object);

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

        recipes->addShapedRecipe(new ItemInstance(weapon), types, SWORD_PATTERN, L'#', Items::stick, L'X',
                                 ingredient->object, L'T');
    }
}
