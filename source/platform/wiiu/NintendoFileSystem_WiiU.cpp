#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"

#include <coreinit/filesystem.h>

#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/WiiU_FileSystem.hpp"

extern void setAltMainOKToRun();
extern void waitForAltMainThreadToComplete();
extern FSCmdBlock* GetCommandBlock();

int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode, void* securityAttributes,
                unsigned int creationDisposition, unsigned int flagsAndAttributes, int templateFile)
{
    (void)shareMode;
    (void)securityAttributes;
    (void)flagsAndAttributes;
    (void)templateFile;

    setAltMainOKToRun();
    char path[256];
    getFinalFilePath(path, fileName);
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSFileHandle file = static_cast<FSFileHandle>(-1);

    switch (creationDisposition)
    {
    case OPEN_EXISTING:
        if (desiredAccess == GENERIC_READ)
        {
            FSOpenFile(gFileSystemClient, commandBlock, path, "r", &file, static_cast<FSErrorFlag>(8));
            break;
        }
    case OPEN_ALWAYS:
        FSOpenFile(gFileSystemClient, commandBlock, path, "a+", &file, static_cast<FSErrorFlag>(8));
        break;
    case CREATE_NEW:
    case CREATE_ALWAYS:
    case TRUNCATE_EXISTING:
        FSOpenFile(gFileSystemClient, commandBlock, path, "w+", &file, static_cast<FSErrorFlag>(8));
        break;
    default:
        waitForAltMainThreadToComplete();
        return static_cast<int>(file);
    }

    if (file != static_cast<FSFileHandle>(-1))
        WiiU_FileCache::CreateFileCache(static_cast<int>(file));

    waitForAltMainThreadToComplete();
    return static_cast<int>(file);
}

int CreateDirectoryA(const char* pathName, void* securityAttributes)
{
    (void)securityAttributes;

    setAltMainOKToRun();
    char path[256];
    getFinalFilePath(path, pathName);
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSStatus result = FSMakeDir(gFileSystemClient, commandBlock, path, FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return result == 0;
}

int ReadDir(int* directory, FSDirEntry* entry)
{
    setAltMainOKToRun();
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSStatus result = FSReadDir(gFileSystemClient, commandBlock, static_cast<FSDirectoryHandle>(*directory), entry,
                                FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return result == 0;
}

int CloseDir(int* directory)
{
    setAltMainOKToRun();
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSStatus result = FSCloseDir(gFileSystemClient, commandBlock, static_cast<FSDirectoryHandle>(*directory),
                                 FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return result == 0;
}

void CloseHandle(int file)
{
    WiiU_FileCache::DeleteFileCache(file);
}
