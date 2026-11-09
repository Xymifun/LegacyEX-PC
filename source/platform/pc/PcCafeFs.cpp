#include "platform/pc/PcPaths.hpp"

#include "game/core/CriticalSection.hpp"
#include "platform/pc/PcOs.hpp"

#include <coreinit/filesystem.h>
#include <dirent.h>
#include <errno.h>
#include <nn/save/save.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include <vector>

namespace
{
const long long SECONDS_FROM1970_TO2000 = 946684800LL;

struct PcDirectory
{
    std::string path;
    std::vector<std::string> names;
    size_t next;
};

OSFastMutex handleSection;

std::vector<FILE*>& Files()
{
    static std::vector<FILE*>* s_files = 0;
    if (s_files == 0)
        s_files = new std::vector<FILE*>;
    return *s_files;
}

std::vector<PcDirectory*>& Directories()
{
    static std::vector<PcDirectory*>* s_directories = 0;
    if (s_directories == 0)
        s_directories = new std::vector<PcDirectory*>;
    return *s_directories;
}

std::string SaveSlotPath(uint8_t slotNo, const char* path)
{
    char account[16];
    if (slotNo == 0xFF)
        strcpy(account, "common");
    else
        snprintf(account, sizeof(account), "%08x", 0x80000000u | slotNo);
    std::string res = PcSaveRoot() + "/" + account;
    if (path != 0 && path[0] != '\0')
    {
        if (path[0] != '/')
            res += "/";
        res += path;
    }
    return res;
}

std::string SystemPath(const char* path)
{
    return PcResolvePath(path);
}

void MakeDirectories(const std::string& path)
{
    for (size_t slash = path.find('/', 1); slash != std::string::npos; slash = path.find('/', slash + 1))
        PcOsMakeDirectory(path.substr(0, slash).c_str());
    PcOsMakeDirectory(path.c_str());
}

FSStatus StatusFromErrno(int err)
{
    switch (err)
    {
    case ENOENT:
        return FS_STATUS_NOT_FOUND;
    case EEXIST:
        return FS_STATUS_EXISTS;
    case EISDIR:
        return FS_STATUS_NOT_FILE;
    case ENOTDIR:
        return FS_STATUS_NOT_DIR;
    case EACCES:
    case EPERM:
        return FS_STATUS_PERMISSION_ERROR;
    case ENOSPC:
        return FS_STATUS_STORAGE_FULL;
    default:
        return FS_STATUS_MEDIA_ERROR;
    }
}

FSTime ToFsTime(time_t secs)
{
    return static_cast<FSTime>(((long long)secs - SECONDS_FROM1970_TO2000) * 1000000LL);
}

void FillStat(const struct stat& info, FSStat* stat)
{
    memset(stat, 0, sizeof(*stat));
    bool dir = (info.st_mode & S_IFMT) == S_IFDIR;
    stat->flags = dir ? FS_STAT_DIRECTORY : FS_STAT_FILE;
    stat->mode = static_cast<FSMode>(FS_MODE_READ_OWNER | FS_MODE_WRITE_OWNER);
    stat->size = dir ? 0 : (uint32_t)info.st_size;
    stat->allocSize = stat->size;
    stat->created = ToFsTime(info.st_ctime);
    stat->modified = ToFsTime(info.st_mtime);
}

FSFileHandle AddFile(FILE* fp)
{
    EnterCriticalSection(&handleSection);
    size_t i = 0;
    while (i < Files().size() && Files()[i] != 0)
        ++i;
    if (i == Files().size())
        Files().push_back(fp);
    else
        Files()[i] = fp;
    LeaveCriticalSection(&handleSection);
    return static_cast<FSFileHandle>(i + 1);
}

FILE* GetFile(FSFileHandle handle)
{
    EnterCriticalSection(&handleSection);
    FILE* fp = handle != 0 && handle <= Files().size() ? Files()[handle - 1] : 0;
    LeaveCriticalSection(&handleSection);
    return fp;
}

FSStatus OpenFile(const std::string& path, const char* mode, FSFileHandle* handle)
{
    std::string systemMode(mode);
    if (systemMode.find('b') == std::string::npos)
        systemMode += "b";
    if (mode[0] == 'w' || mode[0] == 'a')
    {
        size_t slash = path.rfind('/');
        if (slash != std::string::npos)
            MakeDirectories(path.substr(0, slash));
    }
    FILE* fp = fopen(path.c_str(), systemMode.c_str());
    if (fp == 0)
        return StatusFromErrno(errno);
    *handle = AddFile(fp);
    return FS_STATUS_OK;
}

FSStatus OpenDirectory(const std::string& path, FSDirectoryHandle* handle)
{
    DIR* dir = opendir(path.c_str());
    if (dir == 0)
        return StatusFromErrno(errno);
    PcDirectory* listing = new PcDirectory;
    listing->path = path;
    listing->next = 0;
    struct dirent* e;
    while ((e = readdir(dir)) != 0)
    {
        if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
            listing->names.push_back(e->d_name);
    }
    closedir(dir);
    EnterCriticalSection(&handleSection);
    size_t i = 0;
    while (i < Directories().size() && Directories()[i] != 0)
        ++i;
    if (i == Directories().size())
        Directories().push_back(listing);
    else
        Directories()[i] = listing;
    LeaveCriticalSection(&handleSection);
    *handle = static_cast<FSDirectoryHandle>(i + 1);
    return FS_STATUS_OK;
}

FSStatus StatPath(const std::string& path, FSStat* stat)
{
    struct stat info;
    if (::stat(path.c_str(), &info) != 0)
        return StatusFromErrno(errno);
    FillStat(info, stat);
    return FS_STATUS_OK;
}

FSStatus RemovePath(const std::string& path)
{
    if (::remove(path.c_str()) == 0)
        return FS_STATUS_OK;
    return StatusFromErrno(errno);
}
}

void FSInit()
{
}

void FSShutdown()
{
}

FSStatus FSAddClient(FSClient*, FSErrorFlag)
{
    return FS_STATUS_OK;
}

FSStatus FSDelClient(FSClient*, FSErrorFlag)
{
    return FS_STATUS_OK;
}

void FSInitCmdBlock(FSCmdBlock*)
{
}

void FSSetStateChangeNotification(FSClient*, FSStateChangeParams*)
{
}

FSError FSGetLastErrorCodeForViewer(FSClient*)
{
    return FS_ERROR_OK;
}

FSStatus FSOpenFile(FSClient*, FSCmdBlock*, const char* path, const char* mode, FSFileHandle* handle, FSErrorFlag)
{
    bool reading = mode[0] == 'r' && strchr(mode, '+') == 0;
    return OpenFile(reading ? SystemPath(path) : PcResolveWritePath(path), mode, handle);
}

FSStatus FSCloseFile(FSClient*, FSCmdBlock*, FSFileHandle handle, FSErrorFlag)
{
    EnterCriticalSection(&handleSection);
    FILE* fp = 0;
    if (handle != 0 && handle <= Files().size())
    {
        fp = Files()[handle - 1];
        Files()[handle - 1] = 0;
    }
    LeaveCriticalSection(&handleSection);
    if (fp == 0)
        return FS_STATUS_FATAL_ERROR;
    fclose(fp);
    return FS_STATUS_OK;
}

FSStatus FSReadFile(FSClient*, FSCmdBlock*, uint8_t* buf, uint32_t len, uint32_t n, FSFileHandle handle,
                    uint32_t, FSErrorFlag)
{
    FILE* fp = GetFile(handle);
    if (fp == 0 || len == 0)
        return FS_STATUS_FATAL_ERROR;
    size_t read = fread(buf, len, n, fp);
    if (read == 0 && ferror(fp))
        return FS_STATUS_MEDIA_ERROR;
    return static_cast<FSStatus>(read);
}

FSStatus FSWriteFile(FSClient*, FSCmdBlock*, uint8_t* buf, uint32_t len, uint32_t n, FSFileHandle handle,
                     uint32_t, FSErrorFlag)
{
    FILE* fp = GetFile(handle);
    if (fp == 0 || len == 0)
        return FS_STATUS_FATAL_ERROR;
    size_t written = fwrite(buf, len, n, fp);
    if (written < n)
        return FS_STATUS_STORAGE_FULL;
    return static_cast<FSStatus>(written);
}

FSStatus FSGetStatFile(FSClient*, FSCmdBlock*, FSFileHandle handle, FSStat* stat, FSErrorFlag)
{
    FILE* fp = GetFile(handle);
    if (fp == 0)
        return FS_STATUS_FATAL_ERROR;
    fflush(fp);
    struct stat info;
    if (fstat(fileno(fp), &info) != 0)
        return StatusFromErrno(errno);
    FillStat(info, stat);
    return FS_STATUS_OK;
}

FSStatus FSOpenDir(FSClient*, FSCmdBlock*, const char* path, FSDirectoryHandle* handle, FSErrorFlag)
{
    return OpenDirectory(SystemPath(path), handle);
}

FSStatus FSReadDir(FSClient*, FSCmdBlock*, FSDirectoryHandle handle, FSDirectoryEntry* e, FSErrorFlag)
{
    EnterCriticalSection(&handleSection);
    PcDirectory* listing = handle != 0 && handle <= Directories().size() ? Directories()[handle - 1] : 0;
    LeaveCriticalSection(&handleSection);
    if (listing == 0)
        return FS_STATUS_FATAL_ERROR;
    while (listing->next < listing->names.size())
    {
        const std::string& name = listing->names[listing->next++];
        memset(e, 0, sizeof(*e));
        if (StatPath(listing->path + "/" + name, &e->info) != FS_STATUS_OK)
            continue;
        strncpy(e->name, name.c_str(), sizeof(e->name) - 1);
        return FS_STATUS_OK;
    }
    return FS_STATUS_END;
}

FSStatus FSCloseDir(FSClient*, FSCmdBlock*, FSDirectoryHandle handle, FSErrorFlag)
{
    EnterCriticalSection(&handleSection);
    PcDirectory* listing = 0;
    if (handle != 0 && handle <= Directories().size())
    {
        listing = Directories()[handle - 1];
        Directories()[handle - 1] = 0;
    }
    LeaveCriticalSection(&handleSection);
    if (listing == 0)
        return FS_STATUS_FATAL_ERROR;
    delete listing;
    return FS_STATUS_OK;
}

SAVEStatus SAVEInit()
{
    return SAVE_STATUS_OK;
}

SAVEStatus SAVEInitSaveDir(uint8_t slotNo)
{
    MakeDirectories(SaveSlotPath(slotNo, 0));
    return SAVE_STATUS_OK;
}

FSStatus SAVEOpenFile(FSClient*, FSCmdBlock*, uint8_t slotNo, const char* path, const char* mode,
                      FSFileHandle* handle, FSErrorFlag)
{
    return OpenFile(SaveSlotPath(slotNo, path), mode, handle);
}

FSStatus SAVEOpenDir(FSClient*, FSCmdBlock*, uint8_t slotNo, const char* path, FSDirectoryHandle* handle, FSErrorFlag)
{
    std::string dir = SaveSlotPath(slotNo, path);
    MakeDirectories(dir);
    return OpenDirectory(dir, handle);
}

FSStatus SAVERemove(FSClient*, FSCmdBlock*, uint8_t slotNo, const char* path, FSErrorFlag)
{
    return RemovePath(SaveSlotPath(slotNo, path));
}

FSStatus SAVEFlushQuota(FSClient*, FSCmdBlock*, uint8_t, FSErrorFlag)
{
    return FS_STATUS_OK;
}

FSStatus SAVERollbackQuota(FSClient*, FSCmdBlock*, uint8_t, FSErrorFlag)
{
    return FS_STATUS_OK;
}
extern "C" int PcSaveGetStat(FSClient*, FSCmdBlock*, unsigned char slotNo, const char* path, FSStat* stat,
                             FSErrorFlag) asm("SAVEGetStat");

extern "C" int PcSaveGetStat(FSClient*, FSCmdBlock*, unsigned char slotNo, const char* path, FSStat* stat, FSErrorFlag)
{
    return StatPath(SaveSlotPath(slotNo, path), stat);
}

FSStatus FSSetPosFile(FSClient*, FSCmdBlock*, FSFileHandle handle, uint32_t pos, FSErrorFlag)
{
    FILE* fp = GetFile(handle);
    if (fp == 0)
        return FS_STATUS_FATAL_ERROR;
    return fseek(fp, (long)pos, SEEK_SET) == 0 ? FS_STATUS_OK : FS_STATUS_MEDIA_ERROR;
}
