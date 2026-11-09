#ifndef GAME_MINIGAME_LONGWAYDOWNMEDAL_HPP
#define GAME_MINIGAME_LONGWAYDOWNMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class LongWayDownMedal : public GameMedal
{
public:
    LongWayDownMedal(int name, float target, unsigned int weight, EOperator op, bool bestOnly, bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
