#ifndef GAME_WORLD_BLOCK_BLOCKTYPESUSED_HPP
#define GAME_WORLD_BLOCK_BLOCKTYPESUSED_HPP

#include "game/minigame/GameMedal.hpp"

class BlockTypesUsed : public GameMedal
{
public:
    BlockTypesUsed(int name, float target, unsigned int weight, GameMedal::EOperator op);
    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
