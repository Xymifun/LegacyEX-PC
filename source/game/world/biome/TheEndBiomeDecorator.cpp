#include "game/world/biome/TheEndBiomeDecorator.hpp"

#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/entity/mob/EndDragonFight.hpp"
#include "game/world/generation/EndPodiumFeature.hpp"
#include "game/util/Random.hpp"

struct EndSpikeData
{
    int posX;
    int posZ;
    int centerX;
    int centerZ;
    int radius;
    int height;
    bool guarded;
};

static const EndSpikeData spikeData[8] = {{32, -16, 40, 0, 2, 12, false},   {16, 16, 28, 28, 2, 15, false},
                                          {-16, 32, 0, 40, 2, 18, false},   {-32, 16, -28, 28, 3, 21, false},
                                          {-48, -16, -40, 0, 3, 24, false}, {-32, -32, -28, -28, 3, 27, false},
                                          {-16, -48, 0, -40, 4, 30, true},  {16, -32, 28, -28, 4, 33, true}};

TheEndBiomeDecorator::TheEndBiomeDecorator()
    : BiomeDecorator(),
      spikeGen(new SpikeFeature()),
      endPodiumGen(new EndPodiumFeature(false))
{
}

void TheEndBiomeDecorator::decorate(Biome* biome, Level* level, Random* random)
{
    decorateOres(level, random);

    for (unsigned int i = 0; i < 8; ++i)
    {
        if (position.x == spikeData[i].posX && position.z == spikeData[i].posZ)
        {
            SpikeFeature::EndSpike* spike = new SpikeFeature::EndSpike(
                spikeData[i].centerX, spikeData[i].centerZ, spikeData[i].radius,
                spikeData[i].height + EndPodiumFeature::END_PODIUM_LOCATION.y, spikeData[i].guarded);
            spikeGen->setSpike(spike);
            spikeGen->place(
                level, *random,
                BlockPos(spikeData[i].centerX, EndPodiumFeature::END_PODIUM_LOCATION.y, spikeData[i].centerZ));
        }
    }

    if (position.x == 0 && position.z == 0)
    {
        TheEndDimension* dimension = dynamic_cast<TheEndDimension*>(level->mDimension);
        if (dimension != NULL)
        {
            dimension->getDragonFight()->createNewDragon();
        }
    }
}

arrayWithLength<SpikeFeature::EndSpike> TheEndBiomeDecorator::getSpikesForLevel(Level* level)
{
    (void)level;

    arrayWithLength<SpikeFeature::EndSpike> spikes(8, true);
    for (unsigned int i = 0; i < spikes.length; ++i)
    {
        SpikeFeature::EndSpike spike(spikeData[i].centerX, spikeData[i].centerZ, spikeData[i].radius,
                                     spikeData[i].height + EndPodiumFeature::END_PODIUM_LOCATION.y,
                                     spikeData[i].guarded);
        spikes.data[i] = spike;
        spike = SpikeFeature::EndSpike();
    }
    return spikes;
}
