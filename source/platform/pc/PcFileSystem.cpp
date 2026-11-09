#include "platform/pc/PcPaths.hpp"

#include "platform/pc/PcMods.hpp"
#include "platform/pc/PcOs.hpp"

#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/WiiUFileSystemHelpers.hpp"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>

namespace
{
bool saveRootOverridden = false;

std::string& ContentRoot()
{
    static std::string* s_contentRoot = 0;
    if (s_contentRoot == 0)
        s_contentRoot = new std::string("content");
    return *s_contentRoot;
}

std::string& SaveRoot()
{
    static std::string* s_saveRoot = 0;
    if (s_saveRoot == 0)
        s_saveRoot = new std::string("Windows64/save");
    return *s_saveRoot;
}

const unsigned int FILE_BEGIN = 0;
const unsigned int FILE_CURRENT = 1;
const unsigned int FILE_END = 2;
const unsigned int INVALID_SET_FILE_POINTER = 0xFFFFFFFF;

bool StartsWith(const std::string& s, const char* prefix)
{
    return s.compare(0, strlen(prefix), prefix) == 0;
}

#ifndef O_BINARY
const int O_BINARY = 0;
#endif

bool Exists(const std::string& path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

std::string MatchCaseInsensitively(const std::string& path)
{
    if (path.empty() || !PcOsFileSystemIsCaseSensitive() || Exists(path))
        return path;
    std::string resolved = path[0] == '/' ? "/" : "";
    size_t start = path[0] == '/' ? 1 : 0;
    while (start <= path.size())
    {
        size_t end = path.find('/', start);
        if (end == std::string::npos)
            end = path.size();
        std::string component = path.substr(start, end - start);
        start = end + 1;
        if (component.empty())
            continue;
        std::string candidate = resolved + component;
        if (!Exists(candidate))
        {
            DIR* dir = opendir(resolved.empty() ? "." : resolved.c_str());
            if (dir == 0)
                return path;
            struct dirent* e;
            while ((e = readdir(dir)) != 0)
            {
                if (strcasecmp(e->d_name, component.c_str()) == 0)
                {
                    candidate = resolved + e->d_name;
                    break;
                }
            }
            closedir(dir);
        }
        resolved = candidate;
        if (end < path.size())
            resolved += "/";
    }
    return resolved;
}

void CreateParentDirectories(const std::string& path)
{
    for (size_t slash = path.find('/', 1); slash != std::string::npos; slash = path.find('/', slash + 1))
        PcOsMakeDirectory(path.substr(0, slash).c_str());
}
}

void PcSetContentRoot(const std::string& root)
{
    ContentRoot() = root;
}

void PcSetSaveRoot(const std::string& root)
{
    SaveRoot() = root;
    saveRootOverridden = true;
}

bool PcPrepareSaveRoot()
{
    if (saveRootOverridden)
        return true;

    const char* legacy = "save";
    const std::string& root = SaveRoot();
    struct stat info;
    if (stat(root.c_str(), &info) == 0)
    {
        if (!S_ISDIR(info.st_mode))
        {
            fprintf(stderr, "[pc] save path %s is not a directory\n", root.c_str());
            return false;
        }
        if (Exists(legacy))
            fprintf(stderr, "[pc] using %s; existing save/ left untouched (destination already exists)\n",
                    root.c_str());
        return true;
    }
    if (errno != ENOENT)
    {
        fprintf(stderr, "[pc] cannot access %s: %s\n", root.c_str(), strerror(errno));
        return false;
    }

    bool migrate = stat(legacy, &info) == 0;
    if ((migrate && !S_ISDIR(info.st_mode)) || (!migrate && errno != ENOENT))
    {
        fprintf(stderr, "[pc] cannot migrate save/: %s\n", migrate ? "not a directory" : strerror(errno));
        return false;
    }
    if (!PcOsMakeDirectory("Windows64"))
    {
        fprintf(stderr, "[pc] cannot create Windows64: %s\n", strerror(errno));
        return false;
    }
    if (migrate ? rename(legacy, root.c_str()) != 0 : !PcOsMakeDirectory(root.c_str()))
    {
        fprintf(stderr, "[pc] cannot %s %s: %s\n", migrate ? "move save/ to" : "create",
                root.c_str(), strerror(errno));
        return false;
    }
    if (migrate)
        printf("[pc] moved save/ to %s\n", root.c_str());
    return true;
}

const std::string& PcContentRoot()
{
    return ContentRoot();
}

const std::string& PcSaveRoot()
{
    return SaveRoot();
}

extern C4JStorage* gStorage;

namespace
{
std::string ResolveGamePath(const char* gamePath, std::string* contentRelative)
{
    std::string path(gamePath);
    if (gStorage != 0)
    {
        std::string mounted = gStorage->GetMountedPath(path);
        if (!mounted.empty())
            path = mounted;
    }
    for (size_t i = 0; i < path.size(); ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }
    if (StartsWith(path, "/vol/content/"))
    {
        *contentRelative = path.substr(13);
        path = ContentRoot() + "/" + *contentRelative;
    }
    else if (StartsWith(path, "/vol/save/"))
        path = SaveRoot() + "/" + path.substr(10);
    else if (StartsWith(path, "/vol/launcher/"))
        path = path.substr(14);
    else if (path.empty() || path[0] != '/')
    {
        *contentRelative = path;
        path = ContentRoot() + "/" + path;
    }
    return path;
}
}

std::string PcResolvePath(const char* gamePath)
{
    std::string relative;
    std::string path = ResolveGamePath(gamePath, &relative);
    if (!relative.empty())
    {
        std::string modded = PcModsFindContent(relative);
        if (!modded.empty())
            return modded;
    }
    return MatchCaseInsensitively(path);
}

std::string PcResolveWritePath(const char* gamePath)
{
    std::string relative;
    return MatchCaseInsensitively(ResolveGamePath(gamePath, &relative));
}

std::string PcContentRelativePath(const char* gamePath)
{
    std::string relative;
    ResolveGamePath(gamePath, &relative);
    return relative;
}

int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int, void*, unsigned int creationDisposition,
                unsigned int, int)
{
    std::string path = (desiredAccess & GENERIC_WRITE) != 0 || creationDisposition != OPEN_EXISTING
                           ? PcResolveWritePath(fileName)
                           : PcResolvePath(fileName);
    int flags = O_BINARY | ((desiredAccess & GENERIC_WRITE) != 0 ? O_RDWR : O_RDONLY);
    switch (creationDisposition)
    {
    case CREATE_NEW:
        flags |= O_CREAT | O_EXCL;
        break;
    case CREATE_ALWAYS:
        flags |= O_CREAT | O_TRUNC;
        break;
    case OPEN_ALWAYS:
        flags |= O_CREAT;
        break;
    case TRUNCATE_EXISTING:
        flags |= O_TRUNC;
        break;
    case OPEN_EXISTING:
        break;
    default:
        return INVALID_HANDLE_VALUE;
    }
    if ((flags & O_CREAT) != 0)
        CreateParentDirectories(path);
    int fp = open(path.c_str(), flags, 0644);
    return fp >= 0 ? fp : INVALID_HANDLE_VALUE;
}

int CreateDirectoryA(const char* pathName, void*)
{
    std::string path = PcResolveWritePath(pathName);
    CreateParentDirectories(path);
    return PcOsMakeDirectory(path.c_str()) ? 1 : 0;
}

void CloseHandle(int fp)
{
    if (fp >= 0)
        close(fp);
}

unsigned int GetFileSize(int fp, unsigned int* fileSizeHigh)
{
    struct stat info;
    if (fstat(fp, &info) != 0)
        return 0xFFFFFFFF;
    unsigned long long len = (unsigned long long)info.st_size;
    if (fileSizeHigh != 0)
        *fileSizeHigh = (unsigned int)(len >> 32);
    return (unsigned int)len;
}

int ReadFile(int fp, void* buf, unsigned int bytesToRead, unsigned int* bytesRead, _OVERLAPPED*)
{
    unsigned int total = 0;
    while (total < bytesToRead)
    {
        ssize_t n = read(fp, static_cast<char*>(buf) + total, bytesToRead - total);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            break;
        total += (unsigned int)n;
    }
    if (bytesRead != 0)
        *bytesRead = total;
    return total == bytesToRead || total > 0 || bytesToRead == 0 ? 1 : 0;
}

int WriteFile(int fp, const void* buf, unsigned int bytesToWrite, unsigned int* bytesWritten, _OVERLAPPED*)
{
    unsigned int total = 0;
    while (total < bytesToWrite)
    {
        ssize_t n = write(fp, static_cast<const char*>(buf) + total, bytesToWrite - total);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            break;
        total += (unsigned int)n;
    }
    if (bytesWritten != 0)
        *bytesWritten = total;
    return total == bytesToWrite ? 1 : 0;
}

int SetFilePointer(int fp, long distanceToMove, int* distanceToMoveHigh, unsigned int moveMethod)
{
    long long off = distanceToMove;
    if (distanceToMoveHigh != 0)
        off = ((long long)(*distanceToMoveHigh) << 32) | (unsigned int)distanceToMove;
    int whence = SEEK_SET;
    if (moveMethod == FILE_CURRENT)
        whence = SEEK_CUR;
    else if (moveMethod == FILE_END)
        whence = SEEK_END;
    else if (moveMethod != FILE_BEGIN)
        return (int)INVALID_SET_FILE_POINTER;
    long long pos = lseek64(fp, off, whence);
    if (pos == -1)
        return (int)INVALID_SET_FILE_POINTER;
    if (distanceToMoveHigh != 0)
        *distanceToMoveHigh = (int)(pos >> 32);
    return (int)pos;
}

unsigned int GetFileAttributesA(const char* fileName)
{
    struct stat info;
    if (stat(PcResolvePath(fileName).c_str(), &info) != 0)
        return INVALID_FILE_ATTRIBUTES;
    return (info.st_mode & S_IFMT) == S_IFDIR ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}

int OpenDir(const char* path, int* dir)
{
    return FSOpenDir(0, 0, path, reinterpret_cast<FSDirectoryHandle*>(dir), FS_ERROR_FLAG_NONE) == FS_STATUS_OK;
}

int ReadDir(int* dir, FSDirEntry* e)
{
    return FSReadDir(0, 0, static_cast<FSDirectoryHandle>(*dir), e, FS_ERROR_FLAG_NONE) == FS_STATUS_OK;
}

int CloseDir(int* dir)
{
    return FSCloseDir(0, 0, static_cast<FSDirectoryHandle>(*dir), FS_ERROR_FLAG_NONE) == FS_STATUS_OK;
}
