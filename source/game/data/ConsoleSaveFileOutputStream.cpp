#include "game/data/ConsoleSaveFileOutputStream.hpp"

#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include "game/core/FileEntry.hpp"

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, const ConsoleSavePath& path)
    : m_saveFile(saveFile)
{
    m_fileEntry = saveFile->createFile(path);
    m_saveFile->setFilePointer(m_fileEntry, 0, NULL, 0);
}

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile* saveFile, FileEntry* fileEntry)
    : m_saveFile(saveFile),
      m_fileEntry(fileEntry)
{
    saveFile->setFilePointer(fileEntry, 0, NULL, 0);
}

void ConsoleSaveFileOutputStream::write(unsigned int value)
{
    unsigned char byte = (unsigned char)value;
    unsigned int bytesWritten;
    m_saveFile->writeFile(m_fileEntry, &byte, 1, &bytesWritten);
}

void ConsoleSaveFileOutputStream::write(arrayWithLength<unsigned char> data)
{
    unsigned int length = data.length;
    unsigned int bytesWritten;
    m_saveFile->writeFile(m_fileEntry, &data, length, &bytesWritten);
}

void ConsoleSaveFileOutputStream::write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length)
{
    unsigned int bytesWritten;
    m_saveFile->writeFile(m_fileEntry, data.data + offset, length, &bytesWritten);
}

void ConsoleSaveFileOutputStream::close()
{
    if (m_saveFile != NULL)
    {
        m_saveFile->closeHandle(m_fileEntry);
        m_saveFile = NULL;
    }
}

void ConsoleSaveFileOutputStream::flush()
{
}
