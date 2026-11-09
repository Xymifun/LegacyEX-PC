#ifndef GAME_MINIGAME_LEASTKILLSMEDAL_HPP
#define GAME_MINIGAME_LEASTKILLSMEDAL_HPP

#include "game/minigame/MostKillsMedal.hpp"

class PlayerStats;

class LeastKillsMedal : public MostKillsMedal
{
public:
    LeastKillsMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                    bool canMultiAward)
        : MostKillsMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
