#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_SUMMONING_DRAGON_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_SUMMONING_DRAGON_HPP

#include "game/entity/mob/DragonRespawnAnimation.hpp"

class DragonRespawnAnimation_SUMMONING_DRAGON : public DragonRespawnAnimation
{
public:
    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
                      std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                      const BlockPos& portalLocation);
};

#endif
