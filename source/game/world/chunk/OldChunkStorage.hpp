#ifndef GAME_WORLD_CHUNK_OLDCHUNKSTORAGE_HPP
#define GAME_WORLD_CHUNK_OLDCHUNKSTORAGE_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/chunk/ChunkStorage.hpp"
#include "game/core/File.hpp"

class CompoundTag;
class DataFixerUpper;
class DataInputStream;
class DataOutputStream;
class Entity;
class Level;
class LevelChunk;

class OldChunkStorage : public ChunkStorage
{
public:
    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        unsigned char* blockData;
        unsigned int blockDataSize;
        unsigned char* data;
        unsigned int dataSize;
        unsigned char* skyLightData;
        unsigned int skyLightDataSize;
        unsigned char* blockLightData;
        unsigned int blockLightDataSize;
    };

    OldChunkStorage(File file, bool create);

    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();
    File getFile(int x, int z);
    static void swapBlockID(LevelChunk* chunk, int x, int y, int z, int oldId, int newId);
    static void fixBlockIDs(LevelChunk* chunk);
    static boost::shared_ptr<Entity> loadStaticEntity(CompoundTag* tag, Level* level);
    static boost::shared_ptr<Entity> loadAndAddEntity(CompoundTag* tag, Level* level, LevelChunk* chunk);
    static void loadEntities(LevelChunk* chunk, Level* level, CompoundTag* tag);
    static LevelChunk* load(Level* level, CompoundTag* tag, LevelChunk* chunk);
    virtual LevelChunk* load(Level* level, int x, int z, LevelChunk* chunk);
    virtual void save(Level* level, LevelChunk* chunk);
    static bool saveEntities(LevelChunk* chunk, Level* level, CompoundTag* tag);
    static void save(LevelChunk* chunk, Level* level, DataOutputStream* stream);
    static boost::shared_ptr<Entity> loadEntity(CompoundTag* tag, Level* level, double x, double y, double z,
                                                bool addToWorld);
    static void addEntityToWorld(boost::shared_ptr<Entity> entity, Level* level);
    static boost::shared_ptr<Entity> loadEntity(CompoundTag* tag, Level* level, bool addToWorld);
    static void fixCauldronData(LevelChunk* chunk);
    static LevelChunk* load(Level* level, DataInputStream* stream, DataFixerUpper* fixer, LevelChunk* chunk);
    virtual void tick();
    virtual void flush();
    virtual void saveEntities(Level* level, LevelChunk* chunk);

private:
    File directory;
    bool create;
};

#endif
