#ifndef GAME_ENTITY_MOB_SHOULDERRIDINGENTITY_HPP
#define GAME_ENTITY_MOB_SHOULDERRIDINGENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"

class Level;
class Player;

class ShoulderRidingEntity : public TamableAnimal
{
public:
    ShoulderRidingEntity(Level* level);
    virtual ~ShoulderRidingEntity() {}
    bool setEntityOnShoulder(boost::shared_ptr<Player> player);
    virtual void tick();
    bool canSitOnShoulder();

protected:
    int mShoulderTick;
};

#endif
