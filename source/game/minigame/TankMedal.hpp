#ifndef GAME_MINIGAME_TANKMEDAL_HPP
#define GAME_MINIGAME_TANKMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class TankMedal : public GameMedal
{
public:
    TankMedal(int name, float target, unsigned int weight);


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
