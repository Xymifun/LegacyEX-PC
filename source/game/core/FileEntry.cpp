#include "game/core/FileEntry.hpp"
#include "game/core/StringConversion.hpp"

#include <string.h>

FileEntry::FileEntry(wchar_t* name, unsigned int fileSize, unsigned int startOffset)
{
    m_data.m_fileSize = fileSize;
    m_data.m_startOffset = startOffset;
    memset(m_data.m_name, 0, sizeof(m_data.m_name));
    memcpy(m_data.m_name, name, sizeof(m_data.m_name));
    m_fileName = char16towstring(name);
    m_data.m_timestamp = 0;
    m_filePointer = m_data.m_startOffset;
    m_isNewFile = true;
}
