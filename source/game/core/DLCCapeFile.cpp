#include "game/core/DLCCapeFile.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"

DLCCapeFile::DLCCapeFile(const std::wstring& path) : DLCFile(DLCManager::eDLCType_Cape, path)
{
}

void DLCCapeFile::addData(unsigned char* data, unsigned int size)
{
    gApp.AddMemoryTextureFile(m_path, data, size);
}
