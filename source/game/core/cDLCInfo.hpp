#ifndef GAME_CORE_CDLCINFO_HPP
#define GAME_CORE_CDLCINFO_HPP

struct DLC_INFO
{
    unsigned int CatalogIndex;
    int ContentType;
    int PackType;
    char PackName[64];
    char BannerName[24];
    unsigned int PackID;
    unsigned int OfferID;
    unsigned int SortIndex;
    bool Purchased;
    void* pImageData;
    unsigned int ImageSize;
    char RomName[8];
};

class cDLCInfo
{
public:
    cDLCInfo();

    virtual unsigned int GetDLCInfoCount();
    virtual void AddDLCInfo(DLC_INFO* pDLCInfo);
    virtual DLC_INFO* GetDLCInfo(unsigned int catalogIndex);
    virtual DLC_INFO* GetDLCInfo(const wchar_t* pPackName);
    virtual DLC_INFO* GetDLCInfo(char* pPackName);
    virtual DLC_INFO* GetDLCInfoForPackID(unsigned int packID);
    virtual DLC_INFO* GetDLCInfoForPackFolderName(const wchar_t* pFolderName);
    virtual DLC_INFO* GetDLCInfoForPackDisplayName(const wchar_t* pDisplayName);
    virtual unsigned int GetDLCOfferIDForPackID(unsigned int packID);
    virtual DLC_INFO* GetDLCInfoByIndex(unsigned int index);
    virtual DLC_INFO* GetDLCInfoByRomName(const char* pRomName);
    virtual DLC_INFO* GetDLCInfoByCatalogIndex(unsigned int catalogIndex);
    virtual bool GetDLCPurchased(char* pPackName);
    virtual bool SetDLCPurchased(unsigned int catalogIndex, bool purchased);
    virtual DLC_INFO* GetDLCInfoFromBanner(char* pBannerName);
    virtual void LoadLocalDLCImages();
    virtual void FreeLocalDLCImages();
    virtual void LoadLocalDLCImage(DLC_INFO* pDLCInfo);

    void DLC_RetrieveImageData(char* pBannerName, void** ppImageData, unsigned int* pImageSize, bool* pbFreeImageData);
};

#endif
