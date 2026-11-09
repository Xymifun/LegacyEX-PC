#ifndef GAME_MINIGAME_JUMPSMEDAL_HPP
#define GAME_MINIGAME_JUMPSMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class JumpsMedal : public GameMedal
{
public:
    JumpsMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly, bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
