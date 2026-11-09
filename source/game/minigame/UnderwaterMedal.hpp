#ifndef GAME_MINIGAME_UNDERWATERMEDAL_HPP
#define GAME_MINIGAME_UNDERWATERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class UnderwaterMedal : public GameMedal
{
public:
    UnderwaterMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                    bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
