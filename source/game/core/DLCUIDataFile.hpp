#ifndef GAME_CORE_DLCUIDATAFILE_HPP
#define GAME_CORE_DLCUIDATAFILE_HPP

#include <string>

#include "game/core/DLCFile.hpp"

class DLCUIDataFile : public DLCFile
{
public:
    DLCUIDataFile(const std::wstring& path);
    virtual ~DLCUIDataFile();

    virtual unsigned char* getData(unsigned int& size);
    virtual void addData(unsigned char* pData, unsigned int size, bool bTakeDataOwnership);

protected:
    unsigned char* m_pData;
    unsigned int m_dataSize;
    bool m_bTakeDataOwnership;
};

#endif
