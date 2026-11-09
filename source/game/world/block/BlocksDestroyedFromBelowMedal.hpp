#ifndef GAME_WORLD_BLOCK_BLOCKSDESTROYEDFROMBELOWMEDAL_HPP
#define GAME_WORLD_BLOCK_BLOCKSDESTROYEDFROMBELOWMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class BlocksDestroyedFromBelowMedal : public GameMedal
{
public:
    BlocksDestroyedFromBelowMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                                  bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
