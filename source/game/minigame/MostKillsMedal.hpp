#ifndef GAME_MINIGAME_MOSTKILLSMEDAL_HPP
#define GAME_MINIGAME_MOSTKILLSMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class MostKillsMedal : public GameMedal
{
public:
    MostKillsMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                   bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
