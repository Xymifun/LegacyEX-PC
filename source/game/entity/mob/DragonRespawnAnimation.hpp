#ifndef GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_HPP
#define GAME_ENTITY_MOB_DRAGONRESPAWNANIMATION_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

class BlockPos;
class EndDragonFight;
class EnderCrystal;
class ServerLevel;

class DragonRespawnAnimation
{
public:
    static DragonRespawnAnimation* START;
    static DragonRespawnAnimation* PREPARING_TO_SUMMON_PILLARS;
    static DragonRespawnAnimation* SUMMONING_PILLARS;
    static DragonRespawnAnimation* SUMMONING_DRAGON;
    static DragonRespawnAnimation* END;

    virtual void tick(ServerLevel* level, EndDragonFight* dragonFight,
                      std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                      const BlockPos& portalLocation) = 0;
};

#endif
