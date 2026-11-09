#ifndef GAME_WORLD_BLOCK_BLOCKDESTROYEDMEDAL_HPP
#define GAME_WORLD_BLOCK_BLOCKDESTROYEDMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class BlockDestroyedMedal : public GameMedal
{
public:
    BlockDestroyedMedal(int name, float target, unsigned int weight, GameMedal::EOperator op);
    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
