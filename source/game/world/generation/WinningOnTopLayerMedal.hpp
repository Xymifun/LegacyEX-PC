#ifndef GAME_WORLD_GENERATION_WINNINGONTOPLAYERMEDAL_HPP
#define GAME_WORLD_GENERATION_WINNINGONTOPLAYERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class WinningOnTopLayerMedal : public GameMedal
{
public:
    WinningOnTopLayerMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                           bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
