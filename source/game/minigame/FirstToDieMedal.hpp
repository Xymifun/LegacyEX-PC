#ifndef GAME_MINIGAME_FIRSTTODIEMEDAL_HPP
#define GAME_MINIGAME_FIRSTTODIEMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class FirstToDieMedal : public GameMedal
{
public:
    FirstToDieMedal(int name, float target, unsigned int weight);

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
