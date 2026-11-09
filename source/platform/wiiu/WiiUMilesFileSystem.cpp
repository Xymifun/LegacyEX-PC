#include "platform/wiiu/WiiUMilesFileSystem.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "game/core/CriticalSection.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>

#if defined(MINECRAFT_PC) && __SIZEOF_POINTER__ == 8
extern "C" void* platform_alloc(unsigned int uiSize);
extern "C" void platform_free(void* pBlock);
#endif

struct CFileHandle
{
#if defined(MINECRAFT_PC) && __SIZEOF_POINTER__ == 8
    static void* operator new(size_t size) { return platform_alloc(static_cast<unsigned int>(size)); }
    static void operator delete(void* block) { platform_free(block); }
#endif

    CFileHandle(const char* filename, int fileHandle, unsigned int fileSize)
    {
        mFileHandle = fileHandle;
        mCurrentPos = 0;
        mStartOffset = 0;
        mLength = 0;
        mFileSize = fileSize;
        mValidPosition = 1;
        strncpy(mFilename, filename, 0x104);
    }

    char mFilename[0x104];
    int mFileHandle;
    unsigned int mStartOffset;
    unsigned int mLength;
    unsigned int mCurrentPos;
    unsigned char mValidPosition;
    char mPadding[3];
    unsigned int mFileSize;
};

static unsigned char sInitialized = 0;
static std::vector<CFileHandle*> sFileHandles;
static OSFastMutex sMutex;

void InvalidePositionsForOtherHandles(CFileHandle* handle)
{
    EnterCriticalSection(&sMutex);
    for (size_t i = 0; i < sFileHandles.size(); ++i)
    {
        if (sFileHandles[i] != handle && sFileHandles[i]->mFileHandle == handle->mFileHandle)
        {
            sFileHandles[i]->mValidPosition = 0;
        }
    }
    LeaveCriticalSection(&sMutex);
}

unsigned int file_open_callback(const char* filename, unsigned int* handleOut)
{
    if (sInitialized == 0)
    {
        InitializeCriticalSection(&sMutex);
        sInitialized = 1;
    }

    if (filename[0] == '*')
    {
        char localName[260];
        const char* p = filename + 1;
        char* dst = localName;
        while (*p != '*')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        p++;

        CFileHandle* handle = NULL;
        for (size_t i = 0; i < sFileHandles.size(); ++i)
        {
            if (strcmp(sFileHandles[i]->mFilename, localName) == 0)
            {
                handle = new CFileHandle(filename, sFileHandles[i]->mFileHandle, sFileHandles[i]->mFileSize);
                break;
            }
        }

        if (handle == NULL)
        {
            int fd = CreateFileA(localName, 0x80000000, 0, NULL, 3, 0, 0);
            int size = SetFilePointer(fd, 0, NULL, 2);
            SetFilePointer(fd, 0, NULL, 0);
            if (fd == 0)
            {
                *handleOut = 0;
                return 0;
            }
            handle = new CFileHandle(localName, fd, size);
        }

        dst = localName;
        while (*p != '*')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        handle->mLength = atoi(localName);
        p++;

        dst = localName;
        while (*p != '.')
        {
            *dst++ = *p++;
        }
        *dst = '\0';
        int length = atoi(localName);
        handle->mStartOffset = length;
        handle->mCurrentPos = length;
        handle->mValidPosition = 0;
        InvalidePositionsForOtherHandles(handle);
        EnterCriticalSection(&sMutex);
        sFileHandles.push_back(handle);
        LeaveCriticalSection(&sMutex);
        *handleOut = (unsigned int)(uintptr_t)handle;
        return 1;
    }

    int fd = CreateFileA(filename, 0x80000000, 0, NULL, 3, 0, 0);
    if (fd == 0)
    {
        *handleOut = 0;
        return 0;
    }

    int size = SetFilePointer(fd, 0, NULL, 2);
    SetFilePointer(fd, 0, NULL, 0);
    CFileHandle* handle = new CFileHandle(filename, fd, size);
    EnterCriticalSection(&sMutex);
    sFileHandles.push_back(handle);
    LeaveCriticalSection(&sMutex);
    *handleOut = (unsigned int)(uintptr_t)handle;
    return 1;
}

void file_close_callback(unsigned int handleVal)
{
    CFileHandle* handle = (CFileHandle*)handleVal;
    int fd = handle->mFileHandle;

    EnterCriticalSection(&sMutex);
    for (size_t i = 0; i < sFileHandles.size(); ++i)
    {
        if (sFileHandles[i] == handle)
        {
            delete handle;
            sFileHandles[i] = sFileHandles[sFileHandles.size() - 1];
            sFileHandles.pop_back();
            break;
        }
    }

    for (size_t i = 0; i < sFileHandles.size(); ++i)
    {
        if (sFileHandles[i]->mFileHandle == fd)
        {
            LeaveCriticalSection(&sMutex);
            return;
        }
    }
    LeaveCriticalSection(&sMutex);
    CloseHandle(fd);
}

int file_seek_callback(unsigned int handleVal, int offset, unsigned int origin)
{
    CFileHandle* handle = (CFileHandle*)handleVal;

    if (handle->mLength == 0)
    {
        unsigned int target;
        switch (origin)
        {
        case 1:
            target = handle->mCurrentPos + offset;
            if (target > handle->mFileSize)
                target = handle->mFileSize;
            break;
        case 2:
            target = handle->mFileSize + offset;
            if (target > handle->mFileSize)
                target = handle->mFileSize;
            break;
        default:
            target = offset;
            if (target > handle->mFileSize)
                target = handle->mFileSize;
            break;
        }
        handle->mCurrentPos = target;
        handle->mValidPosition = 0;
        InvalidePositionsForOtherHandles(handle);
        return handle->mCurrentPos;
    }

    unsigned int target;
    switch (origin)
    {
    case 1:
        target = handle->mCurrentPos + offset;
        if (target > handle->mStartOffset + handle->mLength)
            target = handle->mStartOffset + handle->mLength;
        break;
    case 2:
        target = handle->mStartOffset + handle->mLength + offset;
        if (target > handle->mStartOffset + handle->mLength)
            target = handle->mStartOffset + handle->mLength;
        break;
    default:
        target = handle->mStartOffset + offset;
        if (target > handle->mStartOffset + handle->mLength)
            target = handle->mStartOffset + handle->mLength;
        break;
    }
    handle->mCurrentPos = target;
    handle->mValidPosition = 0;
    InvalidePositionsForOtherHandles(handle);
    return handle->mCurrentPos - handle->mStartOffset;
}

int file_read_callback(unsigned int handleVal, void* buffer, unsigned int bytes)
{
    CFileHandle* handle = (CFileHandle*)handleVal;

    if (handle->mValidPosition == 0)
    {
        SetFilePointer(handle->mFileHandle, handle->mCurrentPos, NULL, 0);
        handle->mValidPosition = 1;
    }

    if (handle->mLength != 0)
    {
        unsigned int end = handle->mStartOffset + handle->mLength;
        if (handle->mCurrentPos + bytes > end)
        {
            bytes = end - handle->mCurrentPos;
        }
    }

    unsigned int bytesRead;
    int success = ReadFile(handle->mFileHandle, buffer, bytes, &bytesRead, NULL);
    if (success != 0)
    {
        handle->mCurrentPos += bytesRead;
        InvalidePositionsForOtherHandles(handle);
        return bytesRead;
    }
    return 0;
}
