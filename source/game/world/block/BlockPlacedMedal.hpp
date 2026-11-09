#ifndef GAME_WORLD_BLOCK_BLOCKPLACEDMEDAL_HPP
#define GAME_WORLD_BLOCK_BLOCKPLACEDMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class BlockPlacedMedal : public GameMedal
{
public:
    BlockPlacedMedal(int name, float target, unsigned int weight, GameMedal::EOperator op);


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
