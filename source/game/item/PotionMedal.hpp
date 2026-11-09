#ifndef GAME_ITEM_POTIONMEDAL_HPP
#define GAME_ITEM_POTIONMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class PotionMedal : public GameMedal
{
public:
    PotionMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly, bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
