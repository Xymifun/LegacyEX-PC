#ifndef GAME_WORLD_GENERATION_LARGEHELLCAVEFEATURE_HPP
#define GAME_WORLD_GENERATION_LARGEHELLCAVEFEATURE_HPP

#include "game/world/generation/LargeFeature.hpp"

class LargeHellCaveFeature : public LargeFeature
{
public:
    virtual ~LargeHellCaveFeature();

    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);

protected:
    void addTunnel(long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z,
                   float width, float yaw, float pitch, int start, int end, double verticalScale);
    void addRoom(long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y, double z);
};

#endif
