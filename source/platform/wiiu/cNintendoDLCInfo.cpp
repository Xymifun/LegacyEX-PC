#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>

cNintendoDLCInfo::cNintendoDLCInfo()
{
}

unsigned int cNintendoDLCInfo::GetDLCInfoCount()
{
    return (unsigned int)m_dlcInfoByCatalogIndex.size();
}

void cNintendoDLCInfo::AddDLCInfo(DLC_INFO* pDLCInfo)
{
    m_dlcInfoByCatalogIndex[pDLCInfo->CatalogIndex] = pDLCInfo;

    wchar_t packName[64];
    mbstowcs(packName, pDLCInfo->PackName, 64);
    m_dlcInfoByPackName[std::wstring(packName)] = pDLCInfo;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoForPackID(unsigned int packID)
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (packID == pDLCInfo->PackID)
        {
            return pDLCInfo;
        }
    }

    return NULL;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoFromBanner(char* pBannerName)
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (std::strcmp(pDLCInfo->BannerName, pBannerName) == 0)
        {
            return pDLCInfo;
        }
    }

    return NULL;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoByCatalogIndex(unsigned int catalogIndex)
{
    if (!m_dlcInfoByCatalogIndex.empty())
    {
        boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.find(catalogIndex);
        if (it != m_dlcInfoByCatalogIndex.end())
        {
            return it->second;
        }
    }

    return NULL;
}

bool cNintendoDLCInfo::GetDLCPurchased(char* pPackName)
{
    if (!m_dlcInfoByPackName.empty())
    {
        wchar_t packName[64];
        mbstowcs(packName, pPackName, 64);

        boost::unordered_map<std::wstring, DLC_INFO*>::const_iterator it = m_dlcInfoByPackName.find(
            std::wstring(packName));
        if (it != m_dlcInfoByPackName.end())
        {
            DLC_INFO* pDLCInfo = it->second;
            return pDLCInfo->Purchased;
        }
    }

    return false;
}

bool cNintendoDLCInfo::SetDLCPurchased(unsigned int catalogIndex, bool purchased)
{
    bool found = false;
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); it++)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (pDLCInfo->CatalogIndex == catalogIndex)
        {
            pDLCInfo->Purchased = purchased;
            found = true;
            break;
        }
    }

    return found;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoByIndex(unsigned int index)
{
    unsigned int currentIndex = 0;
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); it++)
    {
        if (index == currentIndex)
        {
            return it->second;
        }
        ++currentIndex;
    }

    return NULL;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoByRomName(const char* pRomName)
{
    std::string romName(pRomName);
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); it++)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (romName == pDLCInfo->RomName)
        {
            return pDLCInfo;
        }
    }

    return NULL;
}

void cNintendoDLCInfo::LoadLocalDLCImages()
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        DLC_INFO* pDLCInfo = it->second;
        LoadLocalDLCImage(pDLCInfo);
    }
}

void cNintendoDLCInfo::FreeLocalDLCImages()
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (pDLCInfo->ImageSize != 0)
        {
            std::free(pDLCInfo->pImageData);
            pDLCInfo->ImageSize = 0;
            pDLCInfo->pImageData = NULL;
        }
    }
}

void cNintendoDLCInfo::LoadLocalDLCImage(DLC_INFO* pDLCInfo)
{
    char fileName[68];
    std::sprintf(fileName, "WiiU/DLCImages/%s.png", pDLCInfo->BannerName);

    int file = CreateFileA(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file != INVALID_HANDLE_VALUE)
    {
        unsigned int fileSizeHigh = 0;
        pDLCInfo->ImageSize = GetFileSize(file, &fileSizeHigh);
        if (pDLCInfo->ImageSize != 0)
        {
            pDLCInfo->pImageData = std::malloc(pDLCInfo->ImageSize);
            unsigned int bytesRead;
            if (ReadFile(file, pDLCInfo->pImageData, pDLCInfo->ImageSize, &bytesRead, NULL) == 0)
            {
                std::free(pDLCInfo->pImageData);
                pDLCInfo->ImageSize = 0;
            }
        }
        CloseHandle(file);
    }
}
