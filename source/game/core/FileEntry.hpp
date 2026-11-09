#ifndef GAME_CORE_FILEENTRY_HPP
#define GAME_CORE_FILEENTRY_HPP

#include <string.h>

#include <string>

struct FileEntrySaveData
{
    wchar_t m_name[64];
    unsigned int m_fileSize;
    unsigned int m_startOffset;
    long long m_timestamp;
};

class FileEntry
{
public:
    FileEntry() { memset(&m_data, 0, sizeof(m_data)); }

    FileEntry(wchar_t* name, unsigned int fileSize, unsigned int startOffset);

    static bool newestFirst(FileEntry* first, FileEntry* second)
    {
        return first->m_data.m_timestamp > second->m_data.m_timestamp;
    }

    FileEntrySaveData m_data;
    std::wstring m_fileName;
    unsigned int m_filePointer;
    bool m_isNewFile;
};

#endif
