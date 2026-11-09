#ifndef GAME_ITEM_SPLASHPOTIONMEDAL_HPP
#define GAME_ITEM_SPLASHPOTIONMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class SplashPotionMedal : public GameMedal
{
public:
    SplashPotionMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                      bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
