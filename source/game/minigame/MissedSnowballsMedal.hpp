#ifndef GAME_MINIGAME_MISSEDSNOWBALLSMEDAL_HPP
#define GAME_MINIGAME_MISSEDSNOWBALLSMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class MissedSnowballsMedal : public GameMedal
{
public:
    MissedSnowballsMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                         bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
