#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_DRAGON.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/core/EnderCrystal.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

void DragonRespawnAnimation_SUMMONING_DRAGON::tick(ServerLevel* level, EndDragonFight* dragonFight,
                                                   std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                                                   const BlockPos& portalLocation)
{
    if (tick < 100)
    {
        if (tick < 80)
        {
            if (tick == 0)
            {
                std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
                while (it != crystals->end())
                {
                    boost::shared_ptr<EnderCrystal> crystal = *it;
                    crystal->setBeamTarget(new BlockPos(0, 128, 0));
                    ++it;
                }
            }
            else if (tick < 5)
            {
                level->levelEvent(LevelEvent::ANIMATION_DRAGON_SUMMON_ROAR, BlockPos(0, 128, 0), 0);
            }
        }
        else
        {
            level->levelEvent(LevelEvent::ANIMATION_DRAGON_SUMMON_ROAR, BlockPos(0, 128, 0), 0);
        }
        return;
    }

    dragonFight->setRespawnStage(DragonRespawnAnimation::END);
    dragonFight->resetSpikeCrystals();

    std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
    while (it != crystals->end())
    {
        boost::shared_ptr<EnderCrystal> crystal = *it;
        crystal->setBeamTarget(NULL);
        level->explode(crystal, crystal->mX, crystal->mY, crystal->mZ, 6.0f, false, true, false, false, false, -1.0f);
        crystal->remove();
        ++it;
    }
}
