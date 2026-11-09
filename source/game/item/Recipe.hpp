#ifndef GAME_ITEM_RECIPE_HPP
#define GAME_ITEM_RECIPE_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class BannerRecipes;
class CompoundTag;
class CraftingContainer;
class ItemInstance;
class Level;
class Recipes;

class Recipe
{
    friend class BannerRecipes;
    friend class Recipes;
    friend class ShulkerBoxRecipes;
    friend class IUIScene_CraftingMenu;

public:
    enum _eGroupType
    {
        eGroupType_None = -1,
        eGroupType_Structures = 0,
        eGroupType_Tools = 1,
        eGroupType_Food = 2,
        eGroupType_Armour = 3,
        eGroupType_Mechanisms = 4,
        eGroupType_Vehicles = 5,
        eGroupType_Miscellaneous = 6,
        eGroupType_Excluded = 7
    };

    struct INGREDIENTS_REQUIRED
    {
        int numIngredients;
        int requiresCraftingTable;
        int* itemIds;
        int* itemCounts;
        int* auxValues;
        CompoundTag** itemTags;
        Recipe* recipe;
        bool available[4];
        int* craftingGrid;
        unsigned short missingSlotMask[4];
    };

    Recipe() : mBaseItemType(0), mAchievementId(-1), mDisplayInSimpleCrafting(true) {}

    virtual ~Recipe() {}
    virtual bool matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level) = 0;
    virtual not_null_ptr<ItemInstance> assemble(boost::shared_ptr<CraftingContainer> craftingContainer) = 0;
    virtual int size() = 0;
    virtual ItemInstance* getResultItem() = 0;
    virtual ItemInstance* getIngredientItem(int column, int row) = 0;
    virtual NonNullList<not_null_ptr<ItemInstance> > getRemainingItems(
        boost::shared_ptr<CraftingContainer> craftingContainer) = 0;
    virtual _eGroupType getGroup() = 0;
    virtual bool recipeRequires(int itemId) = 0;
    virtual void recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired) = 0;
    virtual boost::shared_ptr<CraftingContainer> generateExpectedCraftSlots() = 0;
    virtual bool displayInSimpleCrafting();

protected:
    int mField0x0;
    int mBaseItemType;
    int mAchievementId;
    bool mDisplayInSimpleCrafting;
};

#endif
