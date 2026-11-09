#ifndef GAME_CORE_WEAPONRECIPIES_HPP
#define GAME_CORE_WEAPONRECIPIES_HPP

#include <vector>

#include "game/core/Object.hpp"

class Recipes;

class WeaponRecipies
{
public:
    WeaponRecipies() { _init(); }
    void _init();
    void addRecipes(Recipes* recipes);

private:
    std::vector<Object*>* mLists;
};

#endif
