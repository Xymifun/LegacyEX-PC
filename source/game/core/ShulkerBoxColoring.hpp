#ifndef GAME_CORE_SHULKERBOXCOLORING_HPP
#define GAME_CORE_SHULKERBOXCOLORING_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Recipe.hpp"
#include "game/world/Level.hpp"

class CraftingContainer;
class ItemInstance;
class ShulkerBoxColoring : public Recipe
{
public:
    virtual ~ShulkerBoxColoring();

    virtual bool matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level);
    virtual not_null_ptr<ItemInstance> assemble(boost::shared_ptr<CraftingContainer> craftingContainer);
    virtual int size();
    virtual ItemInstance* getResultItem();
    virtual ItemInstance* getIngredientItem(int column, int row);
    virtual NonNullList<not_null_ptr<ItemInstance> > getRemainingItems(
        boost::shared_ptr<CraftingContainer> craftingContainer);
    virtual Recipe::_eGroupType getGroup();
    virtual bool recipeRequires(int itemId);
    virtual void recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired);
    virtual boost::shared_ptr<CraftingContainer> generateExpectedCraftSlots();
};

#endif
