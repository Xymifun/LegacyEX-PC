#ifndef GAME_MINIGAME_GAMEVOTEMEDAL_HPP
#define GAME_MINIGAME_GAMEVOTEMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class GameVoteMedal : public GameMedal
{
public:
    class Range
    {
    public:
        float getScore(float score) const;

        float mTarget;
        float mRange;
    };

    GameVoteMedal(int name, float target, unsigned int weight, Range* ranges);

    virtual float GetScore(PlayerStats* playerStats) const;

private:
    Range mRanges[4];
};

#endif
