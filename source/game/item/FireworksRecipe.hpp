#ifndef GAME_ITEM_FIREWORKSRECIPE_HPP
#define GAME_ITEM_FIREWORKSRECIPE_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Recipe.hpp"
#include "game/item/not_null_ptr.hpp"

class CraftingContainer;
class ItemInstance;
class Level;

class FireworksRecipe : public Recipe
{
public:
    class ThreadStorage
    {
    public:
        ThreadStorage();

        not_null_ptr<ItemInstance> mResultItem;
    };

    FireworksRecipe();
    virtual ~FireworksRecipe();

    static void updatePossibleRecipes(boost::shared_ptr<CraftingContainer> craftingContainer, bool* hasPaper,
                                      bool* hasGunpowder, bool* hasFireworkCharge);
    static bool isValidIngredient(not_null_ptr<ItemInstance> item, bool hasPaper, bool hasGunpowder,
                                  bool hasFireworkCharge);
    void setResultItem(not_null_ptr<ItemInstance> item);
    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

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

#endif
