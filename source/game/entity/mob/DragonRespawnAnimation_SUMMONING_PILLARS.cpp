#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_PILLARS.hpp"

#include <typeinfo>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/TheEndBiomeDecorator.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/SpikeFeature.hpp"

void DragonRespawnAnimation_SUMMONING_PILLARS::tick(ServerLevel* level, EndDragonFight* dragonFight,
                                                    std::vector<boost::shared_ptr<EnderCrystal> >* crystals, int tick,
                                                    const BlockPos& portalLocation)
{
    bool targetSpike = tick % 40 == 0;
    bool rebuildSpike = tick % 40 == 39;

    if (targetSpike || rebuildSpike)
    {
        arrayWithLength<SpikeFeature::EndSpike> spikes = TheEndBiomeDecorator::getSpikesForLevel(level);

        if (tick / 40 < static_cast<int>(spikes.length))
        {
            SpikeFeature::EndSpike& spike = spikes.data[tick / 40];

            if (targetSpike)
            {
                std::vector<boost::shared_ptr<EnderCrystal> >::iterator it = crystals->begin();
                while (it != crystals->end())
                {
                    BlockPos beamTarget(static_cast<int>(spike.getCenterX()), static_cast<int>(spike.getHeight()) + 1,
                                        static_cast<int>(spike.getCenterZ()));
                    boost::shared_ptr<EnderCrystal> crystal = *it;
                    crystal->setBeamTarget(new BlockPos(beamTarget));
                    ++it;
                }
            }
            else
            {
                BlockPos from(static_cast<int>(spike.getCenterX()) - 10, static_cast<int>(spike.getHeight()) - 10,
                              static_cast<int>(spike.getCenterZ()) - 10);
                BlockPos to(static_cast<int>(spike.getCenterX()) + 10, static_cast<int>(spike.getHeight()) + 10,
                            static_cast<int>(spike.getCenterZ()) + 10);
                AABB* box = AABB::newTemp(from, to);

                std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(EnderCrystal), box);
                std::vector<boost::shared_ptr<Entity> >::iterator entity = entities->begin();
                while (entity != entities->end())
                {
                    (*entity)->setInvulnerable(false);
                    ++entity;
                }
                delete entities;

                Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(from, to);
                while (positions->hasNext())
                {
                    level->removeBlock(*positions->next());
                }
                delete positions;

                level->explode(boost::shared_ptr<Entity>(),
                               static_cast<double>(static_cast<float>(static_cast<int>(spike.getCenterX())) + 0.5f),
                               static_cast<double>(static_cast<int>(spike.getHeight())),
                               static_cast<double>(static_cast<float>(static_cast<int>(spike.getCenterZ())) + 0.5f),
                               5.0f, true, true, false, false, false, -1.0f);

                SpikeFeature feature;
                feature.setSpike(new SpikeFeature::EndSpike(spike));
                feature.setCrystalInvulnerable(true);

                feature.setCrystalBeamTarget(BlockPos(0, 128, 0));

                Random random;
                feature.place(level, random,
                              BlockPos(static_cast<int>(spike.getCenterX()), 45, static_cast<int>(spike.getCenterZ())));
            }
        }
        else if (targetSpike)
        {
            dragonFight->setRespawnStage(DragonRespawnAnimation::SUMMONING_DRAGON);
        }
        delete[] spikes.data;
    }
}
