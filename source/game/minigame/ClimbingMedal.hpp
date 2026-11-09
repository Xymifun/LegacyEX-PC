#ifndef GAME_MINIGAME_CLIMBINGMEDAL_HPP
#define GAME_MINIGAME_CLIMBINGMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class ClimbingMedal : public GameMedal
{
public:
    ClimbingMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                  bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
