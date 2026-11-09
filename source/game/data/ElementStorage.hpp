#ifndef GAME_DATA_ELEMENTSTORAGE_HPP
#define GAME_DATA_ELEMENTSTORAGE_HPP

#include "game/data/HierarchicalElementStorage.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CompressedBlockStorage;
class DataInputStream;
class DataOutputStream;
class SparseDataStorage;

class ElementStorage : public HierarchicalElementStorage
{
public:
    class ThreadStorage
    {
    public:
        unsigned int mThreadId;
        int mReadLocks;
        bool mHasTempData;
        bool mBlockReadLock;
        bool mDataLock;
        unsigned char* mTempData;
    };
    enum eHESThreadId
    {
        eHESThreadId_Server = 1,
        eHESThreadId_ServerPostUpdate = 2,
        eHESThreadId_ServerLevel = 7,
        eHESThreadId_NetworkGame = 8,
        eHESThreadId_Main = 10,
        eHESThreadId_Renderer = 11,
        eHESThreadId_GameModeTransition = 16,
        eHESThreadId_RemoteSave = 17,
        eHESThreadId_ClientChunkStorage = 18
    };
    ElementStorage(bool threaded);
    ElementStorage(bool threaded, ElementStorage* other);
    ElementStorage(bool threaded, arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data,
                   unsigned int headroom);
    ElementStorage(bool threaded, arrayWithLength<unsigned short> elements, unsigned int headroom);
    ElementStorage(bool threaded, CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks,
                   SparseDataStorage* data, SparseDataStorage* extraData);
    int getCriticalSectionIndex(bool mainThread);
    void getWriteLock(int reason);
    void releaseWriteLock();
    void removeCompressionHeadroom();
    bool shouldLockForRead(ThreadStorage* storage);
    void getReadLock(int reason);
    void releaseReadLock();
    bool isRenderChunkEmpty(int baseY, int* highestNonEmptyY);
    void setData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data);
    void setData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements);
    void getData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, int minSlice,
                 int maxSlice);
    void getData(arrayWithLength<unsigned short> elements, int minSlice, int maxSlice);
    void getData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, int minSlice,
                 int maxSlice);
    unsigned short get(unsigned int x, unsigned int y, unsigned int z, bool* isUniformSection);
    unsigned short getExtraElement(unsigned int x, unsigned int y, unsigned int z);
    void getElementAndExtraElement_NoLock(unsigned int x, unsigned int y, unsigned int z, unsigned short* element,
                                          unsigned short* extraElement);
    void getElementAndExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short* element,
                                   unsigned short* extraElement);
    void getReadLockExternal();
    void releaseReadLockExternal();
    void set(unsigned int x, unsigned int y, unsigned int z, unsigned short value);
    void setExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short value);
    int setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset,
                      void (*changedCallback)(int, int, int, void*, int), void* callbackData, int callbackValue);
    int getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset);
    unsigned int getFrameLastUpdated();
    unsigned int getAllocatedBytes();
    unsigned int getRawData(unsigned char* rawData);
    void setFromRawData(unsigned char* rawData, unsigned int rawDataBytes);
    static int getRawDataMaxBytes();
    bool sectionContainsElementData(int x, int y, int z, unsigned short value, unsigned short mask);
    int getHighestNonEmptyY();
    void write(DataOutputStream* out);
    void read(DataInputStream* in);
    static void CreateNewThreadStorage(eHESThreadId threadId);
    static void ReleaseThreadStorage();
    static void staticCtor();
    virtual void getReadLockUnconditional(int reason);
    virtual void releaseReadLockUnconditional();
    virtual unsigned char* getTempData();
    virtual ~ElementStorage();
    unsigned char* getAllocatedRawData();

private:
    unsigned int mFrameLastUpdated;
    int mCriticalSectionIndex;
};

#endif
