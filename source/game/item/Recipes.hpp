#ifndef GAME_ITEM_RECIPES_HPP
#define GAME_ITEM_RECIPES_HPP

#include <cstdarg>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/item/Recipe.hpp"

class ArmorDyeRecipe;
class ArmorRecipes;
class BannerRecipes;
class BookCloningRecipe;
class ClothDyeRecipes;
class CraftingContainer;
class FireworksRecipe;
class FoodRecipies;
class ItemInstance;
class Level;
class OreRecipies;
class RepairItemRecipe;
class ShulkerBoxColoring;
class ShulkerBoxRecipes;
class StructureRecipies;
class TippedArrowRecipe;
class ToolRecipies;
class WeaponRecipies;

class Recipes
{
public:
    enum ERecipeCategory
    {
        eRecipeCategory_Crafting = 0,
        eRecipeCategory_Banners,
        eRecipeCategory_Fireworks,
        eRecipeCategory_Dyeing,
        eRecipeCategory_Reserved,
        eRecipeCategory_Count
    };

    Recipes();
    Recipe* addShapedRecipe(ItemInstance* result, int category, va_list arguments);
    Recipe* addShapedRecipe(ItemInstance* result, ...);
    Recipe* addShapedRecipe(ItemInstance* result, int category, ...);
    void addShapelessRecipe(ItemInstance* result, int category, va_list arguments);
    void addShapelessRecipe(ItemInstance* result, ...);
    void addShapelessRecipe(ItemInstance* result, int category, ...);
    void buildRecipeIngredientsArray();
    static void staticCtor();
    void addCustomRecipe(Recipe* recipe, ERecipeCategory category);
    not_null_ptr<ItemInstance> getItemFor(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level,
                                          Recipe* recipe, ERecipeCategory category);
    eAward hasAchievementAttachedToRecipe(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level,
                                          Recipe* recipe, ERecipeCategory category);
    NonNullList<not_null_ptr<ItemInstance> > getRemainingItemsFor(
        boost::shared_ptr<CraftingContainer> craftingContainer, Level* level, ERecipeCategory category);
    std::vector<Recipe*>* getRecipes(ERecipeCategory category);
    not_null_ptr<ItemInstance> getItemForRecipe(Recipe* recipe);
    Recipe::INGREDIENTS_REQUIRED* getRecipeIngredientsArray(ERecipeCategory category);
    void _init();
    static Recipes* getInstance() { return mInstance; }

    static ToolRecipies* mToolRecipies;
    static WeaponRecipies* mWeaponRecipies;
    static OreRecipies* mOreRecipies;
    static FoodRecipies* mFoodRecipies;
    static StructureRecipies* mStructureRecipies;
    static ArmorRecipes* mArmorRecipes;
    static ClothDyeRecipes* mClothDyeRecipes;
    static BannerRecipes* mBannerRecipes;
    static ShulkerBoxRecipes* mShulkerBoxRecipes;
    static ShulkerBoxColoring* mShulkerBoxColoring;
    static FireworksRecipe* mFireworksRecipe;
    static RepairItemRecipe* mRepairItemRecipe;
    static BookCloningRecipe* mBookCloningRecipe;
    static TippedArrowRecipe* mTippedArrowRecipe;
    static ArmorDyeRecipe* mArmorDyeRecipe;

private:
    std::vector<Recipe*> mRecipes[eRecipeCategory_Count];
    Recipe::INGREDIENTS_REQUIRED* mRecipeIngredients[eRecipeCategory_Count];
    static Recipes* mInstance;
};

#endif
