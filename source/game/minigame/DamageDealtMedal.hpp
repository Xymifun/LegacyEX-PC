#ifndef GAME_MINIGAME_DAMAGEDEALTMEDAL_HPP
#define GAME_MINIGAME_DAMAGEDEALTMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class DamageEvent;
class PlayerStats;

class DamageDealtMedal : public GameMedal
{
public:
    DamageDealtMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                     bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
    virtual bool IncludeDamage(const DamageEvent* damageEvent) const;
};

#endif
