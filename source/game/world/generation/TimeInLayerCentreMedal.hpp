#ifndef GAME_WORLD_GENERATION_TIMEINLAYERCENTREMEDAL_HPP
#define GAME_WORLD_GENERATION_TIMEINLAYERCENTREMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class TimeInLayerCentreMedal : public GameMedal
{
public:
    TimeInLayerCentreMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                           bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
