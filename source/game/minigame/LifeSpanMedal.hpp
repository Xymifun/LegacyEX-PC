#ifndef GAME_MINIGAME_LIFESPANMEDAL_HPP
#define GAME_MINIGAME_LIFESPANMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class LifeSpanMedal : public GameMedal
{
public:
    LifeSpanMedal(int name, float target, unsigned int weight, GameMedal::EOperator op);


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
