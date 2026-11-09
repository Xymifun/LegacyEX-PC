#ifndef GAME_WORLD_BIOME_ICEBIOME_HPP
#define GAME_WORLD_BIOME_ICEBIOME_HPP

#include "game/world/biome/Biome.hpp"

class IcePatchFeature;
class IceSpikeFeature;

class IceBiome : public Biome
{
public:
    IceBiome(EBiomeIDs biomeId, bool hasIceSpikes, BiomeProperties* properties);

    virtual float getCreatureProbability();
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual EBiomeBaseClass getBaseClass();

private:
    bool mHasIceSpikes;
    IceSpikeFeature* mIceSpikeFeature;
    IcePatchFeature* mIcePatchFeature;
};

#endif
