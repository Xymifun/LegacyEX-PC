#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_START_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_START_HPP

#include "game/entity/mob/DragonRespawnAnimation.hpp"

class DragonRespawnAnimation_START : public DragonRespawnAnimation
{
public:
    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
                      std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                      const BlockPos& portalLocation);
};

#endif
