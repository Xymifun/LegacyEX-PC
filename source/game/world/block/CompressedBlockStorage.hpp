#ifndef GAME_WORLD_BLOCK_COMPRESSEDBLOCKSTORAGE_HPP
#define GAME_WORLD_BLOCK_COMPRESSEDBLOCKSTORAGE_HPP

#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;

class CompressedBlockStorage
{
public:
    CompressedBlockStorage();
    CompressedBlockStorage(CompressedBlockStorage* other);
    CompressedBlockStorage(arrayWithLength<unsigned char> blocks, unsigned int offset);
    CompressedBlockStorage(bool empty);
    ~CompressedBlockStorage();

    static void staticCtor();
    static void tick();

    bool isSameAs(CompressedBlockStorage* other);
    bool isCompressed();
    bool isRenderChunkEmpty(int renderChunkIndex);

    int getHighestNonEmptyY();
    void getAllocatedSize(int* totalSize, int* uniformCount, int* oneBitCount, int* twoBitCount, int* fourBitCount);

    unsigned char get(int x, int y, int z);
    void set(int x, int y, int z, int blockId);

    void setData(arrayWithLength<unsigned char> blocks, unsigned int offset);
    void getData(arrayWithLength<unsigned char> blocks, unsigned int offset, int minY, int maxY);

    void setDataRegion(arrayWithLength<unsigned char> blocks, int offset, int x, int y, int z, int sizeX, int sizeY,
                       int sizeZ, void (*changedCallback)(int, int, int, void*, int), void* callbackData,
                       int oldBlockId);
    bool testSetDataRegion(arrayWithLength<unsigned char> blocks, int offset, int x, int y, int z, int sizeX, int sizeY,
                           int sizeZ, bool* changed);
    void getDataRegion(arrayWithLength<unsigned char> blocks, int offset, int x, int y, int z, int sizeX, int sizeY,
                       int sizeZ);

    void getDataSection(unsigned char* blocks, int x, int y, int z);

    void compress(int sectionIndex);

    void write(DataOutputStream* out);
    void read(DataInputStream* in);

private:
    void queueForDelete(unsigned char* data, int size);
    void reverseIndices(unsigned char* data);

    unsigned char* mData;
    int mSize;
};

#endif
