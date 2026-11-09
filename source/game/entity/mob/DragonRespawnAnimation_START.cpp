#include "game/entity/mob/DragonRespawnAnimation_START.hpp"

#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

void DragonRespawnAnimation_START::tick(ServerLevel* level, EndDragonFight* dragonFight,
                                        std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                                        const BlockPos& portalLocation)
{
    BlockPos beamTarget(0, 128, 0);

    if (!crystals->empty())
    {
        std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
        while (it != crystals->end())
        {
            boost::shared_ptr<EnderCrystal> crystal = *it;
            crystal->setBeamTarget(new BlockPos(beamTarget));
            ++it;
        }
    }

    dragonFight->setRespawnStage(DragonRespawnAnimation::PREPARING_TO_SUMMON_PILLARS);
}
