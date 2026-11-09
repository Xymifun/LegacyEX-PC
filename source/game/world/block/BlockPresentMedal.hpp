#ifndef GAME_WORLD_BLOCK_BLOCKPRESENTMEDAL_HPP
#define GAME_WORLD_BLOCK_BLOCKPRESENTMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class BlockPresentMedal : public GameMedal
{
public:
    BlockPresentMedal(int name, float target, unsigned int weight, GameMedal::EOperator op);
    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
