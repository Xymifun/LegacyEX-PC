#ifndef GAME_WORLD_BLOCK_PODIUMPLACEMOSTBLOCKSDESTROYEDMEDAL_HPP
#define GAME_WORLD_BLOCK_PODIUMPLACEMOSTBLOCKSDESTROYEDMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class PodiumPlaceMostBlocksDestroyedMedal : public GameMedal
{
public:
    enum EPodium
    {
        PODIUM_FIRST,
        PODIUM_SECOND,
        PODIUM_THIRD
    };

    PodiumPlaceMostBlocksDestroyedMedal(EPodium podium, int name, float target, unsigned int weight,
                                        GameMedal::EOperator op);

    virtual float GetScore(PlayerStats* playerStats) const;

private:
    EPodium mPodium;
};

#endif
