#include "platform/wiiu/PlatformTiming.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/data/ConsoleSaveFileOriginal.hpp"

#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Compression.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/FileHeader.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/LevelGenerationOptions.hpp"

#ifdef RECON_NATIVE
#include "win32_memory.hpp"
#else
#include "platform/wiiu/WiiU_FileCache.hpp"
#endif
void __debugbreak();

static void* s_saveData = NULL;
static unsigned int s_saveDataSize = 0;

extern C4JStorage* gStorage;

ConsoleSaveFileOriginal::ConsoleSaveFileOriginal(const std::wstring& filename, void* data, unsigned int dataSize,
                                                 bool resetData, ESavePlatform platform)
    : ConsoleSaveFileCommon()
{
    if (s_saveData == NULL)
        s_saveData = VirtualAlloc(NULL, 0x4000000, 0x2000, 4);
    mData = s_saveData;

    unsigned int saveDataSize = dataSize;
    bool deleteBaseSaveData = false;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (data == NULL && options != NULL && options->requiresBaseSave()
        && (data = options->getBaseSaveData(saveDataSize)) != NULL && saveDataSize != 0)
        deleteBaseSaveData = true;
    else if (data == NULL || saveDataSize == 0)
        saveDataSize = gStorage->GetSaveSize();

    if (resetData)
        saveDataSize = 0;

    if (deleteBaseSaveData)
#ifdef MINECRAFT_PC
        platform = PcBaseSavePlatform(data, saveDataSize);
#else
        platform = eSavePlatform_WIIU;
#endif
    _initSaveFromData(filename, saveDataSize, data, platform);
    if (deleteBaseSaveData)
    {
        options->deleteBaseSaveData();
        ConvertToLocalPlatform();
    }
}

ConsoleSaveFileOriginal::~ConsoleSaveFileOriginal()
{
    VirtualFree(s_saveData, 0x4000000, 0x4000);
    s_saveDataSize = 0;
}

int ConsoleSaveFileOriginal::SaveSaveDataCallback(void*, bool)
{
    return 0;
}

void ConsoleSaveFileOriginal::refreshFromLevelgen()
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && options->requiresBaseSave())
    {
        unsigned int dataSize;
        void* data = options->getBaseSaveData(dataSize);
        if (data != NULL && dataSize != 0)
        {
            VirtualFree(s_saveData, 0x4000000, 0x4000);
            s_saveDataSize = 0;
            mFileHeader.Reset();
#ifdef MINECRAFT_PC
            _initSaveFromData(mFilename, dataSize, data, PcBaseSavePlatform(data, dataSize));
#else
            _initSaveFromData(mFilename, dataSize, data, eSavePlatform_WIIU);
#endif
            options->deleteBaseSaveData();
            ConvertToLocalPlatform();
        }
    }
}

void ConsoleSaveFileOriginal::AllocInitialQuota(unsigned int size)
{
    unsigned int initialSize = size;
    if (initialSize < SAVE_DATA_MIN_QUOTA)
        initialSize = SAVE_DATA_MIN_QUOTA;

    unsigned int pages = (initialSize + (SAVE_DATA_PAGE_SIZE - 1)) / SAVE_DATA_PAGE_SIZE;
    VirtualAlloc(s_saveData, pages * SAVE_DATA_PAGE_SIZE, 0x1000, 4);
    s_saveDataSize = pages;
}

void ConsoleSaveFileOriginal::ReAlloc(unsigned int size)
{
    unsigned int currentSize = s_saveDataSize * SAVE_DATA_PAGE_SIZE;
    if (size > currentSize)
    {
        unsigned int pages = (size + (SAVE_DATA_PAGE_SIZE - 1)) / SAVE_DATA_PAGE_SIZE;
        void* result = VirtualAlloc(s_saveData, pages * SAVE_DATA_PAGE_SIZE, 0x1000, 4);
        if (result == NULL)
            __debugbreak();
        s_saveDataSize = pages;
    }
}

FileEntry* ConsoleSaveFileOriginal::createFile(const ConsoleSavePath& path)
{
    LockSaveAccess();
    FileEntry* entry = mFileHeader.AddFile(path.getPath(), 0);
    ReAlloc(mFileHeader.GetFileSize());
    ReleaseSaveAccess();
    return entry;
}

void ConsoleSaveFileOriginal::deleteFile(FileEntry* entry)
{
    _deleteFile(entry);
}

void ConsoleSaveFileOriginal::setFilePointer(FileEntry* entry, long distanceToMove, int*, unsigned int moveMethod)
{
    LockSaveAccess();
    entry->m_filePointer = entry->m_data.m_startOffset + distanceToMove;
    if (moveMethod == 2)
        entry->m_filePointer += entry->m_data.m_fileSize;
    ReleaseSaveAccess();
}

bool ConsoleSaveFileOriginal::writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite,
                                        unsigned int* bytesWritten)
{
    return _writeFile(entry, buffer, bytesToWrite, bytesWritten);
}

bool ConsoleSaveFileOriginal::zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten)
{
    return _zeroFile(entry, bytesToZero, bytesWritten);
}

bool ConsoleSaveFileOriginal::readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead,
                                       unsigned int* bytesRead)
{
    return _readFile(entry, buffer, bytesToRead, bytesRead);
}

void ConsoleSaveFileOriginal::Flush(bool, bool)
{
    LockSaveAccess();
    finalizeWrite();
    _LARGE_INTEGER frequency;
    _LARGE_INTEGER start;
    _LARGE_INTEGER end;
    QueryPerformanceFrequency(&frequency);
    unsigned int fileSize = mFileHeader.GetFileSize();
    unsigned int compressedSize = fileSize + 8;
    unsigned char* compressedData = static_cast<unsigned char*>(gStorage->AllocateSaveData(compressedSize));

    if (compressedData == NULL)
    {
        compressedSize = 0;
        PIXBeginNamedEvent(0.0f, "Pre-calc save compression");
        QueryPerformanceCounter(&start);
        Compression::getCompression()->Compress(NULL, &compressedSize, mData, fileSize);
        QueryPerformanceCounter(&end);
        PIXEndNamedEvent();
        compressedSize += 8;
        compressedData = static_cast<unsigned char*>(gStorage->AllocateSaveData(compressedSize));
    }

    if (compressedData != NULL)
    {
        PIXBeginNamedEvent(0.0f, "Actual save compression");
        QueryPerformanceCounter(&start);
        Compression::getCompression()->Compress(compressedData + 8, &compressedSize, mData, fileSize);
        QueryPerformanceCounter(&end);
        PIXEndNamedEvent();

        std::memset(compressedData, 0, 8);
        unsigned int zero = 0;
        std::memcpy(compressedData, &zero, sizeof(zero));
        std::memcpy(compressedData + 4, &fileSize, sizeof(fileSize));

        unsigned char* image = NULL;
        unsigned int imageSize = 0;
        gApp.GetSaveThumbnail(&image, &imageSize);
        unsigned char thumbnail[0x200];
        unsigned int thumbnailSize = fjGetSaveThumbnailData(thumbnail, sizeof(thumbnail));
        int saveUniqueNumber = 0;
        gStorage->GetSaveUniqueNumber(&saveUniqueNumber);
        telemetryManagerInstance->RecordLevelSaveOrCheckpoint(gProfile->GetPrimaryPad(), saveUniqueNumber,
                                                              compressedSize + 8);
        gStorage->SetSaveImages(image, imageSize, NULL, 0, thumbnail, thumbnailSize);
        delete[] image;
        gStorage->SetSaveDataSize(compressedSize + 8);
        gStorage->SaveSaveData(SaveSaveDataCallback, this);
    }
    ReleaseSaveAccess();
}

std::vector<FileEntry*>* ConsoleSaveFileOriginal::getRegionFilesByDimension(unsigned int)
{
    return NULL;
}

unsigned char* ConsoleSaveFileOriginal::getWritePointer(FileEntry* entry)
{
    return static_cast<unsigned char*>(mData) + entry->m_filePointer;
}
