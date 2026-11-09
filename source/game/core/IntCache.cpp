#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int sTlsIndex = TlsAlloc();

IntCache::ThreadStorage::ThreadStorage()
{
}

IntCache::ThreadStorage::~ThreadStorage()
{
    for (unsigned int i = 0; i < mFreeSmallArrays.size(); ++i)
    {
        delete[] mFreeSmallArrays[i].data;
    }

    for (unsigned int i = 0; i < mInUseSmallArrays.size(); ++i)
    {
        delete[] mInUseSmallArrays[i].data;
    }

    for (unsigned int i = 0; i < mFreeLargeArrays.size(); ++i)
    {
        delete[] mFreeLargeArrays[i].data;
    }

    for (unsigned int i = 0; i < mInUseLargeArrays.size(); ++i)
    {
        delete[] mInUseLargeArrays[i].data;
    }

    for (unsigned int i = 0; i < mOldLargeArrays.size(); ++i)
    {
        delete[] mOldLargeArrays[i].data;
    }
}

arrayWithLength<int> IntCache::allocate(int size)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sTlsIndex));

    if (size <= 256)
    {
        if (storage->mFreeSmallArrays.empty())
        {
            PIXBeginNamedEvent(0.0f, "IntCache::allocate tiny");
            arrayWithLength<int> allocated(256, false);
            storage->mInUseSmallArrays.push_back(allocated);
            PIXEndNamedEvent();
            return allocated;
        }
        else
        {
            arrayWithLength<int> reused = storage->mFreeSmallArrays.back();
            storage->mFreeSmallArrays.pop_back();
            storage->mInUseSmallArrays.push_back(reused);
            return reused;
        }
    }
    else if (size > storage->mIntCacheSize)
    {
        PIXBeginNamedEvent(0.0f, "IntCache::allocate new max size");
        storage->mIntCacheSize = size;
        storage->mOldLargeArrays.insert(storage->mOldLargeArrays.end(), storage->mFreeLargeArrays.begin(),
                                        storage->mFreeLargeArrays.end());
        storage->mOldLargeArrays.insert(storage->mOldLargeArrays.end(), storage->mInUseLargeArrays.begin(),
                                        storage->mInUseLargeArrays.end());
        storage->mFreeLargeArrays.clear();
        storage->mInUseLargeArrays.clear();
        arrayWithLength<int> allocated(storage->mIntCacheSize, false);
        storage->mInUseLargeArrays.push_back(allocated);
        PIXEndNamedEvent();
        return allocated;
    }
    else if (storage->mFreeLargeArrays.empty())
    {
        PIXBeginNamedEvent(0.0f, "IntCache::allocate current max size");
        arrayWithLength<int> allocated(storage->mIntCacheSize, false);
        PIXEndNamedEvent();
        storage->mInUseLargeArrays.push_back(allocated);
        return allocated;
    }
    else
    {
        arrayWithLength<int> reused = storage->mFreeLargeArrays.back();
        storage->mFreeLargeArrays.pop_back();
        storage->mInUseLargeArrays.push_back(reused);
        return reused;
    }
}

void IntCache::releaseAll()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sTlsIndex));

    for (unsigned int i = 0; i < storage->mOldLargeArrays.size(); ++i)
    {
        delete[] storage->mOldLargeArrays[i].data;
    }
    storage->mOldLargeArrays.clear();

    if (!storage->mFreeLargeArrays.empty())
    {
        delete[] storage->mFreeLargeArrays.back().data;
        storage->mFreeLargeArrays.pop_back();
    }

    if (!storage->mFreeSmallArrays.empty())
    {
        delete[] storage->mFreeSmallArrays.back().data;
        storage->mFreeSmallArrays.pop_back();
    }

    storage->mFreeLargeArrays.insert(storage->mFreeLargeArrays.end(), storage->mInUseLargeArrays.begin(),
                                     storage->mInUseLargeArrays.end());
    storage->mFreeSmallArrays.insert(storage->mFreeSmallArrays.end(), storage->mInUseSmallArrays.begin(),
                                     storage->mInUseSmallArrays.end());

    storage->mInUseLargeArrays.clear();
    storage->mInUseSmallArrays.clear();
}

#ifdef MINECRAFT_PC
IntCache::PcMark IntCache::pcMark()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sTlsIndex));
    PcMark mark;
    mark.large = storage->mInUseLargeArrays.size();
    mark.small = storage->mInUseSmallArrays.size();
    return mark;
}

void IntCache::pcReleaseSince(const PcMark& mark)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sTlsIndex));
    const size_t large = storage->mInUseLargeArrays.size() >= mark.large ? mark.large : 0;
    storage->mFreeLargeArrays.insert(storage->mFreeLargeArrays.end(), storage->mInUseLargeArrays.begin() + large,
                                     storage->mInUseLargeArrays.end());
    storage->mInUseLargeArrays.resize(large);
    const size_t small = storage->mInUseSmallArrays.size() >= mark.small ? mark.small : 0;
    storage->mFreeSmallArrays.insert(storage->mFreeSmallArrays.end(), storage->mInUseSmallArrays.begin() + small,
                                     storage->mInUseSmallArrays.end());
    storage->mInUseSmallArrays.resize(small);
}
#endif

void IntCache::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();
    TlsSetValue(sTlsIndex, storage);
    storage->mIntCacheSize = 256;
}

void IntCache::ReleaseThreadStorage()
{
    delete static_cast<ThreadStorage*>(TlsGetValue(sTlsIndex));
}

void IntCache::Reset()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sTlsIndex));
    storage->mIntCacheSize = 256;

    for (unsigned int i = 0; i < storage->mInUseLargeArrays.size(); ++i)
    {
        delete[] storage->mInUseLargeArrays[i].data;
    }
    storage->mInUseLargeArrays.clear();

    for (unsigned int i = 0; i < storage->mFreeLargeArrays.size(); ++i)
    {
        delete[] storage->mFreeLargeArrays[i].data;
    }
    storage->mFreeLargeArrays.clear();

    for (unsigned int i = 0; i < storage->mInUseSmallArrays.size(); ++i)
    {
        delete[] storage->mInUseSmallArrays[i].data;
    }
    storage->mInUseSmallArrays.clear();

    for (unsigned int i = 0; i < storage->mFreeSmallArrays.size(); ++i)
    {
        delete[] storage->mFreeSmallArrays[i].data;
    }
    storage->mFreeSmallArrays.clear();

    for (unsigned int i = 0; i < storage->mOldLargeArrays.size(); ++i)
    {
        delete[] storage->mOldLargeArrays[i].data;
    }
    storage->mOldLargeArrays.clear();
}
