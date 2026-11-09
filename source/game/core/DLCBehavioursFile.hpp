#ifndef GAME_CORE_DLCBEHAVIOURSFILE_HPP
#define GAME_CORE_DLCBEHAVIOURSFILE_HPP

#include <string>

#include "game/core/DLCFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DLCBehavioursFile : public DLCFile
{
public:
    DLCBehavioursFile(const std::wstring& path);
    virtual ~DLCBehavioursFile();

    virtual void addData(unsigned char* pData, unsigned int size);

    arrayWithLength<unsigned char> getData();

protected:
    arrayWithLength<unsigned char> m_data;
};

#endif
