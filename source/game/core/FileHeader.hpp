#ifndef GAME_CORE_FILEHEADER_HPP
#define GAME_CORE_FILEHEADER_HPP

#include <string>
#include <vector>

#include "game/core/FileEntry.hpp"

enum ESavePlatform
{
    eSavePlatform_X360 = 0x30363358,
    eSavePlatform_WIIU = 0x55494957,
    eSavePlatform_NX = 0x584E584E,
    eSavePlatform_XB1 = 0x5F314258,
    eSavePlatform_PS3 = 0x5F335350,
    eSavePlatform_PS4 = 0x5F345350,
    eSavePlatform_WIN = 0x5F4E4957,
    eSavePlatform_PSV = 0x5F565350
};

enum ByteOrder
{
    eByteOrder_BigEndian = 0,
    eByteOrder_LittleEndian = 1
};

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
const ESavePlatform kLocalSavePlatform = eSavePlatform_WIIU;
const ByteOrder kLocalByteOrder = eByteOrder_BigEndian;
#else
const ESavePlatform kLocalSavePlatform = eSavePlatform_WIN;
const ByteOrder kLocalByteOrder = eByteOrder_LittleEndian;
#endif

class FileHeader
{
public:
    FileHeader();
    ~FileHeader();

    void Reset();

    unsigned int GetStartOfNextData();
    FileEntry* AddFile(const std::wstring& fileName, unsigned int fileSize);
    void AdjustStartOffsets(FileEntry* entry, unsigned int size, bool bRemove);
    void RemoveFile(FileEntry* entry);

    void WriteHeader(void* pData);
    void ReadHeader(void* pData, ESavePlatform platform);

    unsigned int GetFileSize();

    bool fileExists(const std::wstring& fileName);
    std::vector<FileEntry*>* getFilesWithPrefix(const std::wstring& prefix);

    static ByteOrder getEndian(ESavePlatform platform);

    std::vector<FileEntry*> m_fileEntries;
    ESavePlatform m_savePlatform;
    ByteOrder m_saveEndian;
    short m_saveVersion;
    short m_originalSaveVersion;
    FileEntry* m_currentFile;
};

#endif
