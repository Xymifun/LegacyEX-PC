#ifndef PLATFORM_WIIU_CNINTENDODLCINFO_HPP
#define PLATFORM_WIIU_CNINTENDODLCINFO_HPP

#include "game/core/cDLCInfo.hpp"

#include <boost/unordered_map.hpp>
#include <string>

class cNintendoDLCInfo : public cDLCInfo
{
public:
    cNintendoDLCInfo();

    virtual unsigned int GetDLCInfoCount();
    virtual void AddDLCInfo(DLC_INFO* pDLCInfo);
    virtual DLC_INFO* GetDLCInfoForPackID(unsigned int packID);
    virtual DLC_INFO* GetDLCInfoFromBanner(char* pBannerName);
    virtual DLC_INFO* GetDLCInfoByCatalogIndex(unsigned int catalogIndex);
    virtual bool GetDLCPurchased(char* pPackName);
    virtual bool SetDLCPurchased(unsigned int catalogIndex, bool purchased);
    virtual DLC_INFO* GetDLCInfoByIndex(unsigned int index);
    virtual DLC_INFO* GetDLCInfoByRomName(const char* pRomName);
    virtual void LoadLocalDLCImages();
    virtual void FreeLocalDLCImages();
    virtual void LoadLocalDLCImage(DLC_INFO* pDLCInfo);

private:
    boost::unordered_map<unsigned int, DLC_INFO*> m_dlcInfoByCatalogIndex;
    boost::unordered_map<std::wstring, DLC_INFO*> m_dlcInfoByPackName;
};

#endif
