#ifndef GAME_WORLD_BLOCK_BLOCKPLACEDOVERTIME_HPP
#define GAME_WORLD_BLOCK_BLOCKPLACEDOVERTIME_HPP

#include <map>

#include "game/minigame/GameMedal.hpp"

class BlockPlacedOverTime : public GameMedal
{
public:
    typedef float ScoreFunction(int, const std::map<int, int>&);

    BlockPlacedOverTime(int name, float target, unsigned int weight, ScoreFunction& scoreFunction,
                        GameMedal::EOperator op);

    static float START_DELAY(int lastRound, const std::map<int, int>& blocksPlaced);
    static float TIME_LEFTOVER(int lastRound, const std::map<int, int>& blocksPlaced);
    static float IN_FIRST_10SECS(int lastRound, const std::map<int, int>& blocksPlaced);
    static float IN_LAST_10SECS(int lastRound, const std::map<int, int>& blocksPlaced);

    virtual float GetScore(PlayerStats* playerStats) const;

private:
    ScoreFunction* mScoreFunction;
};

#endif
