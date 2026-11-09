#ifndef GAME_WORLD_BLOCK_BLOCKSPRESENTWITHIDMEDAL_HPP
#define GAME_WORLD_BLOCK_BLOCKSPRESENTWITHIDMEDAL_HPP

#include <set>

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class BlocksPresentWithIdMedal : public GameMedal
{
public:
    BlocksPresentWithIdMedal(int name, float target, unsigned int weight, const std::set<int>& blockIds,
                             float (&scoreFunction)(int, int), float value, GameMedal::EOperator op);


    virtual float GetScore(PlayerStats* playerStats) const;

    static float ABSOLUTE_VALUE(int value, int total);
    static float PERCENTAGE_OF_TOTAL(int value, int total);

private:
    std::set<int> mBlockIds;
    float (*mScoreFunction)(int, int);
};

#endif
