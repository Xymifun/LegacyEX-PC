#include "game/data/SparseLightStorage.hpp"


#include <cstdlib>
#include <cstring>
#include <cstddef>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/XLockFreeStack.hpp"
#include "game/core/XMem.hpp"

long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand);

static const int CHUNK_WIDTH = 16;
static const int PLANE_COUNT = 128;
static const int PLANE_BYTES = 128;
static const int PLANE_VALUES = 256;
static const int COLUMN_BYTES = 64;
static const unsigned int PLANE_ALL_DARK = 0x80;
static const unsigned int PLANE_ALL_BRIGHT = 0x81;
static const int PENDING_DELETE_LIST_COUNT = 3;
static const int PLANE_INDEX_SHIFT = 7;
static const int PLANE_COUNT_SHIFT = 48;
static const long long PLANE_COUNT_MASK = 0xffff000000000000LL;

static int sAllocatedBytes = 0;

static int sPendingDeleteList;
static XLockFreeStack<unsigned char> sPendingDeleteLists[PENDING_DELETE_LIST_COUNT];

SparseLightStorage::SparseLightStorage(bool allBright)
{
    int planeCount = PLANE_COUNT - 1;
    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);
    unsigned char* planeData = newData + PLANE_COUNT;

    sAllocatedBytes += size;

    newData[PLANE_COUNT - 1] = (unsigned char)(allBright ? PLANE_ALL_BRIGHT : PLANE_ALL_DARK);

    for (int plane = 0; plane < planeCount; plane++)
    {
        newData[plane] = (unsigned char)plane;
    }

    XMemSet(planeData, 0, planeCount * PLANE_BYTES);

    mDataAndCount = ((long long)planeCount << PLANE_COUNT_SHIFT) | ((long long)newData & DATA_POINTER_MASK);
}

SparseLightStorage::SparseLightStorage(bool allBright, bool compressed)
{
    unsigned char* newData = (unsigned char*)malloc(PLANE_BYTES);

    sAllocatedBytes += PLANE_BYTES;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        newData[plane] = (unsigned char)(allBright ? PLANE_ALL_BRIGHT : PLANE_ALL_DARK);
    }

    mDataAndCount = (uintptr_t)newData;
}

int SparseLightStorage::getDataSize(long long dataAndCount)
{
    return (int)((unsigned long long)dataAndCount >> PLANE_COUNT_SHIFT) * PLANE_BYTES + PLANE_BYTES;
}

SparseLightStorage::SparseLightStorage(SparseLightStorage* other)
{
    long long otherDataAndCount = other->mDataAndCount;
    unsigned char* sourceData = (unsigned char*)(otherDataAndCount & DATA_POINTER_MASK);

    int size = getDataSize(otherDataAndCount);

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    mDataAndCount = ((long long)newData & DATA_POINTER_MASK) | (otherDataAndCount & PLANE_COUNT_MASK);

    XMemCpy(newData, sourceData, size);
}

SparseLightStorage::~SparseLightStorage()
{
    unsigned char* data = (unsigned char*)(mDataAndCount & DATA_POINTER_MASK);

    int size = getDataSize(mDataAndCount);

    mDataAndCount = 0;

    free(data);

    sAllocatedBytes -= size;
}

void SparseLightStorage::queueForDelete(unsigned char* data, int size)
{
    sPendingDeleteLists[sPendingDeleteList].Push(data);
}

void SparseLightStorage::updateDataAndCount(long long newDataAndCount)
{
    long long lastDataAndCount;
    long long returnedDataAndCount;
    unsigned char* lastData;
    int lastSize;
    do
    {
        lastDataAndCount = mDataAndCount;
        lastData = (unsigned char*)(lastDataAndCount & DATA_POINTER_MASK);
        lastSize = getDataSize(lastDataAndCount);
        returnedDataAndCount = InterlockedCompareExchangeRelease64(&mDataAndCount, newDataAndCount, lastDataAndCount);
    } while (returnedDataAndCount != lastDataAndCount);

    queueForDelete(lastData, lastSize);
}

void SparseLightStorage::setData(arrayWithLength<unsigned char> data, unsigned int offset)
{
    unsigned char planeIndices[PLANE_COUNT];

    int planeCount = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        bool allDark = true;
        bool allBright = true;

        for (int i = 0; i < PLANE_VALUES; i++)
        {
            int position = (i << PLANE_INDEX_SHIFT) | plane;
            int value = (data.data[(position >> 1) + offset] >> ((position & 1) << 2)) & 0xf;

            if (value == 0)
            {
                allBright = false;
            }
            else
            {
                allDark = false;

                if (value != 0xf)
                {
                    allBright = false;
                }
            }
        }

        if (allDark)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
        }
        else
        {
            unsigned char planeIndex = (unsigned char)PLANE_ALL_BRIGHT;

            if (!allBright)
            {
                planeIndex = (unsigned char)planeCount;
                planeCount++;
            }

            planeIndices[plane] = planeIndex;
        }
    }

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    unsigned char* target = newData + PLANE_COUNT;

    XMemCpy(newData, planeIndices, PLANE_COUNT);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (newData[plane] < PLANE_ALL_DARK)
        {
            const unsigned char* column = data.data + offset + (plane >> 1);

            int shift = (plane & 1) * 4;

            for (int i = 0; i < PLANE_VALUES; i += 2)
            {
                *target = (unsigned char)((*column >> shift) & 0xf);
                column += COLUMN_BYTES;
                *target |= (unsigned char)(((*column >> shift) & 0xf) << 4);
                column += COLUMN_BYTES;

                target++;
            }
        }
    }

    updateDataAndCount(((long long)planeCount << PLANE_COUNT_SHIFT) | ((long long)newData & DATA_POINTER_MASK));
}

void SparseLightStorage::getPlaneIndicesAndData(unsigned char** planeIndices, unsigned char** planeData)
{
    unsigned char* data = (unsigned char*)(mDataAndCount & DATA_POINTER_MASK);

    *planeIndices = data;
    *planeData = data + PLANE_COUNT;
}

void SparseLightStorage::getData(arrayWithLength<unsigned char> data, unsigned int offset)
{
    XMemSet(data.data + offset, 0, PLANE_COUNT * PLANE_BYTES);

    unsigned char* planeIndices;
    unsigned char* planeData;

    getPlaneIndicesAndData(&planeIndices, &planeData);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        unsigned int index = planeIndices[plane];

        if (index != PLANE_ALL_DARK)
        {
            int shift = (plane & 1) * 4;

            unsigned char* column = data.data + ((plane >> 1) + offset);

            if (index == PLANE_ALL_BRIGHT)
            {
                for (int i = 0; i < PLANE_VALUES; i++)
                {
                    column[i * COLUMN_BYTES] |= (unsigned char)(0xf << shift);
                }
            }
            else
            {
                const unsigned char* source = planeData + index * PLANE_BYTES;

                for (int i = 0; i < PLANE_VALUES; i += 2)
                {
                    *column |= (unsigned char)((*source & 0xf) << shift);
                    column += COLUMN_BYTES;
                    *column |= (unsigned char)(((*source >> 4) & 0xf) << shift);
                    column += COLUMN_BYTES;

                    source++;
                }
            }
        }
    }
}

int SparseLightStorage::get(int x, int y, int z)
{
    unsigned char* planeIndices;
    unsigned char* planeData;

    getPlaneIndicesAndData(&planeIndices, &planeData);

    unsigned int index = planeIndices[y];

    if (index == PLANE_ALL_DARK)
    {
        return 0;
    }

    if (index == PLANE_ALL_BRIGHT)
    {
        return 0xf;
    }

    int value = x * CHUNK_WIDTH + z;

    return (planeData[index * PLANE_BYTES + value / 2] >> ((value & 1) * 4)) & 0xf;
}

void SparseLightStorage::addNewPlane(std::vector<int>& planes)
{
    for (;;)
    {
        long long lastDataAndCount = mDataAndCount;
        unsigned char* oldData = (unsigned char*)(lastDataAndCount & DATA_POINTER_MASK);
        unsigned int oldPlaneCount = (unsigned int)((unsigned long long)lastDataAndCount >> PLANE_COUNT_SHIFT);
        int oldSize = getDataSize(lastDataAndCount);

        unsigned int planeCount = oldPlaneCount;

        std::vector<int>::iterator it = planes.begin();

        while (it != planes.end())
        {
            if (oldData[*it] < PLANE_ALL_DARK)
            {
                it = planes.erase(it);
            }
            else
            {
                ++it;
                planeCount++;
            }
        }

        int size = planeCount * PLANE_BYTES + PLANE_BYTES;

        unsigned char* newData = (unsigned char*)malloc(size);

        sAllocatedBytes += size;

        XMemCpy(newData, oldData, oldPlaneCount * PLANE_BYTES + PLANE_BYTES);

        unsigned char* newPlaneData = newData + PLANE_COUNT;
        unsigned char nextPlane = (unsigned char)oldPlaneCount;

        for (std::vector<int>::iterator plane = planes.begin(); plane != planes.end(); ++plane)
        {
            int fill = 0;

            if (oldData[*plane] >= PLANE_ALL_DARK)
            {
                if (oldData[*plane] <= PLANE_ALL_DARK)
                {
                    XMemSet(newPlaneData + nextPlane * PLANE_BYTES, 0, PLANE_BYTES);
                }
                else
                {
                    if (oldData[*plane] == PLANE_ALL_BRIGHT)
                    {
                        fill = 0xff;
                    }

                    XMemSet(newPlaneData + nextPlane * PLANE_BYTES, fill, PLANE_BYTES);
                }

                newData[*plane] = nextPlane;

                nextPlane++;
            }
        }

        long long newDataAndCount = ((long long)planeCount << PLANE_COUNT_SHIFT)
                                    | ((long long)newData & DATA_POINTER_MASK);
        long long returnedDataAndCount = InterlockedCompareExchangeRelease64(&mDataAndCount, newDataAndCount,
                                                                             lastDataAndCount);
        if (returnedDataAndCount == lastDataAndCount)
        {
            queueForDelete(oldData, oldSize);
            return;
        }

        free(newData);
    }
}

void SparseLightStorage::set(int x, int y, int z, int value)
{
    unsigned char* planeIndices;
    unsigned char* planeData;

    getPlaneIndicesAndData(&planeIndices, &planeData);

    if (planeIndices[y] >= PLANE_ALL_DARK)
    {
        if (value == 0)
        {
            if (planeIndices[y] == PLANE_ALL_DARK)
            {
                return;
            }
        }
        else if (value == 0xf && planeIndices[y] == PLANE_ALL_BRIGHT)
        {
            return;
        }

        {
            std::vector<int> planes;

            planes.push_back(y);

            addNewPlane(planes);
        }

        getPlaneIndicesAndData(&planeIndices, &planeData);
    }

    int position = x * CHUNK_WIDTH + z;
    int shift = (position & 1) * 4;

    int mask = 0xf0 >> shift;
    int offset = planeIndices[y] * PLANE_BYTES + position / 2;

    planeData[offset] = (unsigned char)((planeData[offset] & mask) | (value << shift));
}

void SparseLightStorage::setAllBright()
{
    unsigned char* newData = (unsigned char*)malloc(PLANE_BYTES);

    sAllocatedBytes += PLANE_BYTES;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        newData[plane] = (unsigned char)PLANE_ALL_BRIGHT;
    }

    updateDataAndCount((uintptr_t)newData);
}

int SparseLightStorage::setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1,
                                      int z1, int offset)
{
    int planePairs = (y1 - y0) / 2;
    int y = y0 & ~1;

    {
        std::vector<int> planes;

        planes.reserve(planePairs * 2);

        for (int i = 0; i < planePairs; i++)
        {
            int plane = y + i * 2;

            planes.push_back(plane);
            planes.push_back(plane + 1);
        }

        addNewPlane(planes);
    }

    unsigned char* source = data.data + offset;
    unsigned char* current = source;

    for (int x = x0; x < x1; x++)
    {
        for (int z = z0; z < z1; z++)
        {
            for (int i = 0; i < planePairs; i++)
            {
                int plane = y + i * 2;

                set(x, plane, z, *current & 0xf);
                set(x, plane + 1, z, (*current >> 4) & 0xf);

                current++;
            }
        }
    }

    return (int)(current - source);
}

int SparseLightStorage::getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1,
                                      int z1, int offset)
{
    unsigned char* destination = data.data + offset;
    unsigned char* current = destination;

    for (int x = x0; x < x1; x++)
    {
        for (int z = z0; z < z1; z++)
        {
            int planePairs = (y1 - y0) / 2;
            int y = y0 & ~1;

            for (int i = 0; i < planePairs; i++)
            {
                int plane = y + i * 2;

                *current = get(x, plane, z);
                *current |= get(x, plane + 1, z) << 4;

                current++;
            }
        }
    }

    return (int)(current - destination);
}

void SparseLightStorage::staticCtor()
{
}

unsigned int SparseLightStorage::compress()
{
    long long lastDataAndCount = mDataAndCount;
    unsigned int planeCount = 0;
    bool changed = false;
    int oldSize = getDataSize(lastDataAndCount);
    unsigned char* oldData = (unsigned char*)(lastDataAndCount & DATA_POINTER_MASK);

    const unsigned char* oldPlaneData = oldData + PLANE_COUNT;

    unsigned char planeIndices[PLANE_COUNT];

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (oldData[plane] == PLANE_ALL_DARK)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
        }
        else if (oldData[plane] == PLANE_ALL_BRIGHT)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
        }
        else
        {
            const unsigned char* values = (const unsigned char*)(oldPlaneData + oldData[plane] * PLANE_BYTES);

            bool allDark = true;
            bool allBright = true;

            for (int i = 0; i < PLANE_BYTES; i++)
            {
                if (*values == 0)
                {
                    allBright = false;
                }
                else
                {
                    allDark = false;

                    if (*values != 0xff)
                    {
                        allBright = false;
                    }
                }

                values++;
            }

            if (allDark)
            {
                changed = true;
                planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
            }
            else if (allBright)
            {
                changed = true;
                planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
            }
            else
            {
                planeIndices[plane] = (unsigned char)planeCount;
                planeCount++;
            }
        }
    }

    if (changed)
    {
        int size = planeCount * PLANE_BYTES + PLANE_BYTES;

        unsigned char* newData = (unsigned char*)malloc(size);

        sAllocatedBytes += size;

        unsigned char* target = newData + PLANE_COUNT;

        XMemCpy(newData, planeIndices, PLANE_COUNT);

        for (int plane = 0; plane < PLANE_COUNT; plane++)
        {
            if (newData[plane] < PLANE_ALL_DARK)
            {
                XMemCpy(target, oldPlaneData + oldData[plane] * PLANE_BYTES, PLANE_BYTES);

                target += PLANE_BYTES;
            }
        }

        long long newDataAndCount = ((long long)planeCount << PLANE_COUNT_SHIFT)
                                    | ((long long)newData & DATA_POINTER_MASK);
        if (InterlockedCompareExchangeRelease64(&mDataAndCount, newDataAndCount, lastDataAndCount) != lastDataAndCount)
        {
            queueForDelete(newData, size);
        }
        else
        {
            queueForDelete(oldData, oldSize);
        }

        return planeCount;
    }

    return (unsigned int)((unsigned long long)lastDataAndCount >> PLANE_COUNT_SHIFT);
}

bool SparseLightStorage::isCompressed()
{
    return (int)((unsigned long long)mDataAndCount >> PLANE_COUNT_SHIFT) < PLANE_COUNT - 1;
}

unsigned int SparseLightStorage::getRawDataMaxBytes()
{
    return PLANE_COUNT * PLANE_BYTES + PLANE_BYTES;
}

unsigned int SparseLightStorage::getRawData(unsigned char* buffer)
{
    long long dataAndCount = mDataAndCount;
    unsigned char* data = (unsigned char*)(dataAndCount & DATA_POINTER_MASK);
    unsigned int planeCount = (unsigned int)((unsigned long long)dataAndCount >> PLANE_COUNT_SHIFT);
    unsigned int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    memcpy(buffer, data, size);

    return size;
}

void SparseLightStorage::setFromRawData(unsigned char* rawData, unsigned int size)
{
    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    int planeCount = (size - PLANE_BYTES) / PLANE_BYTES;
    long long newDataAndCount = ((long long)newData & DATA_POINTER_MASK) | ((long long)planeCount << PLANE_COUNT_SHIFT);

    memcpy(newData, rawData, size);

    updateDataAndCount(newDataAndCount);
}

void SparseLightStorage::tick()
{
    XLockFreeStack<unsigned char>& pending = sPendingDeleteLists[(sPendingDeleteList + 1) % PENDING_DELETE_LIST_COUNT];

    unsigned char* data;

    do
    {
        data = pending.Pop();

        free(data);
    } while (data != NULL);

    sPendingDeleteList = (sPendingDeleteList + 1) % PENDING_DELETE_LIST_COUNT;
}

void SparseLightStorage::write(DataOutputStream* out)
{
    unsigned int planeCount = (unsigned int)((unsigned long long)mDataAndCount >> PLANE_COUNT_SHIFT);

    out->writeInt(planeCount);

    arrayWithLength<unsigned char> data((unsigned char*)(mDataAndCount & DATA_POINTER_MASK),
                                        planeCount * PLANE_BYTES + PLANE_BYTES);

    out->write(data);
}

void SparseLightStorage::read(DataInputStream* in, unsigned char* buffer)
{
    unsigned char* rawData = buffer;

    if (rawData == NULL)
    {
        rawData = (unsigned char*)malloc(PLANE_COUNT * PLANE_BYTES + PLANE_BYTES);
    }

    arrayWithLength<unsigned char> raw;

    raw.data = rawData;
    raw.length = in->readInt() * PLANE_BYTES + PLANE_BYTES;

    in->readFully(raw);

    const unsigned char* rawPlaneData = rawData + PLANE_COUNT;

    unsigned char planeIndices[PLANE_COUNT];

    int planeCount = 0;

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        unsigned int index = rawData[plane];

        if (index == PLANE_ALL_DARK)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
        }
        else if (index == PLANE_ALL_BRIGHT)
        {
            planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
        }
        else
        {
            const unsigned char* values = (const unsigned char*)(rawPlaneData + index * PLANE_BYTES);

            bool allDark = true;
            bool allBright = true;

            for (int i = 0; i < PLANE_BYTES; i++)
            {
                if (values[i] == 0)
                {
                    allBright = false;
                }
                else
                {
                    allDark = false;

                    if (values[i] != 0xff)
                    {
                        allBright = false;
                    }
                }
            }

            if (allDark)
            {
                planeIndices[plane] = (unsigned char)PLANE_ALL_DARK;
            }
            else if (allBright)
            {
                planeIndices[plane] = (unsigned char)PLANE_ALL_BRIGHT;
            }
            else
            {
                planeIndices[plane] = (unsigned char)planeCount;
                planeCount++;
            }
        }
    }

    int size = planeCount * PLANE_BYTES + PLANE_BYTES;

    unsigned char* newData = (unsigned char*)malloc(size);

    sAllocatedBytes += size;

    unsigned char* target = newData + PLANE_COUNT;

    XMemCpy(newData, planeIndices, PLANE_COUNT);

    for (int plane = 0; plane < PLANE_COUNT; plane++)
    {
        if (newData[plane] < PLANE_ALL_DARK)
        {
            XMemCpy(target, rawPlaneData + rawData[plane] * PLANE_BYTES, PLANE_BYTES);

            target += PLANE_BYTES;
        }
    }

    updateDataAndCount(((long long)planeCount << PLANE_COUNT_SHIFT) | ((long long)newData & DATA_POINTER_MASK));

    if (buffer == NULL)
    {
        free(rawData);
    }
}
