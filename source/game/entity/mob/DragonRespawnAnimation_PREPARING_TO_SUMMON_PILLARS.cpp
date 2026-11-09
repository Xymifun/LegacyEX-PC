#include "game/entity/mob/DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/entity/mob/EndDragonFight.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPos.hpp"

void DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS::tick(ServerLevel* level, EndDragonFight* dragonFight,
                                                              std::vector<boost::shared_ptr<EnderCrystal> >* crystals,
                                                              int tick, const BlockPos& portalLocation)
{
    if (tick < 100)
    {
        if (tick == 0 || tick == 50 || tick == 51 || tick == 52 || tick >= 95)
        {
            level->levelEvent(LevelEvent::ANIMATION_DRAGON_SUMMON_ROAR, BlockPos(0, 128, 0), 0);
        }
    }
    else
    {
        dragonFight->setRespawnStage(DragonRespawnAnimation::SUMMONING_PILLARS);
    }
}
