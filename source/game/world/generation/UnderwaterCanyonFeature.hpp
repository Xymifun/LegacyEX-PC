#ifndef GAME_WORLD_GENERATION_UNDERWATERCANYONFEATURE_HPP
#define GAME_WORLD_GENERATION_UNDERWATERCANYONFEATURE_HPP

#include "game/world/generation/LargeFeature.hpp"

class ChunkPrimer;
class Level;

class UnderwaterCanyonFeature : public LargeFeature
{
public:
    virtual void apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer);
    void addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                   double z, float thickness, float yaw, float pitch, int startIndex, int endIndex, double yScale);
    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);

private:
    float rs[1024];
};

#endif
