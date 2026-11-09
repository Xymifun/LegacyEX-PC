#ifndef GAME_CORE_DLCLOCALISATIONFILE_HPP
#define GAME_CORE_DLCLOCALISATIONFILE_HPP

#include <string>

#include "game/core/DLCFile.hpp"

class StringTable;

class DLCLocalisationFile : public DLCFile
{
public:
    DLCLocalisationFile(const std::wstring& path);
    virtual ~DLCLocalisationFile();

    virtual void addData(unsigned char* pData, unsigned int size);
    StringTable* getStringTable();

protected:
    StringTable* m_pStringTable;
};

#endif
