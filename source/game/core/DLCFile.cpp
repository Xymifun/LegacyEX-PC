#include "game/core/DLCFile.hpp"

#include <sstream>

DLCFile::DLCFile(DLCManager::EDLCType type, const std::wstring& path) : m_pPack(NULL), m_type(type), m_path(path)
{
    if (path.substr(0, 3) == L"dlc")
    {
        std::wstring idString = path.substr(7, path.length());
        idString = idString.substr(0, idString.find(L'.'));

        std::wstringstream stream;
        stream.setf(std::ios_base::dec, std::ios_base::basefield);
        stream << idString.c_str();
        stream >> m_fileID;

        m_fileID |= 0x80000000;
    }
    else
    {
        m_fileID = 0;
    }
}

void DLCFile::addData(unsigned char* pData, unsigned int size)
{
}

unsigned char* DLCFile::getData(unsigned int& size)
{
    size = 0;
    return NULL;
}

void DLCFile::addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value)
{
}

std::wstring DLCFile::getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring& name)
{
    return L"";
}

bool DLCFile::getParameterAsBool(DLCManager::EDLCParameterType parameter)
{
    return false;
}
