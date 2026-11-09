#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_PREPARING_TO_SUMMON_PILLARS_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_PREPARING_TO_SUMMON_PILLARS_HPP

#include "game/entity/mob/DragonRespawnAnimation.hpp"

class DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS : public DragonRespawnAnimation
{
public:
    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
                      std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                      const BlockPos& portalLocation);
};

#endif
