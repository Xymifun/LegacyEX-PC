#ifndef GAME_CORE_ORERECIPIES_HPP
#define GAME_CORE_ORERECIPIES_HPP

#include <vector>

#include "game/core/Object.hpp"

class Recipes;

class OreRecipies
{
public:
    OreRecipies() { _init(); }
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<Object*> blockPairs[9];
};

#endif
