#ifndef GAME_CORE_DLCGAMERULESFILE_HPP
#define GAME_CORE_DLCGAMERULESFILE_HPP

#include <string>

#include "game/core/DLCGameRules.hpp"

class DLCGameRulesFile : public DLCGameRules
{
public:
    DLCGameRulesFile(const std::wstring& path);

    virtual void addData(unsigned char* pData, unsigned int size);
    virtual unsigned char* getData(unsigned int& size);
    virtual void addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value);
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType parameter);

protected:
    unsigned char* m_pData;
    unsigned int m_dataSize;
    bool m_bFree;
};

#endif
