#include "game/core/FileOutputStream.hpp"

#include "game/core/File.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);

FileOutputStream::FileOutputStream(const File& file)
{
    mFile = INVALID_HANDLE_VALUE;

    if (!file.exists() || !file.isDirectory())
    {
        char buffer[256];
        mFile = CreateFileA(wstringtofilename(file.getPath(), buffer), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, 0);
    }
}

FileOutputStream::~FileOutputStream()
{
    if (mFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mFile);
    }
}

void FileOutputStream::write(unsigned int b)
{
    unsigned char value = (unsigned char)b;
    unsigned int bytesWritten;

    WriteFile(mFile, &value, 1, &bytesWritten, NULL);
}

void FileOutputStream::write(arrayWithLength<unsigned char> data)
{
    unsigned int bytesWritten;

    WriteFile(mFile, data.data, data.length, &bytesWritten, NULL);
}

void FileOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int bytesWritten;
    unsigned char* buffer = data.data + offset;

    WriteFile(mFile, buffer, length, &bytesWritten, NULL);
}

void FileOutputStream::close()
{
    CloseHandle(mFile);
    mFile = INVALID_HANDLE_VALUE;
}

void FileOutputStream::flush()
{
}
