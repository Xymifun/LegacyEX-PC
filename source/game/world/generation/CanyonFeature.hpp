#ifndef GAME_WORLD_GENERATION_CANYONFEATURE_HPP
#define GAME_WORLD_GENERATION_CANYONFEATURE_HPP

#include "game/world/generation/LargeFeature.hpp"

class ChunkPrimer;
class Level;

class CanyonFeature : public LargeFeature
{
public:
    void addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                   double z, float thickness, float yaw, float pitch, int startIndex, int endIndex, double yScale);

    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);

private:
    float rs[1024];
};

#endif
