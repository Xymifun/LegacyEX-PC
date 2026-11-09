#ifndef GAME_WORLD_GENERATION_MAPGENERATOR_HPP
#define GAME_WORLD_GENERATION_MAPGENERATOR_HPP

#include <stdint.h>
#include <vector>

#include "game/core/GenerationWriteTask.hpp"
#include "game/util/Random.hpp"

class Level;
class AABB;

class MapGenerator
{
public:
    MapGenerator(Random random);
    virtual ~MapGenerator();

    virtual void StartGeneration(Level* level) = 0;
    virtual AABB* GetSpawnArea() const = 0;
    virtual void CleanUp(Level* level) = 0;
    virtual AABB* GetLayerBB(unsigned int layer) = 0;
    virtual int GetNumberOfLayers() = 0;
    virtual void GetLayerExtents(unsigned int layer, float* minimum, float* maximum) = 0;
    virtual unsigned int GetBlockCountOnLayer(unsigned int layer);

    bool FinishedGeneration() const;
    bool ContinueGeneration();

protected:
    Random mRandom;
    std::vector<GenerationWriteTask*> mGenerationWriteTasks;
};

#endif
