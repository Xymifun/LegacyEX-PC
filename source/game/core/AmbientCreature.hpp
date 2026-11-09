#ifndef GAME_CORE_AMBIENTCREATURE_HPP
#define GAME_CORE_AMBIENTCREATURE_HPP

#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Creature.hpp"

class AmbientCreature : public Mob, public Creature
{
public:
    AmbientCreature(Level* level);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
};

#endif
