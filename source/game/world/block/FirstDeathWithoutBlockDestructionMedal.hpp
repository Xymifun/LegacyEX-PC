#ifndef GAME_WORLD_BLOCK_FIRSTDEATHWITHOUTBLOCKDESTRUCTIONMEDAL_HPP
#define GAME_WORLD_BLOCK_FIRSTDEATHWITHOUTBLOCKDESTRUCTIONMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class FirstDeathWithoutBlockDestructionMedal : public GameMedal
{
public:
    FirstDeathWithoutBlockDestructionMedal(int name, float target, unsigned int weight, GameMedal::EOperator op,
                                           bool bestOnly, bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
