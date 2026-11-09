#ifndef GAME_MINIGAME_FOODMEDAL_HPP
#define GAME_MINIGAME_FOODMEDAL_HPP

#include <vector>

#include "game/item/FoodItem.hpp"
#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class FoodMedal : public GameMedal
{
public:
    FoodMedal(int name, float target, unsigned int weight, Item* item);
    FoodMedal(int name, float target, unsigned int weight, std::vector<Item*>* items);

    virtual ~FoodMedal();

    virtual float GetScore(PlayerStats* playerStats) const;

private:
    std::vector<Item*>* mItems;
};

#endif
