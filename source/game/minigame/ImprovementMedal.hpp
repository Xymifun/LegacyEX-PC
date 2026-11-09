#ifndef GAME_MINIGAME_IMPROVEMENTMEDAL_HPP
#define GAME_MINIGAME_IMPROVEMENTMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class ImprovementMedal : public GameMedal
{
public:
    ImprovementMedal(int name, float target, unsigned int weight);

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
