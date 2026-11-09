#ifndef GAME_WORLD_GENERATION_SHIPWRECKFEATURE_HPP
#define GAME_WORLD_GENERATION_SHIPWRECKFEATURE_HPP

#include <string>
#include <vector>

#include "game/world/biome/FjFeatureBiomeRequirements.hpp"
#include "game/world/chunk/FjIsFeatureChunk.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"

class Biome;
class BiomeSource;
class BlockPos;
class GenericOverworldLevelSource;
class Level;
class OceanMonumentFeature;
class Random;
class SuperflatConfig;

class ShipwreckFeature : public StructureFeature
{
public:
    class ShipwreckStart : public StructureStart
    {
    public:
        ShipwreckStart(Level* level, GenericOverworldLevelSource* source, Random& random, int chunkX, int chunkZ);
        ShipwreckStart();

        void create(Level* level, GenericOverworldLevelSource* source, Random& random, int chunkX, int chunkZ);
        virtual EStructureStart GetType();
        static StructureStart* Create();
    };

    ShipwreckFeature(GenericOverworldLevelSource* source, OceanMonumentFeature* monument);
    ShipwreckFeature(GenericOverworldLevelSource* source, OceanMonumentFeature* monument, SuperflatConfig* config);

    static std::vector<Biome*>* GetSpawnBiomes();
    static std::vector<Biome*>* GetAllowedBiomes();
    static FjFeatureBiomeRequirements* GetFJBiomeRequirements();
    virtual std::wstring getFeatureName();
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    static char fjIsFeatureChunk(BiomeSource* biomeSource, long long seed, int sizeInChunks, int chunkX, int chunkZ);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);

private:
    GenericOverworldLevelSource* mSource;
    FjIsFeatureChunk mFeatureChunk;
    FjIsFeatureChunk mReducedFeatureChunk;
    int mSpacing;
    int mSeparation;
    OceanMonumentFeature* mOceanMonument;
};

#endif
