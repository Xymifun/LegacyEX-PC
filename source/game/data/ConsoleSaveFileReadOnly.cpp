#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/data/ConsoleSaveFileReadOnly.hpp"

#include <algorithm>
#include <cstring>

#include "platform/wiiu/CStorage.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"

#ifdef RECON_NATIVE
#include "win32_memory.hpp"
#else
#include "platform/wiiu/WiiU_FileCache.hpp"
#endif
void __debugbreak();

static void* sSaveData = NULL;
static unsigned int sSaveDataPages = 0;

ConsoleSaveFileReadOnly::ConsoleSaveFileReadOnly(const std::wstring& filename, void* data, unsigned int dataSize,
                                                 bool resetData, ESavePlatform platform)
    : ConsoleSaveFileCommon()
{
    if (sSaveData == NULL)
        sSaveData = VirtualAlloc(NULL, 0x4000000, 0x2000, 4);
    mData = sSaveData;

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

ConsoleSaveFileReadOnly::~ConsoleSaveFileReadOnly()
{
    VirtualFree(sSaveData, 0x4000000, 0x4000);
    sSaveDataPages = 0;
}

void ConsoleSaveFileReadOnly::refreshFromLevelgen()
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && options->requiresBaseSave())
    {
        unsigned int saveDataSize;
        void* saveData = options->getBaseSaveData(saveDataSize);
        if (saveData != NULL && saveDataSize != 0)
        {
            VirtualFree(sSaveData, 0x4000000, 0x4000);
            sSaveDataPages = 0;
            mFileHeader.Reset();
#ifdef MINECRAFT_PC
            _initSaveFromData(mFilename, saveDataSize, saveData, PcBaseSavePlatform(saveData, saveDataSize));
#else
            _initSaveFromData(mFilename, saveDataSize, saveData, eSavePlatform_WIIU);
#endif
            options->deleteBaseSaveData();
            ConvertToLocalPlatform();
        }
    }
}

void ConsoleSaveFileReadOnly::AllocInitialQuota(unsigned int size)
{
    unsigned int initialSize = size;
    if (initialSize < SAVE_DATA_MIN_QUOTA)
        initialSize = SAVE_DATA_MIN_QUOTA;

    unsigned int pages = (initialSize + (SAVE_DATA_PAGE_SIZE - 1)) / SAVE_DATA_PAGE_SIZE;
    VirtualAlloc(sSaveData, pages * SAVE_DATA_PAGE_SIZE, 0x1000, 4);
    sSaveDataPages = pages;
}

void ConsoleSaveFileReadOnly::ReAlloc(unsigned int size)
{
    unsigned int currentSize = sSaveDataPages * SAVE_DATA_PAGE_SIZE;
    if (size > currentSize)
    {
        unsigned int pages = (size + (SAVE_DATA_PAGE_SIZE - 1)) / SAVE_DATA_PAGE_SIZE;
        void* result = VirtualAlloc(sSaveData, pages * SAVE_DATA_PAGE_SIZE, 0x1000, 4);
        if (result == NULL)
            __debugbreak();
        sSaveDataPages = pages;
    }
}

FileEntry* ConsoleSaveFileReadOnly::createFile(const ConsoleSavePath& path)
{
    LockSaveAccess();
    FileEntry* entry = mFileHeader.AddFile(path.getPath(), 0);
    ReleaseSaveAccess();
    return entry;
}

void ConsoleSaveFileReadOnly::deleteFile(FileEntry* entry)
{
    _deleteFile(entry);
}

void ConsoleSaveFileReadOnly::setFilePointer(FileEntry* entry, long distanceToMove, int*, unsigned int moveMethod)
{
    LockSaveAccess();
    entry->m_filePointer = entry->m_data.m_startOffset + distanceToMove;
    if (moveMethod == 2)
        entry->m_filePointer += entry->m_data.m_fileSize;
    ReleaseSaveAccess();
}

bool ConsoleSaveFileReadOnly::writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite,
                                        unsigned int* bytesWritten)
{
    return _writeFile(entry, buffer, bytesToWrite, bytesWritten);
}

bool ConsoleSaveFileReadOnly::zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten)
{
    return _zeroFile(entry, bytesToZero, bytesWritten);
}

bool ConsoleSaveFileReadOnly::readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead,
                                       unsigned int* bytesRead)
{
    return _readFile(entry, buffer, bytesToRead, bytesRead);
}

void ConsoleSaveFileReadOnly::Flush(bool commit, bool force)
{
    (void)commit;
    (void)force;
}

std::vector<FileEntry*>* ConsoleSaveFileReadOnly::getRegionFilesByDimension(unsigned int dimension)
{
    (void)dimension;
    std::vector<FileEntry*>* regionFiles = NULL;
    return regionFiles;
}
