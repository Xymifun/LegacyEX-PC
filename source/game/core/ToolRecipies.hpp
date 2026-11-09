#ifndef GAME_CORE_TOOLRECIPIES_HPP
#define GAME_CORE_TOOLRECIPIES_HPP

#include <vector>

#include "game/core/Object.hpp"

class Recipes;

class ToolRecipies
{
public:
    ToolRecipies() { _init(); }
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<Object*>* mLists;
};

#endif
