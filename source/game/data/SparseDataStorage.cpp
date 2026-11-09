#include "game/data/SparseDataStorage.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"
#include "game/core/XMem.hpp"
#include <cstdlib>
#include <string.h>

long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand);

int SparseDataStorage::sPendingDeleteList;
XLockFreeStack<unsigned char> SparseDataStorage::sPendingDeleteLists[3];
static OSFastMutex sCompressLock;
int SparseDataStorage::sAllocatedBytes;

void SparseDataStorage::Array256_1Bit::init()
{
    for (int i = 0; i < 2; ++i)
        palette[i] = -1;
    for (int i = 2; i < 4; ++i)
        palette[i] = 0;
}

unsigned int SparseDataStorage::Array256_1Bit::set(unsigned char index, unsigned char value)
{
    unsigned int result;
    if (value == palette[0])
    {
        result = 1;
        indices[index >> 3] &= ~(1 << (index & 7));
    }
    else if (palette[0] == -1)
    {
        palette[0] = value;
        result = 1;
        indices[index >> 3] &= ~(1 << (index & 7));
    }
    else if (value == palette[1])
    {
        result = 1;
        indices[index >> 3] &= ~(1 << (index & 7));
        indices[index >> 3] |= 1 << (index & 7);
    }
    else if (palette[1] == -1)
    {
        palette[1] = value;
        result = 1;
        indices[index >> 3] &= ~(1 << (index & 7));
        indices[index >> 3] |= 1 << (index & 7);
    }
    else
    {
        result = 0;
    }
    return result;
}

unsigned char SparseDataStorage::Array256_1Bit::get(unsigned char index)
{
    return palette[(indices[index >> 3] >> (index & 7)) & 1];
}

void SparseDataStorage::Array256_2Bit::init()
{
    palette[0] = -1;
    palette[1] = -1;
    palette[2] = -1;
    palette[3] = -1;
}

unsigned int SparseDataStorage::Array256_2Bit::set(unsigned char index, unsigned char value)
{
    unsigned int result;
    if (value == palette[0])
    {
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
    }
    else if (palette[0] == -1)
    {
        palette[0] = value;
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
    }
    else if (value == palette[1])
    {
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
        indices[index >> 2] |= 1 << ((index & 3) << 1);
    }
    else if (palette[1] == -1)
    {
        palette[1] = value;
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
        indices[index >> 2] |= 1 << ((index & 3) << 1);
    }
    else if (value == palette[2])
    {
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
        indices[index >> 2] |= 2 << ((index & 3) << 1);
    }
    else if (palette[2] == -1)
    {
        palette[2] = value;
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
        indices[index >> 2] |= 2 << ((index & 3) << 1);
    }
    else if (value == palette[3])
    {
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
        indices[index >> 2] |= 3 << ((index & 3) << 1);
    }
    else if (palette[3] == -1)
    {
        palette[3] = value;
        result = 1;
        indices[index >> 2] &= ~(3 << ((index & 3) << 1));
        indices[index >> 2] |= 3 << ((index & 3) << 1);
    }
    else
    {
        result = 0;
    }
    return result;
}

unsigned char SparseDataStorage::Array256_2Bit::get(unsigned char index)
{
    return palette[(indices[index >> 2] >> ((index & 3) << 1)) & 3];
}

void SparseDataStorage::PlaneOffsets::initPlane(int plane, void* data)
{
    if (offsets[plane].type != 0)
        setDataOffset(
            plane,
            static_cast<unsigned int>(static_cast<unsigned char*>(data) - reinterpret_cast<unsigned char*>(this)) >> 2);
    switch (offsets[plane].type)
    {
    case 1:
        reinterpret_cast<Array256_1Bit*>(getDataPointer(plane))->init();
        break;
    case 2:
        reinterpret_cast<Array256_2Bit*>(getDataPointer(plane))->init();
        break;
    }
}

unsigned int SparseDataStorage::PlaneOffsets::setData(int plane, int index, int value)
{
    unsigned int result;
    const unsigned int type = offsets[plane].type;
    switch (type)
    {
    case 0:
        if (value == getDataOffset(plane))
            result = 1;
        else
            result = 0;
        break;
    case 1:
        result = reinterpret_cast<Array256_1Bit*>(getDataPointer(plane))
                     ->set(static_cast<unsigned char>(index), static_cast<unsigned char>(value));
        break;
    case 2:
        result = reinterpret_cast<Array256_2Bit*>(getDataPointer(plane))
                     ->set(static_cast<unsigned char>(index), static_cast<unsigned char>(value));
        break;
    case 3:
    {
        unsigned char* nibbles = getDataPointer(plane);
        const unsigned char position = static_cast<unsigned char>(index);
        nibbles[position >> 1] = static_cast<unsigned char>((nibbles[position >> 1] & ~(15 << ((index & 1) << 2)))
                                                            | (value << ((index & 1) << 2)));
        result = 1;
        break;
    }
    default:
        result = 0;
        break;
    }
    return result;
}

unsigned char SparseDataStorage::PlaneOffsets::getDataSizeFromType(int type)
{
    switch (type)
    {
    case 1:
        return sizeof(Array256_1Bit);
    case 2:
        return sizeof(Array256_2Bit);
    case 3:
        return 0x80;
    }
    return 0;
}
unsigned char SparseDataStorage::PlaneOffsets::getDataSize(int plane)
{
    return getDataSizeFromType(offsets[plane].type);
}
unsigned int SparseDataStorage::PlaneOffsets::getData(int plane, int index)
{
    const unsigned int type = offsets[plane].type;
    unsigned int result = 0;
    switch (type)
    {
    case 0:
        result = getDataOffset(plane);
        break;
    case 1:
        result = reinterpret_cast<Array256_1Bit*>(getDataPointer(plane))->get(static_cast<unsigned char>(index));
        break;
    case 2:
        result = reinterpret_cast<Array256_2Bit*>(getDataPointer(plane))->get(static_cast<unsigned char>(index));
        break;
    case 3:
    {
        unsigned char* nibbles = getDataPointer(plane);
        result = (nibbles[static_cast<unsigned char>(index) >> 1] >> ((index & 1) << 2)) & 15;
        break;
    }
    }
    return result;
}

unsigned int SparseDataStorage::PlaneOffsets::getValuesInPlane(int plane, char (&values)[16])
{
    int count = 0;
    for (int i = 0; i < 256; ++i)
    {
        char value = getData(plane, i);
        bool found = false;
        if (i != 0)
        {
            for (int j = 0; j < count; ++j)
            {
                if (value == values[j])
                {
                    found = true;
                    break;
                }
            }
        }
        if (!found)
            values[count++] = value;
    }
    return count;
}
SparseDataStorage::PlaneOffsets* SparseDataStorage::allocatePlaneOffsets(unsigned int size)
{
    sAllocatedBytes += size;
    return static_cast<PlaneOffsets*>(malloc(size));
}
void SparseDataStorage::freePlaneOffsets(PlaneOffsets* offsets, int size)
{
    sAllocatedBytes -= size;
    free(offsets);
}
long long SparseDataStorage::createDataAndCount(PlaneOffsets* offsets, int size)
{
    return (static_cast<long long>(size) << 48) | reinterpret_cast<uintptr_t>(offsets);
}
SparseDataStorage::SparseDataStorage()
{
    PlaneOffsets* offsets = allocatePlaneOffsets(0x4080);
    unsigned char* payload = offsets->payload;
    unsigned char* cursor = payload;
    offsets->offsets[0].type = 0;
    offsets->offsets[0].offset = 0;
    for (int plane = 1; plane < 128; ++plane)
    {
        offsets->offsets[plane].type = 3;
        offsets->initPlane(plane, cursor);
        cursor += 0x80;
    }
    XMemSet(payload, 0, 0x3f80);
    dataAndCount.bits = createDataAndCount(offsets, 0x4080);
}
SparseDataStorage::~SparseDataStorage()
{
    PlaneOffsets* offsets = getPlaneOffsets();
    freePlaneOffsets(offsets, getPlaneOffsetsDataSize());
}
SparseDataStorage::PlaneOffsets* SparseDataStorage::getPlaneOffsets()
{
    return reinterpret_cast<PlaneOffsets*>(dataAndCount.bits & DATA_POINTER_MASK);
}
unsigned int SparseDataStorage::getPlaneOffsetsDataSize()
{
    return dataAndCount.value.sizeAndCount >> DATA_SIZE_SHIFT;
}
void SparseDataStorage::queueForDelete(unsigned char* data, int)
{
    sPendingDeleteLists[sPendingDeleteList].Push(data);
}
void SparseDataStorage::updateDataAndCount(PlaneOffsets* offsets, int size)
{
    long long replacement = createDataAndCount(offsets, size);
    int oldSize = getPlaneOffsetsDataSize();
    long long previous;
    long long current;
    unsigned char* oldData;
    do
    {
        previous = dataAndCount.bits;
        oldData = reinterpret_cast<unsigned char*>(previous & DATA_POINTER_MASK);
        current = InterlockedCompareExchangeRelease64(&dataAndCount.bits, replacement, previous);
    } while (current != previous);
    queueForDelete(oldData, oldSize);
}
unsigned int SparseDataStorage::get(int x, int y, int z)
{
    PlaneOffsets* offsets = getPlaneOffsets();
    int index = x * 16 + z;
    return offsets->getData(y, index);
}
unsigned int SparseDataStorage::tryUpdateDataAndCount(PlaneOffsets* offsets, int size)
{
    unsigned char* oldData = reinterpret_cast<unsigned char*>(getPlaneOffsets());
    int oldSize = getPlaneOffsetsDataSize();
    long long previous = dataAndCount.bits;
    long long replacement = createDataAndCount(offsets, size);
    long long current = InterlockedCompareExchangeRelease64(&dataAndCount.bits, replacement, previous);
    if (current == previous)
    {
        queueForDelete(oldData, oldSize);
        return 1;
    }
    return 0;
}
void SparseDataStorage::staticCtor()
{
    InitializeCriticalSection(&sCompressLock);
}

unsigned int SparseDataStorage::compress()
{
    EnterCriticalSection(&sCompressLock);

    PlaneOffsets* sourceOffsets = getPlaneOffsets();
    unsigned int currentSize = getPlaneOffsetsDataSize();
    if (currentSize < 0x3f80)
    {
        LeaveCriticalSection(&sCompressLock);
        return currentSize;
    }

    PlaneOffsets::Entry localOffsets[128];
    unsigned int payloadSize = 0;
    for (int plane = 0; plane < 128; ++plane)
    {
        char uniqueValues[16];
        int valueCount = sourceOffsets->getValuesInPlane(plane, uniqueValues);
        if (valueCount <= 4)
        {
            if (valueCount > 2)
            {
                localOffsets[plane].type = 2;
                payloadSize += sizeof(Array256_2Bit);
            }
            else if (valueCount < 2)
            {
                localOffsets[plane].type = 0;
                localOffsets[plane].offset = uniqueValues[0];
            }
            else
            {
                localOffsets[plane].type = 1;
                payloadSize += sizeof(Array256_1Bit);
            }
        }
        else
        {
            localOffsets[plane].type = 3;
            payloadSize += 0x80;
        }
    }

    const unsigned int newSize = payloadSize + 0x100;
    currentSize = getPlaneOffsetsDataSize();
    if (newSize != currentSize)
    {
        PlaneOffsets* destination = allocatePlaneOffsets(newSize);
        unsigned char* dataCursor = destination->payload;
        XMemCpy(destination, localOffsets, 0x100);
        for (int plane = 0; plane < 128; ++plane)
        {
            destination->initPlane(plane, dataCursor);
            for (int index = 0; index < 256; ++index)
            {
                destination->setData(plane, index, static_cast<char>(sourceOffsets->getData(plane, index)));
            }
            dataCursor += destination->getDataSize(plane);
        }

        if (tryUpdateDataAndCount(destination, static_cast<int>(newSize)) == 0)
            queueForDelete(reinterpret_cast<unsigned char*>(destination), static_cast<int>(newSize));
        LeaveCriticalSection(&sCompressLock);
        return newSize;
    }

    LeaveCriticalSection(&sCompressLock);
    return currentSize;
}
void SparseDataStorage::tick()
{
    int index = (sPendingDeleteList + 1) % 3;
    unsigned char* data;
    do
    {
        data = sPendingDeleteLists[index].Pop();
        if (data != NULL)
            freePlaneOffsets(static_cast<PlaneOffsets*>(static_cast<void*>(data)), 0);
    } while (data != NULL);
    sPendingDeleteList = (sPendingDeleteList + 1) % 3;
}
void SparseDataStorage::write(DataOutputStream* stream)
{
    PlaneOffsets* planeOffsets = getPlaneOffsets();
    unsigned int nonEmpty = 0;
    for (int plane = 0; plane < 128; ++plane)
        if (planeOffsets->offsets[plane].type != 0 || planeOffsets->offsets[plane].offset != 0)
            ++nonEmpty;

    const unsigned int blockSize = 0x80;
    const unsigned int totalSize = blockSize * (nonEmpty + 1);
    unsigned char* encoded = new unsigned char[totalSize];
    int block = 0;
    unsigned char* destination = encoded + blockSize;
    for (int plane = 0; plane < 128; ++plane)
    {
        if (planeOffsets->offsets[plane].type == 0 && planeOffsets->offsets[plane].offset == 0)
        {
            encoded[plane] = 0x80;
            continue;
        }
        encoded[plane] = static_cast<unsigned char>(block++);
        for (int i = 0; i < 128; ++i)
        {
            unsigned char low = static_cast<unsigned char>(planeOffsets->getData(plane, i * 2));
            unsigned char high = static_cast<unsigned char>(planeOffsets->getData(plane, i * 2 + 1));
            destination[i] = (unsigned char)(low | (high << 4));
        }
        destination += blockSize;
    }
    stream->writeInt((int)nonEmpty);
    stream->write(arrayWithLength<unsigned char>(encoded, totalSize));
    delete[] encoded;
}
void SparseDataStorage::read(DataInputStream* stream)
{
    const unsigned int nonEmpty = (unsigned int)stream->readInt();
    const unsigned int inputSize = 0x80 * (nonEmpty + 1);
    unsigned char* encoded = static_cast<unsigned char*>(malloc(inputSize));
    stream->readFully(arrayWithLength<unsigned char>(encoded, inputSize));

    const unsigned int requiredSize = nonEmpty * 0x80 + 0x100;
    PlaneOffsets* destination;
    bool replaced = false;
    if (getPlaneOffsetsDataSize() != requiredSize)
    {
        destination = allocatePlaneOffsets(requiredSize);
        replaced = true;
    }
    else
    {
        destination = getPlaneOffsets();
    }
    unsigned char* payload = reinterpret_cast<unsigned char*>(destination) + 0x100;
    for (int plane = 0; plane < 128; ++plane)
    {
        const unsigned char directory = encoded[plane];
        if (directory == 0x80)
        {
            destination->offsets[plane].type = 0;
            destination->offsets[plane].offset = 0;
            payload += destination->getDataSize(plane);
            continue;
        }
        destination->offsets[plane].type = 3;
        destination->setDataOffset(
            plane, static_cast<unsigned int>(payload - reinterpret_cast<unsigned char*>(destination)) >> 2);
        const unsigned char* source = &encoded[0x80 + directory * 0x80];
        for (int group = 0; group < 16; ++group)
            for (int pair = 0; pair < 8; ++pair)
            {
                unsigned char packed = source[group * 8 + pair];
                destination->setData(plane, group * 16 + pair * 2, packed & 15);
                destination->setData(plane, group * 16 + pair * 2 + 1, packed >> 4);
            }
        payload += destination->getDataSize(plane);
    }
    if (replaced)
        updateDataAndCount(destination, (int)requiredSize);
    compress();
    free(encoded);
}
