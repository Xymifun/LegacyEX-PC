#ifndef GAME_WORLD_CHUNK_CHUNKPRIMER_HPP
#define GAME_WORLD_CHUNK_CHUNKPRIMER_HPP

#include "game/world/biome/arrayWithLength.hpp"

class BlockState;

class ChunkPrimer
{
public:
    ChunkPrimer(bool ownsElementIds, arrayWithLength<unsigned short> elementIds);
    ~ChunkPrimer();

    const BlockState* getState(int index);
    const BlockState* getState(int x, int y, int z);
    void setState(int index, const BlockState* state);
    void setState(int x, int y, int z, const BlockState* state);
    void setBlockAndData(int index, int blockId, int blockData);
    int getBlockId(int index);
    arrayWithLength<unsigned short> getElementIds();
    unsigned short getEmissiveMaxHeight();
    int getHighestNonAirPos(int x, int z);
    int getHighestSolidPos(int x, int z);

private:
    arrayWithLength<unsigned short> mElementIds;
    const BlockState* mDefaultState;
    bool mOwnsElementIds;
    unsigned short mEmissiveMaxHeight;
};

#endif
