#ifndef GAME_ITEM_FURNACERECIPES_HPP
#define GAME_ITEM_FURNACERECIPES_HPP

#include <boost/unordered_map.hpp>

#include "game/item/ItemInstance.hpp"

class Block;
class Item;

class FurnaceRecipes : public boost::unordered_map<ItemInstance*, ItemInstance*>
{
public:
    FurnaceRecipes();
    bool isSameItem(not_null_ptr<ItemInstance> item, ItemInstance* recipeItem);
    ItemInstance* getResult(not_null_ptr<ItemInstance> item);
    static FurnaceRecipes* getInstance();
    float getRecipeValue(not_null_ptr<ItemInstance> item);
    void addFurnaceRecipe(ItemInstance* item, ItemInstance* result, float experience);
    void addFurnaceRecipe(Item* item, ItemInstance* result, float experience);
    void addFurnaceRecipe(Block* block, ItemInstance* result, float experience);
    static void staticCtor();
    boost::unordered_map<ItemInstance*, ItemInstance*> getRecipies();

private:
    boost::unordered_map<ItemInstance*, float> mRecipeExperience;
};

#endif
