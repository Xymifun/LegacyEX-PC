#ifndef GAME_WORLD_CHUNK_CHUNKSTORAGE_HPP
#define GAME_WORLD_CHUNK_CHUNKSTORAGE_HPP

class Level;
class LevelChunk;

class ChunkStorage
{
public:
    virtual ~ChunkStorage() {}

    virtual LevelChunk* load(Level* level, int x, int z, LevelChunk* chunk) = 0;
    virtual void save(Level* level, LevelChunk* chunk) = 0;
    virtual void saveEntities(Level* level, LevelChunk* chunk) = 0;
    virtual void tick() = 0;
    virtual void flush() = 0;
    virtual void WaitForAll();
    virtual void WaitIfTooManyQueuedChunks();
};

#endif
