#ifndef GAME_WORLD_BIOME_EXTREMEHILLSBIOME_HPP
#define GAME_WORLD_BIOME_EXTREMEHILLSBIOME_HPP

#include "game/world/biome/Biome.hpp"

class OreFeature;

class ExtremeHillsBiome : public Biome
{
public:
    enum EType
    {
        eType_Normal = 0,
        eType_ExtraTrees = 1,
        eType_Mutated = 2
    };

    ExtremeHillsBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties);
    virtual ~ExtremeHillsBiome();

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual EBiomeBaseClass getBaseClass();

private:
    OreFeature* mMonsterEggFeature;
    EType mType;
};

#endif
