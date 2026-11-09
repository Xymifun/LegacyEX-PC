#ifndef GAME_WORLD_GENERATION_STRUCTUREPLACESETTINGS_HPP
#define GAME_WORLD_GENERATION_STRUCTUREPLACESETTINGS_HPP

#include <boost/unordered_map.hpp>

class Block;
class BlockPos;
class BlockState;
class BoundingBox;
class ChunkPos;
class DataInputStream;
class DataOutputStream;
class Mirror;
class Random;
class Rotation;

class StructurePlaceSettings
{
public:
    StructurePlaceSettings();
    virtual ~StructurePlaceSettings();

    void read(DataInputStream* input);
    void write(DataOutputStream* output);
    StructurePlaceSettings* copy();

    StructurePlaceSettings* setMirror(Mirror* mirror);
    StructurePlaceSettings* setRotation(Rotation* rotation);
    StructurePlaceSettings* setIgnoreEntities(bool ignoreEntities);
    StructurePlaceSettings* setIgnoreBlock(Block* block);
    StructurePlaceSettings* setBoundingBox(BoundingBox* box);
    StructurePlaceSettings* setRandom(Random* random);
    StructurePlaceSettings* setIntegrity(float integrity);

    Mirror* getMirror() const;
    Rotation* getRotation() const;
    Random* getRandom(const BlockPos* position, bool& generated);
    float getIntegrity();
    bool isIgnoreEntities();
    Block* getIgnoreBlock();

    BoundingBox* calculateBoundingBox(ChunkPos* chunkPos);
    void updateBoundingBoxFromChunkPos();
    BoundingBox* getBoundingBox();
    bool isIgnoreStructureBlocks();
    void addSwapAuxValue(int blockId, int auxValue);
    const BlockState* getSwappedBlock(const BlockState* state);
    void placeWaterBelowSeaLevel(bool placeWater);
    bool isPlacingWaterBelowSeaLevel() const;

    void setChunkPos(ChunkPos* chunkPos);
    void setSeed(long long seed);
    void setIgnoreStructureBlocks(bool ignore);
    ChunkPos* getChunkPos() const;
    long long getSeed();

private:
    Mirror* mMirror;
    Rotation* mRotation;
    bool mIgnoreEntities;
    Block* mIgnoreBlock;
    ChunkPos* mChunkPos;
    BoundingBox* mBoundingBox;
    bool mIgnoreStructureBlocks;
    float mIntegrity;
    Random* mRandom;
    long long mSeed;
    boost::unordered_map<int, int> mSwapAuxValues;
    bool mPlaceWaterBelowSeaLevel;
};

#endif
