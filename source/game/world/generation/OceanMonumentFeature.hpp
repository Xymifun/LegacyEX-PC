#ifndef GAME_WORLD_GENERATION_OCEANMONUMENTFEATURE_HPP
#define GAME_WORLD_GENERATION_OCEANMONUMENTFEATURE_HPP

#include <string>
#include <vector>

#include "game/core/ColourTable.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/FjIsFeatureChunk.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"

class BiomeSource;
class BlockPos;
class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;
class Random;
class SuperflatConfig;

class OceanMonumentFeature : public StructureFeature
{
public:
    class OceanMonumentStart : public StructureStart
    {
    public:
        OceanMonumentStart(Level* level, Random& random, int chunkX, int chunkZ);
        OceanMonumentStart();

        void Init();
        void create(Level* level, Random& random, int chunkX, int chunkZ);
        virtual void postProcess(Level* level, Random& random, BoundingBox* box);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input);
        virtual bool isValidForPostProcess(ChunkPos* chunkPos);
        virtual void notifyPostProcessAt(ChunkPos* chunkPos);
        virtual EStructureStart GetType();

        static StructureStart* Create();

    private:
        std::vector<ChunkPos> mProcessedChunks;
        bool mCreated;
    };

    OceanMonumentFeature();
    OceanMonumentFeature(SuperflatConfig* config);
    virtual ~OceanMonumentFeature();

    static void staticCtor();
    virtual std::wstring getFeatureName();
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);

    static char fjIsFeatureChunk(BiomeSource* biomeSource, long long seed, int sizeInChunks, int chunkX, int chunkZ);
    std::vector<Biome::MobSpawnerData*>* getEnemies();

private:
    int mSpacing;
    int mSeparation;
    FjIsFeatureChunk mFeatureChunk;
    FjIsFeatureChunk mReducedFeatureChunk;
};

#endif
