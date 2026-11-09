#ifndef GAME_ITEM_BANNERRECIPES_HPP
#define GAME_ITEM_BANNERRECIPES_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/Recipe.hpp"

class CraftingContainer;
class BannerPattern;
class Level;
class ItemInstance;
class Recipes;

class BannerRecipes
{
public:
    BannerRecipes();
    ~BannerRecipes() {}

    ItemInstance* createOutputItem(int color, const std::wstring& pattern);
    void addRecipes(Recipes* recipes);

    class BannerAddPatternRecipe : public Recipe
    {
    public:
        virtual ~BannerAddPatternRecipe();
        const BannerPattern* matchPatterns(boost::shared_ptr<CraftingContainer> craftingContainer);
        virtual bool matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level);
        virtual not_null_ptr<ItemInstance> assemble(boost::shared_ptr<CraftingContainer> craftingContainer);
        virtual int size();
        virtual ItemInstance* getResultItem();
        virtual ItemInstance* getIngredientItem(int column, int row);
        virtual NonNullList<not_null_ptr<ItemInstance> > getRemainingItems(
            boost::shared_ptr<CraftingContainer> craftingContainer);
        virtual _eGroupType getGroup();
        virtual bool recipeRequires(int itemId);
        virtual void recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired);
        virtual boost::shared_ptr<CraftingContainer> generateExpectedCraftSlots();
    };

    class BannerDuplicateRecipe : public Recipe
    {
    public:
        virtual ~BannerDuplicateRecipe();
        virtual bool matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level);
        virtual not_null_ptr<ItemInstance> assemble(boost::shared_ptr<CraftingContainer> craftingContainer);
        virtual int size();
        virtual ItemInstance* getResultItem();
        virtual ItemInstance* getIngredientItem(int column, int row);
        virtual NonNullList<not_null_ptr<ItemInstance> > getRemainingItems(
            boost::shared_ptr<CraftingContainer> craftingContainer);
        virtual _eGroupType getGroup();
        virtual bool recipeRequires(int itemId);
        virtual void recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired);
        virtual boost::shared_ptr<CraftingContainer> generateExpectedCraftSlots();
    };

    static BannerDuplicateRecipe* mDuplicateRecipe;
    static BannerAddPatternRecipe* mAddPatternRecipe;
};

#endif
