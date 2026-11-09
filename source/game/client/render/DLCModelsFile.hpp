#ifndef GAME_CLIENT_RENDER_DLCMODELSFILE_HPP
#define GAME_CLIENT_RENDER_DLCMODELSFILE_HPP

#include <string>

#include "game/core/DLCFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DLCModelsFile : public DLCFile
{
public:
    DLCModelsFile(const std::wstring& path);
    virtual ~DLCModelsFile();

    virtual void addData(unsigned char* pData, unsigned int size);

    arrayWithLength<unsigned char> getData();

protected:
    arrayWithLength<unsigned char> m_data;
};

#endif
