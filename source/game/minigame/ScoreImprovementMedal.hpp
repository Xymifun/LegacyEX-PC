#ifndef GAME_MINIGAME_SCOREIMPROVEMENTMEDAL_HPP
#define GAME_MINIGAME_SCOREIMPROVEMENTMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class ScoreImprovementMedal : public GameMedal
{
public:
    ScoreImprovementMedal(int name, float target, unsigned int weight, GameMedal::EOperator op);

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
