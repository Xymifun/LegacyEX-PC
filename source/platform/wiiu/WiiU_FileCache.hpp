#ifndef SOURCE_PLATFORM_WIIU_WIIU_FILECACHE_HPP
#define SOURCE_PLATFORM_WIIU_WIIU_FILECACHE_HPP

#include <stdint.h>
#include <coreinit/filesystem.h>

struct _OVERLAPPED;

unsigned int GetFileSize(int file, unsigned int* fileSizeHigh);
int ReadFile(int file, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead, _OVERLAPPED* overlapped);
int WriteFile(int file, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten,
              _OVERLAPPED* overlapped);
int SetFilePointer(int file, long distanceToMove, int* distanceToMoveHigh, unsigned int moveMethod);
unsigned int GetFileAttributesA(const char* fileName);
void* VirtualAlloc(void* address, unsigned long size, unsigned int allocationType, unsigned int protect);
int VirtualFree(void* address, unsigned long size, unsigned int freeType);

static const unsigned int FILE_CACHE_BUFFER_SIZE = 0x10000;

extern unsigned char* s_writeBuffer;
extern OSFastMutex s_writeMutex;
extern OSFastMutex s_commandBlockMutex;

class WiiU_FileCache
{
    friend int SetFilePointer(int file, long distanceToMove, int* distanceToMoveHigh, unsigned int moveMethod);

private:
    unsigned char* m_cache;
    int m_file;
    unsigned int m_fileSize;
    int m_filePos;
    int m_cacheLength;
    int m_cachePos;

public:
    WiiU_FileCache(int file);
    ~WiiU_FileCache();
    static WiiU_FileCache* GetFileCache(int file);
    int bytesAvailableInCache();
    void fillCache();
    unsigned int Read(void* buffer, unsigned int size);
    unsigned int SetFilePos(int position);
    static void CreateFileCache(int file);
    bool Close();
    static bool DeleteFileCache(int file);
};

#endif
