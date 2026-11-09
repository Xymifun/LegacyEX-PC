#ifndef GAME_WORLD_BIOME_TAIGABIOME_HPP
#define GAME_WORLD_BIOME_TAIGABIOME_HPP

#include "game/world/biome/Biome.hpp"

class TaigaBiome : public Biome
{
public:
    enum EType
    {
        eType_Normal = 0,
        eType_MegaTaiga = 1,
        eType_Mutated = 2
    };

    TaigaBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties);

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual Feature* getGrassFeature(Random& random);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual EBiomeBaseClass getBaseClass();

private:
    EType mType;
};

#endif
