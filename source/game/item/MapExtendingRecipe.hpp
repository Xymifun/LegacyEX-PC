#ifndef GAME_ITEM_MAPEXTENDINGRECIPE_HPP
#define GAME_ITEM_MAPEXTENDINGRECIPE_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/ShapedRecipe.hpp"

class CraftingContainer;
class Level;
class MapItemSavedData;

class MapExtendingRecipe : public ShapedRecipe
{
public:
    MapExtendingRecipe();
    virtual ~MapExtendingRecipe();

    static void StaticCtor();
    bool isExplorationMap(const boost::shared_ptr<MapItemSavedData>& data);
    virtual bool matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level);
    virtual not_null_ptr<ItemInstance> assemble(boost::shared_ptr<CraftingContainer> craftingContainer);
    virtual Recipe::_eGroupType getGroup();
    virtual bool recipeRequires(int itemId);
    virtual void recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired);
};

#endif
