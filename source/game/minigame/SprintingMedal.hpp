#ifndef GAME_MINIGAME_SPRINTINGMEDAL_HPP
#define GAME_MINIGAME_SPRINTINGMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class SprintingMedal : public GameMedal
{
public:
    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
