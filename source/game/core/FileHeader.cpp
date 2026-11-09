#include "game/core/FileHeader.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"

#include <string.h>
#include <wchar.h>

#include <algorithm>

wchar_t* wstringtochar16(const std::wstring& string);

wchar_t* wstringtochar16(const std::wstring& string)
{
    return const_cast<wchar_t*>(string.c_str());
}

static const unsigned int SAVE_HEADER_SIZE = 12;
static const unsigned int FILE_ENTRY_DATA_SIZE = 144;
static const unsigned int FILE_ENTRY_DATA_SIZE_V1 = 136;
static const short CURRENT_SAVE_VERSION = 11;
static const short MAX_SUPPORTED_SAVE_VERSION = 12;

FileHeader::FileHeader()
{
    m_currentFile = NULL;
    m_saveVersion = 0;
    m_originalSaveVersion = CURRENT_SAVE_VERSION;
    m_savePlatform = kLocalSavePlatform;
    m_saveEndian = kLocalByteOrder;
}

FileHeader::~FileHeader()
{
    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        delete m_fileEntries[i];
    }
}

void FileHeader::Reset()
{
    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        delete m_fileEntries[i];
    }

    m_fileEntries.clear();

    m_currentFile = NULL;
    m_saveVersion = 0;
    m_originalSaveVersion = CURRENT_SAVE_VERSION;
    m_savePlatform = kLocalSavePlatform;
    m_saveEndian = kLocalByteOrder;
}

unsigned int FileHeader::GetStartOfNextData()
{
    unsigned int startOfNextData = SAVE_HEADER_SIZE;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i]->m_data.m_fileSize != 0)
            startOfNextData += m_fileEntries[i]->m_data.m_fileSize;
    }

    return startOfNextData;
}

FileEntry* FileHeader::AddFile(const std::wstring& fileName, unsigned int fileSize)
{
    wchar_t name[64];

    memset(name, 0, sizeof(name));

    memcpy(name, wstringtochar16(fileName), std::min(sizeof(name), sizeof(wchar_t) * fileName.length()));

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i]->m_fileName.compare(fileName) == 0)
            return m_fileEntries[i];
    }

    m_fileEntries.push_back(new FileEntry(name, fileSize, GetStartOfNextData()));
    m_currentFile = m_fileEntries[m_fileEntries.size() - 1];

    return m_currentFile;
}

void FileHeader::AdjustStartOffsets(FileEntry* entry, unsigned int size, bool bRemove)
{
    bool bFound = false;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (bFound)
        {
            if (bRemove)
            {
                m_fileEntries[i]->m_data.m_startOffset -= size;
                m_fileEntries[i]->m_filePointer -= size;
            }
            else
            {
                m_fileEntries[i]->m_data.m_startOffset += size;
                m_fileEntries[i]->m_filePointer += size;
            }
        }
        else if (m_fileEntries[i] == entry)
        {
            bFound = true;
        }
    }
}

void FileHeader::RemoveFile(FileEntry* entry)
{
    if (entry == NULL)
        return;

    AdjustStartOffsets(entry, entry->m_data.m_fileSize, true);

    std::vector<FileEntry*>::iterator it = std::find(m_fileEntries.begin(), m_fileEntries.end(), entry);
    if (it < m_fileEntries.end())
        m_fileEntries.erase(it);

    delete entry;
}

void FileHeader::WriteHeader(void* pData)
{
    unsigned int indexOffset = GetStartOfNextData();
    unsigned int fileCount = m_fileEntries.size();
    short originalSaveVersion = m_originalSaveVersion;
    short saveVersion = CURRENT_SAVE_VERSION;

    unsigned int* pOffsets = (unsigned int*)pData;
    short* pVersions = (short*)(pOffsets + 2);

    pOffsets[0] = indexOffset;
    pOffsets[1] = fileCount;
    pVersions[0] = originalSaveVersion;
    pVersions[1] = saveVersion;

    unsigned char* pEntryData = (unsigned char*)pData + indexOffset;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        memcpy(pEntryData, &m_fileEntries[i]->m_data, FILE_ENTRY_DATA_SIZE);
        pEntryData += FILE_ENTRY_DATA_SIZE;
    }
}

void FileHeader::ReadHeader(void* pData, ESavePlatform platform)
{
    m_savePlatform = platform;

    switch (m_savePlatform)
    {
    case eSavePlatform_X360:
    case eSavePlatform_WIIU:
    case eSavePlatform_PS3:
        m_saveEndian = eByteOrder_BigEndian;
        break;

    case eSavePlatform_NX:
    case eSavePlatform_XB1:
    case eSavePlatform_PS4:
    case eSavePlatform_WIN:
    case eSavePlatform_PSV:
        m_saveEndian = eByteOrder_LittleEndian;
        break;

    default:
        m_savePlatform = kLocalSavePlatform;
        m_saveEndian = kLocalByteOrder;
        break;
    }

    unsigned int indexOffset = *(unsigned int*)pData;
    if (m_saveEndian != kLocalByteOrder)
        System::ReverseULONG(&indexOffset);

    unsigned int fileCount = *(unsigned int*)((char*)pData + sizeof(indexOffset));
    if (m_saveEndian != kLocalByteOrder)
        System::ReverseULONG(&fileCount);

    char* pVersions = (char*)pData + sizeof(indexOffset) + sizeof(fileCount);
    m_originalSaveVersion = *(short*)pVersions;
    if (m_saveEndian != kLocalByteOrder)
        System::ReverseSHORT(&m_originalSaveVersion);

    m_saveVersion = *(short*)(pVersions + sizeof(m_originalSaveVersion));
    if (m_saveEndian != kLocalByteOrder)
        System::ReverseSHORT(&m_saveVersion);

    unsigned char* pEntryData = (unsigned char*)pData + indexOffset;

    switch (m_saveVersion)
    {
    case 1:
        for (unsigned int readOffset = 0; readOffset < fileCount; readOffset += FILE_ENTRY_DATA_SIZE_V1)
        {
            FileEntry* pEntry = new FileEntry();

            memcpy(&pEntry->m_data, pEntryData, FILE_ENTRY_DATA_SIZE_V1);

            pEntry->m_filePointer = pEntry->m_data.m_startOffset;
            m_currentFile = pEntry;
            m_fileEntries.push_back(pEntry);

            pEntryData += FILE_ENTRY_DATA_SIZE_V1;
        }
        break;

    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case CURRENT_SAVE_VERSION:
    case MAX_SUPPORTED_SAVE_VERSION:
        for (unsigned int i = 0; i < fileCount; i++)
        {
            FileEntry* pEntry = new FileEntry();

            memcpy(&pEntry->m_data, pEntryData, FILE_ENTRY_DATA_SIZE);

            if (m_saveEndian != kLocalByteOrder)
            {
                System::ReverseCHAR16A(pEntry->m_data.m_name, 64);
                System::ReverseULONG(&pEntry->m_data.m_fileSize);
                System::ReverseULONG(&pEntry->m_data.m_startOffset);
                System::ReverseULONGLONG(&pEntry->m_data.m_timestamp);
            }

            pEntry->m_fileName = char16towstring(pEntry->m_data.m_name);

            pEntry->m_filePointer = pEntry->m_data.m_startOffset;
            m_currentFile = pEntry;
            m_fileEntries.push_back(pEntry);

            pEntryData += FILE_ENTRY_DATA_SIZE;
        }
        break;
    }
}

unsigned int FileHeader::GetFileSize()
{
    return GetStartOfNextData() + m_fileEntries.size() * FILE_ENTRY_DATA_SIZE;
}

bool FileHeader::fileExists(const std::wstring& fileName)
{
    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (m_fileEntries[i]->m_fileName.compare(fileName) == 0)
            return true;
    }

    return false;
}

std::vector<FileEntry*>* FileHeader::getFilesWithPrefix(const std::wstring& prefix)
{
    std::vector<FileEntry*>* pFiles = NULL;

    for (unsigned int i = 0; i < m_fileEntries.size(); i++)
    {
        if (wcsncmp(m_fileEntries[i]->m_fileName.c_str(), prefix.c_str(), prefix.length()) == 0)
        {
            if (pFiles == NULL)
                pFiles = new std::vector<FileEntry*>();

            pFiles->push_back(m_fileEntries[i]);
        }
    }

    return pFiles;
}

ByteOrder FileHeader::getEndian(ESavePlatform platform)
{
    switch (platform)
    {
    case eSavePlatform_X360:
    case eSavePlatform_WIIU:
    case eSavePlatform_PS3:
        return eByteOrder_BigEndian;

    default:
        return eByteOrder_LittleEndian;
    }
}
