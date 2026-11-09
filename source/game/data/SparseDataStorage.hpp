#ifndef GAME_DATA_SPARSEDATASTORAGE_HPP
#define GAME_DATA_SPARSEDATASTORAGE_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include <stddef.h>
#include <stdint.h>

class DataInputStream;
class DataOutputStream;

template <class T>
class XLockFreeStack;

class SparseDataStorage
{
public:
    class Array256_1Bit
    {
    public:
        unsigned char indices[32];
        char palette[4];
        void init();
        unsigned int set(unsigned char index, unsigned char value);
        unsigned char get(unsigned char index);
    };

    class Array256_2Bit
    {
    public:
        unsigned char indices[64];
        char palette[4];
        void init();
        unsigned int set(unsigned char index, unsigned char value);
        unsigned char get(unsigned char index);
    };

    class PlaneOffsets
    {
    public:
        static const unsigned int PLANE_TYPE_SHIFT = 14;
        static const unsigned int PLANE_OFFSET_MASK = (1 << PLANE_TYPE_SHIFT) - 1;
        static const unsigned int PLANE_TYPE_MASK = 3;

        struct Entry
        {
            unsigned short type : 2;
            unsigned short offset : 14;
        };

        Entry offsets[128];
        unsigned char payload[0x3f80];
        void initPlane(int plane, void* data);
        unsigned int setData(int plane, int index, int value);
        unsigned char getDataSizeFromType(int type);
        unsigned char getDataSize(int plane);
        unsigned int getData(int plane, int index);
        unsigned int getValuesInPlane(int plane, char (&values)[16]);
        void setDataOffset(int plane, unsigned short offset) { offsets[plane].offset = offset; }
        unsigned short getDataOffset(int plane) { return offsets[plane].offset; }
        unsigned char* getDataPointer(int plane)
        {
            unsigned char* data = NULL;
            if (offsets[plane].type != 0)
                data = reinterpret_cast<unsigned char*>(this) + getDataOffset(plane) * 4;
            return data;
        }
        unsigned int getData(int plane, char (&values)[256]);
    };

    SparseDataStorage();
    SparseDataStorage(bool compressed);
    SparseDataStorage(SparseDataStorage* other);
    ~SparseDataStorage();
    PlaneOffsets* allocatePlaneOffsets(unsigned int size);
    static void freePlaneOffsets(PlaneOffsets* offsets, int size);
    long long createDataAndCount(PlaneOffsets* offsets, int size);
    PlaneOffsets* getPlaneOffsets();
    unsigned int getPlaneOffsetsDataSize();
    void queueForDelete(unsigned char* data, int size);
    void updateDataAndCount(PlaneOffsets* offsets, int size);
    unsigned int get(int x, int y, int z);
    unsigned int tryUpdateDataAndCount(PlaneOffsets* offsets, int size);
    static void staticCtor();
    unsigned int compress();
    static void tick();
    void write(DataOutputStream* stream);
    void read(DataInputStream* stream);
    bool isSameAs(SparseDataStorage* other);
    unsigned int getValuesInPlane(arrayWithLength<unsigned char> values, unsigned int plane, int count,
                                  char (&unique)[16]);
    void setData(arrayWithLength<unsigned char> values, unsigned int plane, bool compressed);
    void getData(arrayWithLength<unsigned char> values, unsigned int plane, int offset, int count);
    void increasePlaneBitdepth(int plane);
    void set(int x, int y, int z, int value);
    bool testSetDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth,
                           int value, bool* changed);
    void setDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth,
                       int value, void* oldData, void* newData);
    void getDataRegion(arrayWithLength<unsigned char> values, int x, int y, int z, int width, int height, int depth,
                       int value);
    bool isCompressed();

private:
    union DataAndCountValue
    {
        long long bits;
        struct
        {
            unsigned int sizeAndCount;
            PlaneOffsets* offsets;
        } value;
    };
    DataAndCountValue dataAndCount;

    static const unsigned int DATA_SIZE_SHIFT = 16;
    static const long long DATA_POINTER_MASK = 0x0000ffffffffffffLL;

    static int sPendingDeleteList;
    static XLockFreeStack<unsigned char> sPendingDeleteLists[3];
    static int sAllocatedBytes;
};

#endif
