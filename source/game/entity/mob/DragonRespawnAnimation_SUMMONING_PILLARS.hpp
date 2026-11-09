#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_SUMMONING_PILLARS_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_SUMMONING_PILLARS_HPP

#include "game/entity/mob/DragonRespawnAnimation.hpp"

class DragonRespawnAnimation_SUMMONING_PILLARS : public DragonRespawnAnimation
{
public:
    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
                      std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                      const BlockPos& portalLocation);
};

#endif
