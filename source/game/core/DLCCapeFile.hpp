#ifndef GAME_CORE_DLC_CAPE_FILE_HPP
#define GAME_CORE_DLC_CAPE_FILE_HPP

#include "game/core/DLCFile.hpp"

class DLCCapeFile : public DLCFile
{
public:
    DLCCapeFile(const std::wstring& path);

    virtual void addData(unsigned char* data, unsigned int size);
};

#endif
