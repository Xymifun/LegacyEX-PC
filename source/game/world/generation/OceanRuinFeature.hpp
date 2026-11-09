#ifndef GAME_WORLD_GENERATION_OCEANRUINFEATURE_HPP
#define GAME_WORLD_GENERATION_OCEANRUINFEATURE_HPP

#include <string>
#include <vector>

#include "game/core/OceanRuin.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"

class BlockPos;
class Direction;
class Level;
class Random;
class StructurePiece;
class SuperflatConfig;

struct OceanRuinConfiguration
{
    OceanRuinConfiguration()
        : biomeType(OceanRuin::eRuinType_Cold),
          smallProbability(0.0f),
          clusterProbability(0.0f),
          enabled(false)
    {
    }

    OceanRuin::ERuinType biomeType;
    float smallProbability;
    float clusterProbability;
    bool enabled;
};

class OceanRuinFeature : public StructureFeature
{
public:
    class OceanRuinStart : public StructureStart
    {
    public:
        OceanRuinStart();
        OceanRuinStart(Level* level, Random& random, int chunkX, int chunkZ,
                       const OceanRuinConfiguration& configuration);

        std::vector<BlockPos>* allPositions(Random& random, int x, int z);
        void addClusterRuins(Random& random, const Direction* direction, int x, int z, bool warm,
                             std::vector<StructurePiece*>& pieces);
        void create(Random& random, int chunkX, int chunkZ, const OceanRuinConfiguration& configuration);
        virtual EStructureStart GetType();

    private:
        Level* mLevel;
    };

    OceanRuinFeature();
    OceanRuinFeature(SuperflatConfig* config);

    static bool IsValidSpawnArea(Level* level, int x, int z, int radius);

    virtual std::wstring getFeatureName();
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);

private:
    int mSpacing;
    int mSeparation;
};

#endif
