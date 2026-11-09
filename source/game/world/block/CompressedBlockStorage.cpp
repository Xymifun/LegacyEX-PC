#include "game/world/block/CompressedBlockStorage.hpp"

#include <string.h>

#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/System.hpp"
#include "game/core/XLockFreeStack.hpp"
#include "game/core/XMem.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

void __debugbreak();

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeletes[3];
static OSFastMutex sCompressLock;
static int sAllocatedBytes;

CompressedBlockStorage::CompressedBlockStorage()
{
    mData = NULL;
    mSize = 0;
}

CompressedBlockStorage::~CompressedBlockStorage()
{
    if (mData != NULL)
    {
        int size = mSize;

        XPhysicalFree(mData);

        sAllocatedBytes -= size;
    }
}

void CompressedBlockStorage::staticCtor()
{
    InitializeCriticalSectionAndSpinCount(&sCompressLock, 5120);
}

void CompressedBlockStorage::tick()
{
    XLockFreeStack<unsigned char>* queue = &sPendingDeletes[(sPendingDeleteList + 1) % 3];

    unsigned char* toFree = NULL;

    do
    {
        toFree = queue->Pop();

        if (toFree != NULL)
        {
            XPhysicalFree(toFree);
        }
    } while (toFree != NULL);

    sPendingDeleteList = (sPendingDeleteList + 1) % 3;
}

void CompressedBlockStorage::queueForDelete(unsigned char* data, int size)
{
    if (data != NULL)
    {
        sPendingDeletes[sPendingDeleteList].Push(data);

        sAllocatedBytes -= size;
    }
}

void CompressedBlockStorage::reverseIndices(unsigned char* data)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    unsigned short* headers = (unsigned short*)data;

    for (int i = 0; i < 512; i++)
    {
        System::ReverseUSHORT(&headers[i]);
    }
#else
    (void)data;
#endif
}

void CompressedBlockStorage::getDataSection(unsigned char* blocks, int x, int y, int z)
{
    unsigned short* headers = (unsigned short*)mData;
    unsigned char* data = mData + 1024;

    int index = (x << 7) | (z << 5) | y;
    int format = headers[index] & 3;

    if (format == 3)
    {
        if ((headers[index] & 4) != 0)
        {
            for (int i = 0; i < 64; i++)
            {
                blocks[i] = (headers[index] >> 8) & 0xFF;
            }
        }
        else
        {
            unsigned char* section = data + ((headers[index] >> 1) & 0x7FFE);

            for (int i = 0; i < 64; i++)
            {
                blocks[i] = section[i];
            }
        }
    }
    else
    {
        int bitsPerBlock = 1 << format;
        int shift = 3 - format;
        int paletteSize = 1 << bitsPerBlock;
        int valueMask = paletteSize - 1;
        int slotMask = 7 >> format;
        int indexMask = 0x3E >> shift;
        unsigned char* palette = data + ((headers[index] >> 1) & 0x7FFE);
        unsigned char* indices = palette + paletteSize;

        for (int i = 0; i < 64; i++)
        {
            int indexByte = (i >> shift) & indexMask;
            int bit = (i & slotMask) * bitsPerBlock;

            blocks[i] = palette[(indices[indexByte] >> bit) & valueMask];
        }
    }
}

void CompressedBlockStorage::compress(int upgradeBlock)
{
    if (mData == NULL)
    {
        return;
    }

    if (mSize == 1024 && upgradeBlock == -1)
    {
        return;
    }

    unsigned char tempData[64];
    unsigned short newIndices[512];

    bool needsCompressing = upgradeBlock > -1;

    EnterCriticalSection(&sCompressLock);

    unsigned short* blockIndices = (unsigned short*)mData;
    unsigned char* data = mData + 1024;

    int memToAlloc = 0;

    for (int i = 0; i < 512; i++)
    {
        unsigned short indexType = blockIndices[i] & 3;

        unsigned char* unpacked = NULL;

        if (upgradeBlock == -1)
        {
            if (indexType == 3)
            {
                if ((blockIndices[i] & 4) == 0)
                {
                    unpacked = data + ((blockIndices[i] >> 1) & 0x7FFE);
                }
            }
            else
            {
                int bitsPerBlock = 1 << indexType;
                int shift = 3 - indexType;
                int paletteSize = 1 << bitsPerBlock;
                int valueMask = paletteSize - 1;
                int slotMask = 7 >> indexType;
                int indexMask = 0x3E >> shift;
                unsigned char* palette = data + ((blockIndices[i] >> 1) & 0x7FFE);
                unsigned char* indices = palette + paletteSize;

                for (int j = 0; j < 64; j++)
                {
                    int indexByte = (j >> shift) & indexMask;
                    int bit = (j & slotMask) * bitsPerBlock;

                    tempData[j] = palette[(indices[indexByte] >> bit) & valueMask];
                }

                unpacked = tempData;
            }

            if (unpacked != NULL)
            {
                unsigned long long usedTypes[4] = {0, 0, 0, 0};

                for (int j = 0; j < 64; j++)
                {
                    int blockId = unpacked[j];

                    usedTypes[blockId & 3] |= 1ULL << (blockId >> 2);
                }

                int typeCount = 0;

                for (int j = 0; j < 256; j++)
                {
                    if ((usedTypes[j & 3] & (1ULL << (j >> 2))) != 0)
                    {
                        typeCount++;
                    }
                }

                if (typeCount == 1)
                {
                    newIndices[i] = 7;
                    needsCompressing = true;
                }
                else if (typeCount == 2)
                {
                    newIndices[i] = 0;

                    if (indexType != 0)
                    {
                        needsCompressing = true;
                    }

                    memToAlloc += 10;
                }
                else if (typeCount <= 4)
                {
                    newIndices[i] = 1;

                    if (indexType != 1)
                    {
                        needsCompressing = true;
                    }

                    memToAlloc += 20;
                }
                else if (typeCount <= 16)
                {
                    newIndices[i] = 2;

                    if (indexType != 2)
                    {
                        needsCompressing = true;
                    }

                    memToAlloc += 48;
                }
                else
                {
                    newIndices[i] = 3;
                    memToAlloc = (memToAlloc + 3) & 0xFFFC;
                    memToAlloc += 64;
                }
            }
            else
            {
                newIndices[i] = blockIndices[i];
            }
        }
        else
        {
            if (i == upgradeBlock)
            {
                indexType = (indexType + 1) & 3;
                newIndices[i] = indexType;
            }
            else
            {
                newIndices[i] = indexType;

                if (indexType == 3)
                {
                    indexType = (blockIndices[i] & 4) | indexType;
                    newIndices[i] = indexType;
                }
            }

            switch (indexType)
            {
            case 0:
                memToAlloc += 10;
                break;
            case 1:
                memToAlloc += 20;
                break;
            case 2:
                memToAlloc += 48;
                break;
            case 3:
                memToAlloc = (memToAlloc + 3) & 0xFFFC;
                memToAlloc += 64;
                break;
            }
        }
    }

    if (needsCompressing)
    {
        memToAlloc += 1024;

        unsigned char* newIndicesAndData = (unsigned char*)XPhysicalAlloc(memToAlloc, 0xFFFFFFFF, 0x1000, 4);

        sAllocatedBytes += memToAlloc;

        if (newIndicesAndData == NULL)
        {
            _MEMORYSTATUS memoryStatus;

            GlobalMemoryStatus(&memoryStatus);
            __debugbreak();
        }

        unsigned char* newData = newIndicesAndData + 1024;
        unsigned short dataOffset = 0;
        unsigned short* newBlockIndices = (unsigned short*)newIndicesAndData;

        for (int i = 0; i < 512; i++)
        {
            unsigned short newIndexType = newIndices[i] & 3;
            unsigned short oldIndexType = blockIndices[i] & 3;

            newBlockIndices[i] = newIndexType;

            bool done = false;

            if (oldIndexType == newIndexType)
            {
                unsigned char* packed;

                if (oldIndexType == 3)
                {
                    if ((blockIndices[i] & 4) == (newIndices[i] & 4))
                    {
                        if ((blockIndices[i] & 4) != 0)
                        {
                            newBlockIndices[i] = blockIndices[i];
                        }
                        else
                        {
                            packed = data + ((blockIndices[i] >> 1) & 0x7FFE);
                            dataOffset = (dataOffset + 3) & 0xFFFC;

                            XMemCpy(newData + dataOffset, packed, 64);

                            newBlockIndices[i] |= (dataOffset & 0x7FFE) << 1;
                            dataOffset += 64;
                        }

                        done = true;
                    }
                }
                else
                {
                    packed = data + ((blockIndices[i] >> 1) & 0x7FFE);

                    int dataSize = 8 << oldIndexType;
                    dataSize += 1 << (1 << oldIndexType);

                    newBlockIndices[i] |= (dataOffset & 0x7FFE) << 1;

                    XMemCpy(newData + dataOffset, packed, dataSize);

                    dataOffset += dataSize;
                    done = true;
                }
            }

            if (!done)
            {
                unsigned char* unpacked = NULL;
                unsigned char* tileTypes = NULL;
                unsigned char* packed = NULL;

                if (oldIndexType == 3)
                {
                    if ((blockIndices[i] & 4) != 0)
                    {
                        unpacked = tempData;

                        int value = (blockIndices[i] >> 8) & 0xFF;

                        XMemSet(tempData, value, 64);
                    }
                    else
                    {
                        unpacked = data + ((blockIndices[i] >> 1) & 0x7FFE);
                    }
                }
                else
                {
                    int bitsPerBlock = 1 << oldIndexType;
                    int shift = 3 - oldIndexType;
                    int paletteSize = 1 << bitsPerBlock;
                    int valueMask = paletteSize - 1;
                    int slotMask = 7 >> oldIndexType;
                    int indexMask = 0x3E >> shift;
                    unsigned char* palette = data + ((blockIndices[i] >> 1) & 0x7FFE);
                    unsigned char* indices = palette + paletteSize;

                    for (int j = 0; j < 64; j++)
                    {
                        int indexByte = (j >> shift) & indexMask;
                        int bit = (j & slotMask) * bitsPerBlock;

                        tempData[j] = palette[(indices[indexByte] >> bit) & valueMask];
                    }

                    unpacked = tempData;
                }

                unsigned int typeMapWords[64];
                unsigned char* typeMap = (unsigned char*)typeMapWords;

                for (int j = 0; j < 64; j++)
                {
                    typeMapWords[j] = 0xFFFFFFFF;
                }

                if (newIndexType == 3)
                {
                    if ((newIndices[i] & 4) != 0)
                    {
                        newBlockIndices[i] = 3 | 4 | (((unsigned short)unpacked[0]) << 8);
                    }
                    else
                    {
                        dataOffset = (dataOffset + 3) & 0xFFFC;

                        XMemCpy(newData + dataOffset, unpacked, 64);

                        newBlockIndices[i] |= (dataOffset & 0x7FFE) << 1;
                        dataOffset += 64;
                    }
                }
                else
                {
                    int bitsPerBlock = 1 << newIndexType;
                    int paletteSize = 1 << bitsPerBlock;
                    int shift = 3 - newIndexType;
                    int slotMask = 7 >> newIndexType;
                    int indexMask = 0x3E >> shift;

                    tileTypes = newData + dataOffset;
                    packed = tileTypes + paletteSize;

                    int packedSize = 8 << newIndexType;

                    XMemSet(tileTypes, 255, paletteSize);
                    XMemSet(packed, 0, packedSize);

                    newBlockIndices[i] |= (dataOffset & 0x7FFE) << 1;
                    dataOffset += paletteSize + packedSize;

                    int typeCount = 0;

                    for (int j = 0; j < 64; j++)
                    {
                        int blockId = unpacked[j];

                        if (typeMap[blockId] == 255)
                        {
                            typeMap[blockId] = typeCount;
                            tileTypes[typeCount++] = blockId;
                        }

                        int indexByte = (j >> shift) & indexMask;
                        int bit = (j & slotMask) * bitsPerBlock;

                        packed[indexByte] |= typeMap[blockId] << bit;
                    }
                }
            }
        }

        queueForDelete(mData, mSize);

        mData = newIndicesAndData;
        mSize = memToAlloc;
    }

    LeaveCriticalSection(&sCompressLock);
}

void CompressedBlockStorage::write(DataOutputStream* out)
{
    out->writeInt(mSize);

    if (mData != NULL)
    {
        arrayWithLength<unsigned char> indices(1024, true);

        memcpy(indices.data, mData, 1024);

        reverseIndices(indices.data);

        out->write(indices);

        delete[] indices.data;

        if (mSize > 1024)
        {
            out->write(arrayWithLength<unsigned char>(mData + 1024, mSize - 1024));
        }
    }
}

void CompressedBlockStorage::read(DataInputStream* in)
{
    int size = in->readInt();

    if (size > 0)
    {
        if (mData != NULL)
        {
            int oldSize = mSize;

            XPhysicalFree(mData);

            sAllocatedBytes -= oldSize;
        }

        mData = (unsigned char*)XPhysicalAlloc(size, 0xFFFFFFFF, 0x1000, 4);
        mSize = size;
        sAllocatedBytes += size;
        arrayWithLength<unsigned char> wrapper(mData, size);

        in->readFully(wrapper);

        reverseIndices(mData);

        compress(-1);
    }
}

CompressedBlockStorage::CompressedBlockStorage(CompressedBlockStorage* other)
{
    mData = NULL;
    mSize = 0;
    if (other != NULL && other->mData != NULL)
    {
        setData(arrayWithLength<unsigned char>(), 0);
        mSize = other->mSize;
        mData = (unsigned char*)XPhysicalAlloc(mSize, 0xFFFFFFFF, 0x1000, 4);
        memcpy(mData, other->mData, mSize);
        sAllocatedBytes += mSize;
    }
}

CompressedBlockStorage::CompressedBlockStorage(arrayWithLength<unsigned char> blocks, unsigned int offset)
{
    mData = NULL;
    mSize = 0;
    setData(blocks, offset);
}

CompressedBlockStorage::CompressedBlockStorage(bool empty)
{
    mData = NULL;
    mSize = 0;
    if (!empty)
    {
        mSize = 1024;
        mData = (unsigned char*)XPhysicalAlloc(mSize, 0xFFFFFFFF, 0x1000, 4);
        memset(mData, 0, mSize);
        sAllocatedBytes += mSize;
    }
}

bool CompressedBlockStorage::isSameAs(CompressedBlockStorage* other)
{
    if (other == NULL || mSize != other->mSize)
        return false;
    if (mSize == 0)
        return true;
    return memcmp(mData, other->mData, mSize) == 0;
}

bool CompressedBlockStorage::isCompressed()
{
    return mData != NULL && mSize != 1024;
}

unsigned char CompressedBlockStorage::get(int x, int y, int z)
{
    unsigned char section[64];
    getDataSection(section, x >> 2, y >> 2, z >> 2);
    return section[((x & 3) << 4) | ((z & 3) << 2) | (y & 3)];
}

void CompressedBlockStorage::set(int x, int y, int z, int blockId)
{
    if (mData == NULL)
    {
        mSize = 1024;
        mData = (unsigned char*)XPhysicalAlloc(mSize, 0xFFFFFFFF, 0x1000, 4);
        memset(mData, 0, mSize);
        sAllocatedBytes += mSize;
    }
    unsigned short* headers = (unsigned short*)mData;
    int sectionIndex = ((x >> 2) << 7) | ((z >> 2) << 5) | (y >> 2);
    unsigned char section[64];
    getDataSection(section, x >> 2, y >> 2, z >> 2);
    section[((x & 3) << 4) | ((z & 3) << 2) | (y & 3)] = (unsigned char)blockId;
    unsigned char* dst = mData + 1024 + ((headers[sectionIndex] >> 1) & 0x7FFE);
    unsigned int format = headers[sectionIndex] & 3;
    if (format == 3 && (headers[sectionIndex] & 4) == 0)
        memcpy(dst, section, 64);
    else if (format == 3)
        headers[sectionIndex] = (unsigned short)(section[0] << 8 | 7);
    else
        compress(sectionIndex);
}

void CompressedBlockStorage::setData(arrayWithLength<unsigned char> blocks, unsigned int offset)
{
    if (mData != NULL)
    {
        XPhysicalFree(mData);
        sAllocatedBytes -= mSize;
    }
    mData = NULL;
    mSize = 0;
    if (blocks.data != NULL && offset < blocks.length)
    {
        mSize = blocks.length - offset;
        mData = (unsigned char*)XPhysicalAlloc(mSize, 0xFFFFFFFF, 0x1000, 4);
        memcpy(mData, blocks.data + offset, mSize);
        sAllocatedBytes += mSize;
        reverseIndices(mData);
        compress(-1);
    }
}

void CompressedBlockStorage::getData(arrayWithLength<unsigned char> blocks, unsigned int offset, int minY, int maxY)
{
    if (mData == NULL || blocks.data == NULL)
        return;
    unsigned int count = blocks.length - offset;
    if (count > (unsigned int)mSize)
        count = mSize;
    memcpy(blocks.data + offset, mData, count);
    reverseIndices(blocks.data + offset);
}

bool CompressedBlockStorage::isRenderChunkEmpty(int renderChunkIndex)
{
    if (mData == NULL)
        return true;
    unsigned short* headers = (unsigned short*)mData;
    int first = renderChunkIndex * 32;
    for (int i = 0; i < 32; i++)
        if ((headers[first + i] & 3) != 3 || (headers[first + i] & 4) == 0)
            return false;
    return true;
}

int CompressedBlockStorage::getHighestNonEmptyY()
{
    if (mData == NULL)
        return -1;
    unsigned short* headers = (unsigned short*)mData;
    for (int y = 7; y >= 0; y--)
        for (int x = 0; x < 8; x++)
            for (int z = 0; z < 8; z++)
                if ((headers[(x << 7) | (z << 5) | y] & 3) != 3 || (headers[(x << 7) | (z << 5) | y] & 4) == 0)
                    return y;
    return -1;
}

void CompressedBlockStorage::getAllocatedSize(int* totalSize, int* uniformCount, int* oneBitCount, int* twoBitCount,
                                              int* fourBitCount)
{
    int u = 0, a = 0, b = 0, c = 0;
    if (mData != NULL)
        for (int i = 0; i < 512; i++)
        {
            int f = ((unsigned short*)mData)[i] & 3;
            if (f == 3)
                u++;
            else if (f == 0)
                a++;
            else if (f == 1)
                b++;
            else
                c++;
        }
    if (totalSize)
        *totalSize = mSize;
    if (uniformCount)
        *uniformCount = u;
    if (oneBitCount)
        *oneBitCount = a;
    if (twoBitCount)
        *twoBitCount = b;
    if (fourBitCount)
        *fourBitCount = c;
}

void CompressedBlockStorage::setDataRegion(arrayWithLength<unsigned char> blocks, int offset, int x, int y, int z,
                                           int sizeX, int sizeY, int sizeZ,
                                           void (*changedCallback)(int, int, int, void*, int), void* callbackData,
                                           int oldBlockId)
{
    for (int dz = 0; dz < sizeZ; dz++)
        for (int dy = 0; dy < sizeY; dy++)
            for (int dx = 0; dx < sizeX; dx++)
            {
                int p = offset + (dz * sizeY + dy) * sizeX + dx;
                if (blocks.data && p < (int)blocks.length)
                {
                    set(x + dx, y + dy, z + dz, blocks.data[p]);
                    if (changedCallback)
                        changedCallback(x + dx, y + dy, z + dz, callbackData, oldBlockId);
                }
            }
}

bool CompressedBlockStorage::testSetDataRegion(arrayWithLength<unsigned char> blocks, int offset, int x, int y, int z,
                                               int sizeX, int sizeY, int sizeZ, bool* changed)
{
    bool result = false;
    for (int dz = 0; dz < sizeZ; dz++)
        for (int dy = 0; dy < sizeY; dy++)
            for (int dx = 0; dx < sizeX; dx++)
            {
                int p = offset + (dz * sizeY + dy) * sizeX + dx;
                if (blocks.data && p < (int)blocks.length && get(x + dx, y + dy, z + dz) != blocks.data[p])
                {
                    set(x + dx, y + dy, z + dz, blocks.data[p]);
                    result = true;
                }
            }
    if (changed)
        *changed = result;
    return result;
}

void CompressedBlockStorage::getDataRegion(arrayWithLength<unsigned char> blocks, int offset, int x, int y, int z,
                                           int sizeX, int sizeY, int sizeZ)
{
    for (int dz = 0; dz < sizeZ; dz++)
        for (int dy = 0; dy < sizeY; dy++)
            for (int dx = 0; dx < sizeX; dx++)
            {
                int p = offset + (dz * sizeY + dy) * sizeX + dx;
                if (blocks.data && p < (int)blocks.length)
                    blocks.data[p] = get(x + dx, y + dy, z + dz);
            }
}
