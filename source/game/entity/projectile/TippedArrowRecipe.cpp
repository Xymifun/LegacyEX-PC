#include "game/entity/projectile/TippedArrowRecipe.hpp"

#include "game/item/CraftingContainer.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/item/Recipes.hpp"

TippedArrowRecipe::TippedArrowRecipe() : Recipe()
{
}

TippedArrowRecipe::~TippedArrowRecipe()
{
}

void TippedArrowRecipe::addRecipes(Recipes* recipes)
{
    mPotions = new std::vector<Potion*>;
    mPotions->push_back(Potions::WATER);
    mPotions->push_back(Potions::NIGHT_VISION);
    mPotions->push_back(Potions::INVISIBILITY);
    mPotions->push_back(Potions::LEAPING);
    mPotions->push_back(Potions::FIRE_RESISTANCE);
    mPotions->push_back(Potions::SWIFTNESS);
    mPotions->push_back(Potions::SLOWNESS);
    mPotions->push_back(Potions::WATER_BREATHING);
    mPotions->push_back(Potions::HEALING);
    mPotions->push_back(Potions::HARMING);
    mPotions->push_back(Potions::POISON);
    mPotions->push_back(Potions::REGENERATION);
    mPotions->push_back(Potions::STRENGTH);
    mPotions->push_back(Potions::WEAKNESS);
    mPotions->push_back(Potions::LUCK);
    mPotions->push_back(Potions::TURTLE_MASTER);
    mPotions->push_back(Potions::SLOW_FALLING);
    mPotions->push_back(Potions::LONG_NIGHT_VISION);
    mPotions->push_back(Potions::LONG_INVISIBILITY);
    mPotions->push_back(Potions::LONG_LEAPING);
    mPotions->push_back(Potions::LONG_FIRE_RESISTANCE);
    mPotions->push_back(Potions::LONG_SWIFTNESS);
    mPotions->push_back(Potions::LONG_SLOWNESS);
    mPotions->push_back(Potions::LONG_WATER_BREATHING);
    mPotions->push_back(Potions::LONG_POISON);
    mPotions->push_back(Potions::LONG_REGENERATION);
    mPotions->push_back(Potions::LONG_STRENGTH);
    mPotions->push_back(Potions::LONG_WEAKNESS);
    mPotions->push_back(Potions::LONG_TURTLE_MASTER);
    mPotions->push_back(Potions::WITHER);
    mPotions->push_back(Potions::LONG_SLOW_FALLING);
    mPotions->push_back(Potions::STRONG_LEAPING);
    mPotions->push_back(Potions::STRONG_SWIFTNESS);
    mPotions->push_back(Potions::STRONG_HEALING);
    mPotions->push_back(Potions::STRONG_HARMING);
    mPotions->push_back(Potions::STRONG_POISON);
    mPotions->push_back(Potions::STRONG_REGENERATION);
    mPotions->push_back(Potions::STRONG_STRENGTH);
    mPotions->push_back(Potions::STRONG_TURTLE_MASTER);
    for (std::vector<Potion*>::iterator it = mPotions->begin(); it != mPotions->end(); ++it)
    {
        Potion* potion = *it;
        not_null_ptr<ItemInstance> result = PotionUtils::setPotion(
            not_null_ptr<ItemInstance>(new ItemInstance(Items::tippedArrow, 8)), potion);
        not_null_ptr<ItemInstance> ingredient = PotionUtils::setPotion(
            not_null_ptr<ItemInstance>(new ItemInstance(Items::lingeringPotion)), potion);

        recipes->addShapedRecipe(result->copy_not_shared(), L"sssciczg", L"|||", L"|o|", L"|||", L'|', Items::arrow,
                                 L'o', ingredient->copy_not_shared(), L'T');
    }
}

bool TippedArrowRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    if (craftingContainer->getWidth() != 3 || craftingContainer->getHeight() != 3)
    {
        return false;
    }

    for (int column = 0; column < craftingContainer->getWidth(); ++column)
    {
        for (int row = 0; row < craftingContainer->getHeight(); ++row)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(column, row);
            if (item->isEmpty())
            {
                return false;
            }
            Item* itemType = item->getItem();
            if (column == 1 && row == 1)
            {
                if (itemType != Items::lingeringPotion)
                {
                    return false;
                }
            }
            else if (itemType != Items::arrow)
            {
                return false;
            }
        }
    }
    return true;
}

not_null_ptr<ItemInstance> TippedArrowRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> ingredient = craftingContainer->getItem(1, 1);
    if (ingredient->getItem() != Items::lingeringPotion)
    {
        return ItemInstance::EMPTY;
    }

    not_null_ptr<ItemInstance> result(new ItemInstance(Items::tippedArrow, 8));
    PotionUtils::setPotionAndCustomEffects(result, ingredient);
    return result;
}

int TippedArrowRecipe::size()
{
    return 9;
}

ItemInstance* TippedArrowRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* TippedArrowRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > TippedArrowRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    return NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(),
                                                              ItemInstance::EMPTY);
}

boost::shared_ptr<CraftingContainer> TippedArrowRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}

Recipe::_eGroupType TippedArrowRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool TippedArrowRecipe::recipeRequires(int itemId)
{
    return false;
}

void TippedArrowRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}
