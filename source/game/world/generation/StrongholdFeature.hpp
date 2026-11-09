#ifndef GAME_WORLD_GENERATION_STRONGHOLDFEATURE_HPP
#define GAME_WORLD_GENERATION_STRONGHOLDFEATURE_HPP

#include <string>
#include <vector>

#include "game/core/StrongholdPieces.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"

class Biome;
class BlockPos;
class ChunkPos;
class Level;
class Random;

class StrongholdFeature : public StructureFeature
{
public:
    StrongholdFeature();
    StrongholdFeature(SuperflatConfig* config);
    virtual ~StrongholdFeature();

    void _init();
    virtual std::wstring getFeatureName();
    void generatePositions();
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    virtual std::vector<BlockPos>* getGuesstimatedFeaturePositions();
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);

    class StrongholdStart : public StructureStart
    {
    public:
        StrongholdStart(Level* level, Random& random, int chunkX, int chunkZ);
        StrongholdStart();
        virtual EStructureStart GetType();
        static StructureStart* Create();
    };

private:
#ifdef MINECRAFT_PC
    bool GenerateRingPositions();
#endif
    std::vector<Biome*> mAllowedBiomes;
    bool mPositionsGenerated;
    std::vector<ChunkPos*> mPositions;
    double mDistance;
    int mVariant;
};

#endif
