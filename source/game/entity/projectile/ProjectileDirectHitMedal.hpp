#ifndef GAME_ENTITY_PROJECTILE_PROJECTILEDIRECTHITMEDAL_HPP
#define GAME_ENTITY_PROJECTILE_PROJECTILEDIRECTHITMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class ProjectileDirectHitMedal : public GameMedal
{
public:
    ProjectileDirectHitMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly,
                             bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }


    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
