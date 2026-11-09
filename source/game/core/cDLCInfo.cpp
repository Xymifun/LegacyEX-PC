#include "game/core/cDLCInfo.hpp"

#include <stddef.h>

cDLCInfo::cDLCInfo()
{
}

unsigned int cDLCInfo::GetDLCInfoCount()
{
    return 0;
}

void cDLCInfo::AddDLCInfo(DLC_INFO* pDLCInfo)
{
}

DLC_INFO* cDLCInfo::GetDLCInfo(unsigned int catalogIndex)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfo(const wchar_t* pPackName)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfo(char* pPackName)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfoForPackID(unsigned int packID)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfoForPackFolderName(const wchar_t* pFolderName)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfoForPackDisplayName(const wchar_t* pDisplayName)
{
    return NULL;
}

unsigned int cDLCInfo::GetDLCOfferIDForPackID(unsigned int packID)
{
    return 0;
}

DLC_INFO* cDLCInfo::GetDLCInfoByIndex(unsigned int index)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfoByRomName(const char* pRomName)
{
    return NULL;
}

DLC_INFO* cDLCInfo::GetDLCInfoByCatalogIndex(unsigned int catalogIndex)
{
    return NULL;
}

bool cDLCInfo::GetDLCPurchased(char* pPackName)
{
    return false;
}

bool cDLCInfo::SetDLCPurchased(unsigned int catalogIndex, bool purchased)
{
    return false;
}

DLC_INFO* cDLCInfo::GetDLCInfoFromBanner(char* pBannerName)
{
    return NULL;
}

void cDLCInfo::LoadLocalDLCImages()
{
}

void cDLCInfo::FreeLocalDLCImages()
{
}

void cDLCInfo::LoadLocalDLCImage(DLC_INFO* pDLCInfo)
{
}

void cDLCInfo::DLC_RetrieveImageData(char* pBannerName, void** ppImageData, unsigned int* pImageSize,
                                     bool* pbFreeImageData)
{
    *pImageSize = 0;
    *ppImageData = NULL;
    *pbFreeImageData = false;

    DLC_INFO* pDLCInfo = this->GetDLCInfoFromBanner(pBannerName);
    if (pDLCInfo != NULL && pDLCInfo->ImageSize != 0)
    {
        void* pImageData = pDLCInfo->pImageData;
        *pImageSize = pDLCInfo->ImageSize;
        *ppImageData = pImageData;
        *pbFreeImageData = false;
    }
}
