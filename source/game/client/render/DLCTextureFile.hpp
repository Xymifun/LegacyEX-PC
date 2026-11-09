#ifndef GAME_CLIENT_RENDER_DLCTEXTUREFILE_HPP
#define GAME_CLIENT_RENDER_DLCTEXTUREFILE_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/core/DLCFile.hpp"

class DLCTextureFile : public DLCFile
{
public:
    DLCTextureFile(const std::wstring& path);

    virtual void addData(unsigned char* pData, unsigned int size);
    virtual unsigned char* getData(unsigned int& size);
    virtual void addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value);
    virtual std::wstring getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring& name);
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType parameter);

protected:
    bool m_hasAnimation;
    boost::unordered_map<std::wstring, std::wstring> m_parameters;
    bool m_disableMipMap;
    unsigned char* m_pData;
    unsigned int m_dataSize;
};

#endif
