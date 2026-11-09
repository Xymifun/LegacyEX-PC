#ifndef GAME_WORLD_GENERATION_BURIEDTREASUREFEATURE_HPP
#define GAME_WORLD_GENERATION_BURIEDTREASUREFEATURE_HPP

#include <string>
#include <vector>

#include "game/world/chunk/FjIsFeatureChunk.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"
#include "game/world/generation/TemplateStructurePiece.hpp"

class Biome;
class BlockPos;
class BoundingBox;
class Level;
class Random;

class BuriedTreasureFeature : public StructureFeature
{
public:
    class BuriedTreasurePiece : public TemplateStructurePiece
    {
    public:
        BuriedTreasurePiece(int x, int z);

        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        virtual void handleDataMarker(const std::wstring& marker, const BlockPos& position, Level* level,
                                      Random& random, BoundingBox* box);
        virtual EStructurePiece GetType();
    };

    class BuriedTreasureStart : public StructureStart
    {
    public:
        BuriedTreasureStart(BuriedTreasureFeature* feature, Level* level, Random& random, int chunkX, int chunkZ);
        BuriedTreasureStart();

        virtual EStructureStart GetType();
        static StructureStart* Create();

    private:
        BuriedTreasureFeature* mFeature;
    };

    BuriedTreasureFeature(int sizeInChunks);

    static void staticCtor();
    virtual std::wstring getFeatureName();
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);

private:
    FjIsFeatureChunk mFeatureChunk;
    FjIsFeatureChunk mReducedFeatureChunk;
};

#endif
