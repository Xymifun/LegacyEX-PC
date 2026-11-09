#ifndef GAME_WORLD_GENERATION_WOODLANDMANSIONFEATURE_HPP
#define GAME_WORLD_GENERATION_WOODLANDMANSIONFEATURE_HPP

#include <string>

#include "game/world/biome/FjFeatureBiomeRequirements.hpp"
#include "game/world/chunk/FjIsFeatureChunk.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"

class BiomeSource;
class BlockPos;
class GenericOverworldLevelSource;
class Level;
class Random;
class SuperflatConfig;

class WoodlandMansionFeature : public StructureFeature
{
public:
    class WoodlandMansionStart : public StructureStart
    {
    public:
        WoodlandMansionStart(WoodlandMansionFeature* feature, Level* level, GenericOverworldLevelSource* source,
                             Random& random, int chunkX, int chunkZ);
        WoodlandMansionStart();

        static StructureStart* Create();
        void create(Level* level, GenericOverworldLevelSource* source, Random& random, int chunkX, int chunkZ);
        virtual void postProcess(Level* level, Random& random, BoundingBox* box);
        virtual bool isValid();
        virtual EStructureStart GetType();

    private:
        WoodlandMansionFeature* mFeature;
        bool mValid;
    };

    WoodlandMansionFeature(GenericOverworldLevelSource* source);
    WoodlandMansionFeature(GenericOverworldLevelSource* source, SuperflatConfig* config);

    static void staticCtor();
    virtual std::wstring getFeatureName();
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    static char fjIsFeatureChunk(BiomeSource* biomeSource, long long seed, int sizeInChunks, int chunkX, int chunkZ);
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);

private:
    int mSpacing;
    int mSeparation;
    FjIsFeatureChunk mFeatureChunk;
    FjIsFeatureChunk mReducedFeatureChunk;
    GenericOverworldLevelSource* mSource;
};

#endif
