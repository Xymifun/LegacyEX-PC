#include "game/core/FileInputStream.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/File.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

class CConsoleMinecraftApp;

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);

FileInputStream::FileInputStream(const File& file)
{
    mEndOfFile = false;

    char buffer[256];
    char* fileName = wstringtofilename(file.getPath(), buffer);

    mFile = CreateFileA(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (mFile == INVALID_HANDLE_VALUE)
    {
        gApp.FatalLoadError(L"<FileInputStream@%d> Failed to open %ls", __LINE__, file.getPath().c_str());
    }
}

FileInputStream::~FileInputStream()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mFile);
    }
}

int FileInputStream::read()
{
    if (mEndOfFile)
    {
        return -1;
    }

    unsigned char value = 0;
    unsigned int bytesRead;

    if (ReadFile(mFile, &value, 1, &bytesRead, NULL) == 0)
    {
        gApp.FatalLoadError(L"<FileInputStream@%d> Failed to read", __LINE__);
    }
    else if (bytesRead == 0)
    {
        mEndOfFile = true;
        return -1;
    }

    return value;
}

int FileInputStream::read(arrayWithLength<unsigned char> data)
{
    if (mEndOfFile)
    {
        return -1;
    }

    unsigned int bytesRead;

    unsigned int result = ReadFile(mFile, data.data, data.length, &bytesRead, NULL);
    if (result == 0)
    {
        gApp.FatalLoadError(L"<FileInputStream@%d> Failed to read array", __LINE__);
    }
    else if (bytesRead == 0)
    {
        mEndOfFile = true;
        return -1;
    }

    return bytesRead;
}

int FileInputStream::read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    if (mEndOfFile)
    {
        return -1;
    }

    unsigned int bytesRead;

    unsigned int result = ReadFile(mFile, data.data + offset, length, &bytesRead, NULL);
    if (result == 0)
    {
        gApp.FatalLoadError(L"<FileInputStream@%d> Failed to read special", __LINE__);
    }
    else if (bytesRead == 0)
    {
        mEndOfFile = true;
        return -1;
    }

    return bytesRead;
}

void FileInputStream::close()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mFile);
        mFile = INVALID_HANDLE_VALUE;
    }
}

long long FileInputStream::skip(long long count)
{
    return 0;
}

unsigned int FileInputStream::filesize()
{
    if (mFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    return GetFileSize(mFile, NULL);
}
