#ifndef GAME_WORLD_GENERATION_MINESHAFTFEATURE_HPP
#define GAME_WORLD_GENERATION_MINESHAFTFEATURE_HPP

#include <string>

#include "game/world/generation/StructureFeature.hpp"

class BlockPos;
class Level;
class SuperflatConfig;
class StructureStart;

class MineShaftFeature : public StructureFeature
{
public:
    enum MineShaftType
    {
        eMineShaftType_Normal = 0,
        eMineShaftType_Mesa = 1
    };

    MineShaftFeature();
    MineShaftFeature(SuperflatConfig* config);

    virtual std::wstring getFeatureName();
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);

private:
    double mChance;
};

#endif
