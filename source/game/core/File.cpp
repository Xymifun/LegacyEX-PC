#include "game/core/File.hpp"
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#include <stdio.h>

struct _WIN32_FIND_DATAA
{
    unsigned int dwFileAttributes;
    _FILETIME ftCreationTime;
    _FILETIME ftLastAccessTime;
    _FILETIME ftLastWriteTime;
    unsigned int nFileSizeHigh;
    unsigned int nFileSizeLow;
    unsigned int dwReserved0;
    unsigned int dwReserved1;
    char cFileName[260];
    char cAlternateFileName[14];
};

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);
std::wstring filenametowstring(const char* filename);

char* wstringtofilename(const std::wstring& path, char (&filename)[256])
{
    std::wstring::size_type length = path.length();

    for (std::wstring::size_type i = 0; i < length; ++i)
    {
        wchar_t character = path[i];

        if (character == L'\\')
        {
            character = L'/';
        }

        filename[i] = character;
    }
    filename[path.length()] = '\0';
    return filename;
}

std::wstring filenametowstring(const char* filename)
{
    return convStringToWstring(std::string(filename));
}

int DeleteFileA(const char* fileName);
int FindFirstFileA(const char* fileName, _WIN32_FIND_DATAA* findData);
int FindNextFileA(int findHandle, _WIN32_FIND_DATAA* findData);
int MoveFileA(const char* existingFileName, const char* newFileName);

File::File()
{
    mPath = L"";
}

File::File(const std::wstring& path)
{
    if (path.length() == 0)
    {
        mPath = std::wstring(L"");
    }
    else
    {
        mPath = path;
    }
}

File::File(const std::wstring& parent, const std::wstring& child)
{
    mPath = parent + L'\\' + child;
}

File::File(const File& parent, const std::wstring& child)
{
    mPath = parent.getPath() + L'\\' + child;
}

std::wstring File::getPath() const
{
    return mPath;
}

std::wstring File::getName() const
{
    std::wstring::size_type separator = mPath.rfind(L'\\');

    if (separator == std::wstring::npos)
    {
        separator = mPath.rfind(L'/');
    }

    return mPath.substr(separator + 1, mPath.length());
}

std::wstring File::ext() const
{
    std::wstring name = getName();
    std::wstring::size_type dot = name.rfind(std::wstring(L"."));

    if (dot == std::wstring::npos || dot == 0)
    {
        return L"";
    }

    return name.substr(dot);
}

bool File::exists() const
{
    char filename[256];

    return GetFileAttributesA(wstringtofilename(getPath(), filename)) != INVALID_FILE_ATTRIBUTES;
}

bool File::isDirectory() const
{
    char filename[256];

    return exists() && (GetFileAttributesA(wstringtofilename(getPath(), filename)) & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool File::isFile() const
{
    return exists() && !isDirectory();
}

bool File::mkdir() const
{
    char filename[256];

    return CreateDirectoryA(wstringtofilename(getPath(), filename), NULL) != 0;
}

bool File::_delete()
{
    char filename[256];

    if (DeleteFileA(wstringtofilename(getPath(), filename)) == 0)
    {
        GetLastError();
        return false;
    }

    return true;
}

long long File::length()
{
    char filename[256];

    int file = CreateFileA(wstringtofilename(getPath(), filename), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, 0);

    if (file == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    int size = GetFileSize(file, NULL);
    CloseHandle(file);

    return size;
}

std::vector<File*>* File::listFiles() const
{
    std::vector<File*>* files = new std::vector<File*>;

    if (isDirectory())
    {
        char filename[256];
        char searchPath[260];
        _WIN32_FIND_DATAA findData;

        sprintf(searchPath, "%s\\*", wstringtofilename(getPath(), filename));

        int findHandle = FindFirstFileA(searchPath, &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                files->push_back(new File(*this, filenametowstring(findData.cFileName)));
            } while (FindNextFileA(findHandle, &findData) != 0);

            CloseHandle(findHandle);
        }
    }

    return files;
}

std::vector<File*>* File::listFiles(FileFilter* filter) const
{
    if (!isDirectory())
    {
        return NULL;
    }

    std::vector<File*>* files = new std::vector<File*>;

    char filename[256];
    char searchPath[260];
    _WIN32_FIND_DATAA findData;

    sprintf(searchPath, "%s\\*", wstringtofilename(getPath(), filename));

    int findHandle = FindFirstFileA(searchPath, &findData);

    if (findHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            File entry(*this, filenametowstring(findData.cFileName));

            if (filter->accept(entry))
            {
                File copy(entry);

                files->push_back(&copy);
            }
        } while (FindNextFileA(findHandle, &findData) != 0);

        CloseHandle(findHandle);
    }

    return files;
}

int File::hash_fnct(const File& file)
{
    const wchar_t* path = file.mPath.c_str();
    unsigned int length = file.mPath.length();
    int hash = 0;

    for (unsigned int i = 0; i < length; ++i)
    {
        hash += (hash * 33 + path[i]) % 149;
    }

    return hash;
}

bool File::eq_test(const File& first, const File& second)
{
    return first.getPath() == second.getPath();
}

bool File::renameTo(File destination)
{
    char sourceName[256];
    char destinationName[256];
    return MoveFileA(wstringtofilename(getPath(), sourceName),
                     wstringtofilename(destination.getPath(), destinationName))
           != 0;
}

long long File::lastModified()
{
    char filename[256];
    char searchPath[256];
    _WIN32_FIND_DATAA findData;
    int findHandle;

    wstringtofilename(getPath(), filename);
    sprintf(searchPath, "%s", filename);
    findHandle = FindFirstFileA(searchPath, &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    CloseHandle(findHandle);
    return (static_cast<long long>(findData.ftLastWriteTime.dwHighDateTime) << 32)
           | findData.ftLastWriteTime.dwLowDateTime;
}

std::wstring File::getBaseName() const
{
    std::wstring name = getName();
    std::wstring::size_type dot = name.rfind(L'.');
    if (dot == std::wstring::npos || dot == 0)
    {
        return name;
    }
    return name.substr(0, dot);
}

File File::parentDir()
{
    std::wstring::size_type separator = mPath.rfind(L'\\');
    if (separator == std::wstring::npos)
    {
        separator = mPath.rfind(L'/');
    }
    if (separator == std::wstring::npos)
    {
        return File(L"");
    }
    return File(mPath.substr(0, separator));
}

int DeleteFileA(const char*)
{
    return 0;
}

int FindFirstFileA(const char*, _WIN32_FIND_DATAA*)
{
    return 0;
}

int FindNextFileA(int, _WIN32_FIND_DATAA*)
{
    return 0;
}

int MoveFileA(const char*, const char*)
{
    return 0;
}
