#ifndef GAME_WORLD_GENERICOVERWORLDLEVELSOURCE_HPP
#define GAME_WORLD_GENERICOVERWORLDLEVELSOURCE_HPP

class ChunkPrimer;

class GenericOverworldLevelSource
{
public:
    virtual void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer) = 0;
    virtual bool isFlat();
};

#endif
