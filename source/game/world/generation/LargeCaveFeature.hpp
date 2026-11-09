#ifndef GAME_WORLD_GENERATION_LARGECAVEFEATURE_HPP
#define GAME_WORLD_GENERATION_LARGECAVEFEATURE_HPP

#include "game/world/generation/LargeFeature.hpp"

class ChunkPrimer;
class Level;

class LargeCaveFeature : public LargeFeature
{
public:
    static unsigned int canReplaceBlock(int blockId, int data);
    void setCaveBlocks(Level* level, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, int chunkX, int chunkZ,
                       double centerX, double centerY, double centerZ, bool hasWater, double radiusX, double radiusZ,
                       ChunkPrimer* primer);
    void addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                   double z, float width, float yaw, float pitch, int start, int length, double scale);
    void addRoom(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                 double z);
    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);
};

#endif
