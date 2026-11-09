#ifndef GAME_WORLD_GENERATION_UNDERWATERLARGECAVEFEATURE_HPP
#define GAME_WORLD_GENERATION_UNDERWATERLARGECAVEFEATURE_HPP

#include "game/world/generation/LargeFeature.hpp"

class ChunkPrimer;
class Level;

class UnderwaterLargeCaveFeature : public LargeFeature
{
public:
    static bool isDiggable(int blockId);
    virtual void apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer);
    void addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                   double z, float width, float yaw, float pitch, int originChunkX, int originChunkZ, double scale);
    void addRoom(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                 double z);
    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);
};

#endif
