#ifndef GAME_WORLD_BIOME_FORESTBIOME_HPP
#define GAME_WORLD_BIOME_FORESTBIOME_HPP

#include "game/world/biome/Biome.hpp"

class AbstractTreeFeature;
class BlockPos;
class Level;
class Random;

class ForestBiome : public Biome
{
public:
    enum EType
    {
        eType_Normal = 0,
        eType_Flower = 1,
        eType_Birch = 2,
        eType_Roofed = 3
    };

    ForestBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties);
    virtual ~ForestBiome() {}

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual FlowerBlock::Flower* getRandomFlower(Random& random, const BlockPos& pos);
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual unsigned int getGrassColor(const BlockPos& pos);
    virtual EBiomeBaseClass getBaseClass();

private:
    void addMushrooms(Level* level, Random& random, const BlockPos& pos);
    void addDoublePlants(Level* level, Random& random, const BlockPos& pos, int count);

    EType mType;
};

#endif
