#ifndef GAME_WORLD_GENERATION_TUMBLEGENERATOR_HPP
#define GAME_WORLD_GENERATION_TUMBLEGENERATOR_HPP

#include <vector>

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MapGenerator.hpp"

class AABB;
class LayerGenerator;
class Level;

class TumbleGenerator : public MapGenerator
{
public:
    TumbleGenerator(Random random, std::vector<LayerGenerator*>& layers);
    virtual ~TumbleGenerator();

    virtual void StartGeneration(Level* level);
    virtual AABB* GetSpawnArea() const;
    virtual void CleanUp(Level* level);
    virtual AABB* GetLayerBB(unsigned int layer);
    virtual int GetNumberOfLayers();
    virtual void GetLayerExtents(unsigned int layer, float* minimum, float* maximum);
    virtual unsigned int GetBlockCountOnLayer(unsigned int layer);

    AABB* GetSpawnArea(unsigned int layer) const;
    void AddAdditionalBlock(BlockPos position);

private:
    std::vector<LayerGenerator*> mLayers;
    std::vector<int> mBlockCounts;
    std::vector<BlockPos> mAdditionalBlocks;
};

#endif
