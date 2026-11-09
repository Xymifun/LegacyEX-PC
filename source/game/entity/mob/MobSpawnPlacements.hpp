#ifndef GAME_ENTITY_MOB_MOBSPAWNPLACEMENTS_HPP
#define GAME_ENTITY_MOB_MOBSPAWNPLACEMENTS_HPP

#include "game/entity/mob/Mob.hpp"

class MobSpawnPlacements
{
public:
    static Mob::SpawnPlacementType getPlacementType(unsigned long long entityType);
};

#endif
