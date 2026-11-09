#ifndef GAME_ENTITY_PROJECTILE_HITSFROMPROJECTILEMEDAL_HPP
#define GAME_ENTITY_PROJECTILE_HITSFROMPROJECTILEMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class PlayerStats;

class HitsFromProjectileMedal : public GameMedal
{
public:
    HitsFromProjectileMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                            bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
