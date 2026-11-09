#ifndef GAME_WORLD_GENERATION_TIMESPENTONLAYERMEDAL_HPP
#define GAME_WORLD_GENERATION_TIMESPENTONLAYERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class TimeSpentOnLayerMedal : public GameMedal
{
public:
    enum ELayerMedal
    {
        LAYER_MEDAL_TOP,
        LAYER_MEDAL_BOTTOM
    };

    TimeSpentOnLayerMedal(ELayerMedal layer, int name, float target, unsigned int weight, GameMedal::EOperator op);

    virtual float GetScore(PlayerStats* playerStats) const;

private:
    int mLayer;
};

#endif
