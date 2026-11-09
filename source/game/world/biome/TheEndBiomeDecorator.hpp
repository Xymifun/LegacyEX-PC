#ifndef GAME_WORLD_BIOME_THEENDBIOMEDECORATOR_HPP
#define GAME_WORLD_BIOME_THEENDBIOMEDECORATOR_HPP

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/SpikeFeature.hpp"

class Biome;
class EndPodiumFeature;
class Level;
class Random;

class TheEndBiomeDecorator : public BiomeDecorator
{
public:
    TheEndBiomeDecorator();
    virtual void decorate(Biome* biome, Level* level, Random* random);
    static arrayWithLength<SpikeFeature::EndSpike> getSpikesForLevel(Level* level);

private:
    SpikeFeature* spikeGen;
    EndPodiumFeature* endPodiumGen;
};

#endif
