#ifndef GAME_MINIGAME_TREASUREHUNTERMEDAL_HPP
#define GAME_MINIGAME_TREASUREHUNTERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class TreasureHunterMedal : public GameMedal
{
public:
    TreasureHunterMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                        bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
