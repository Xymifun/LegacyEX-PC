#ifndef GAME_WORLD_GENERATION_STRUCTURESTART_HPP
#define GAME_WORLD_GENERATION_STRUCTURESTART_HPP

#include <vector>

#include "game/world/generation/StructurePiece.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class StructureStart;

#include "game/world/generation/StructureFeatureIO.hpp"

class BoundingBox;
class ChunkPos;
class DataInputStream;
class DataOutputStream;
class Level;
class Random;

class StructureStart
{
public:
    StructureStart();
    StructureStart(int chunkX, int chunkZ);
    virtual ~StructureStart();
    BoundingBox* getBoundingBox();
    bool isDirty();
    unsigned int getChunkX();
    unsigned int getChunkZ();
    std::vector<StructurePiece*>* getPieces();
    void clearDirty();
    void setDirty();
    virtual void postProcess(Level* level, Random& random, BoundingBox* box);
    void calculateBoundingBox();
    virtual arrayWithLength<unsigned char> createTag(int chunkX, int chunkZ);
    virtual void addAdditionalSaveData(DataOutputStream* output);
    virtual void load(Level* level, DataInputStream* input);
    virtual void readAdditionalSaveData(DataInputStream* input);
    void moveBelowSeaLevel(Level* level, Random& random, int offset);
    void moveInsideHeights(Level* level, Random& random, int minHeight, int maxHeight);
    virtual bool isValid();
    virtual bool isValidForPostProcess(ChunkPos* chunkPos);
    virtual void notifyPostProcessAt(ChunkPos* chunkPos);
    virtual EStructureStart GetType() = 0;

public:
    std::vector<StructurePiece*> mPieces;

protected:
    BoundingBox* mBoundingBox;

private:
    int mChunkX;
    int mChunkZ;
    bool mDirty;
};

#endif
