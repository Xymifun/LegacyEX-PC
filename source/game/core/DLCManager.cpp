#include "game/core/DLCManager.hpp"
#include "game/core/BigEndian.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <sstream>
#include <cstring>
#include <wchar.h>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern ConsoleUIController gConsoleUIController;

char* wstringtofilename(const std::wstring& text, char (&fileName)[256]);

const wchar_t* s_parameterNames[] = {L"DISPLAYNAME",
                                     L"THEMENAME",
                                     L"LOC_KEY",
                                     L"FREE",
                                     L"CREDIT",
                                     L"CAPEPATH",
                                     L"BOX",
                                     L"ANIM",
                                     L"GAME_FLAGS",
                                     L"PACKID",
                                     L"NETHERPARTICLECOLOUR",
                                     L"ENCHANTTEXTCOLOUR",
                                     L"ENCHANTTEXTFOCUSCOLOUR",
                                     L"DATAPATH",
                                     L"PACKVERSION",
                                     L"DISPLAYNAMEID",
                                     L"THEMENAMEID",
                                     L"CLOUDHEIGHT",
                                     L"CREDITID",
                                     L"DISABLEMIPMAP",
                                     L"OFFSET",
                                     L"XMLVERSION"};

char s_dlcDataFileName[256];

DLCManager::DLCManager()
{
    m_bCorruptDLCFound = true;
}

DLCManager::~DLCManager()
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        delete *it;
    }
}

DLCManager::EDLCParameterType DLCManager::getParameterType(const std::wstring& name)
{
    EDLCParameterType result = eDLCParameterType_Invalid;

    for (int i = 0; i < eDLCParameterType_NumTypes; i++)
    {
        const wchar_t* parameterName = s_parameterNames[i];

        if (name == parameterName)
        {
            result = (EDLCParameterType)i;
            break;
        }
    }

    return result;
}

int DLCManager::getPackCount(EDLCType type)
{
    if (type != eDLCType_All)
    {
        int count = 0;

        for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
        {
            if ((*it)->getDLCItemsCount(type) > 0)
            {
                count++;
            }
        }

        return count;
    }

    return m_packs.size();
}

void DLCManager::addPack(DLCPack* pPack)
{
    m_packs.push_back(pPack);
}

void DLCManager::removePack(DLCPack* pPack)
{
    if (pPack != NULL)
    {
        std::vector<DLCPack*>::iterator it = std::find(m_packs.begin(), m_packs.end(), pPack);

        if (it != m_packs.end())
        {
            m_packs.erase(it);
        }

        delete pPack;
    }
}

void DLCManager::LanguageChanged()
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;
        pPack->UpdateLanguage();
    }
}

DLCPack* DLCManager::getPack(const std::wstring& name)
{
    DLCPack* pFound = NULL;

    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;
        const std::wstring& packName = pPack->getName();

        if (packName == name)
        {
            pFound = pPack;
            break;
        }
    }

    return pFound;
}

#ifdef MINECRAFT_PC
bool DLCManager::PcPackIdFree(DLCPack* pack, unsigned int id)
{
    if (id == 0 || id > 0xFFFFFFU)
        return false;
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
        if (*it != pack && ((*it)->GetPackId() & 0xFFFFFFU) == id)
            return false;
    return true;
}

void DLCManager::PcGiveLauncherPackId(DLCPack* pack)
{
    unsigned int hash = 2166136261U;
    const std::wstring name = pack->getName();
    for (std::wstring::size_type i = 0; i < name.size(); ++i)
        hash = (hash ^ static_cast<unsigned int>(name[i])) * 16777619U;
    unsigned int id = 0xE00000U | (hash & 0xFFFFFU);
    while (!PcPackIdFree(pack, id))
        id = 0xE00000U | ((id + 1) & 0x1FFFFFU);
    pack->SetPackId(id);
    pack->PcSetLauncherPack();
}

#endif
DLCPack* DLCManager::getPackFromPackID(int packID)
{
    std::vector<DLCPack*>::iterator it;
    std::vector<DLCPack*>::iterator end;
    for (it = m_packs.begin(), end = m_packs.end(); it != end; ++it)
    {
        DLCPack* pPack = *it;

        if ((int)pPack->GetPackId() == packID)
        {
            return pPack;
        }
    }

    return NULL;
}

DLCPack* DLCManager::getPack(unsigned int index, EDLCType type)
{
    if (type != eDLCType_All)
    {
        unsigned int matchIndex = 0;

        for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
        {
            DLCPack* pPack = *it;

            if (pPack->getDLCItemsCount(type) > 0)
            {
                if (matchIndex == index)
                {
                    return pPack;
                }

                matchIndex++;
            }
        }
    }
    else if (index < m_packs.size())
    {
        return m_packs[index];
    }

    return NULL;
}

DLCFile* DLCManager::getSkinFile(const std::wstring& fileName)
{
    DLCFile* pFile = NULL;

    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;
        pFile = pPack->getSkinFile(fileName);

        if (pFile != NULL)
        {
            break;
        }
    }

    return pFile;
}

DLCPack* DLCManager::getPackContainingSkin(const std::wstring& fileName)
{
    DLCPack* pFound = NULL;

    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;

        if (pPack->getDLCItemsCount(eDLCType_Skin) > 0 && pPack->doesPackContainSkin(fileName))
        {
            pFound = pPack;
            break;
        }
    }

    return pFound;
}

bool DLCManager::IsDlcPackTemporarilyFree(int packID)
{
    bool bFree = false;
    long long now = System::currentUnixEpochTimeSeconds();

    boost::unordered_map<int, FreePackPeriod>::iterator it = m_freeDLCPackPeriods.find(packID);

    if (it != m_freeDLCPackPeriods.end())
    {
        if (it->second.startTime <= now && it->second.endTime >= now)
        {
            bFree = true;
        }
    }

    return bFree;
}

DLCFile* DLCManager::getSkinFile(int skinIndex, bool& bIsDefaultSkin, bool& bHasPurchasedPack)
{
    DLCFile* pFile = NULL;

    bIsDefaultSkin = false;
    bHasPurchasedPack = false;

    if (((skinIndex >> 31) & 1) != 0)
    {
        wchar_t fileName[256];

        swprintf(fileName, 256, L"dlcskin%08d.png", skinIndex & 0x7FFFFFFF);

        DLCPack* pPack = getPackContainingSkin(fileName);

        if (pPack != NULL)
        {
            pFile = pPack->getSkinFile(fileName);
            bHasPurchasedPack = pPack->hasPurchasedPack();
        }
    }
    else if ((skinIndex & 0x7FFFFFE0) == 0)
    {
        bIsDefaultSkin = true;
        bHasPurchasedPack = true;
    }

    return pFile;
}

int DLCManager::checkForCorruptDLCAndAlert(bool bShowMessage)
{
    int numCorruptPacks = m_numCorruptPacks;
    DLCPack* pFirstCorruptPack = NULL;

    std::vector<DLCPack*>::iterator it;
    std::vector<DLCPack*>::iterator end;

    for (it = m_packs.begin(), end = m_packs.end(); it != end; ++it)
    {
        DLCPack* pPack = *it;

        if (pPack->IsCorrupt())
        {
            numCorruptPacks++;

            if (pFirstCorruptPack == NULL)
            {
                pFirstCorruptPack = pPack;
            }
        }
    }

    if (numCorruptPacks != 0 && bShowMessage)
    {
        int messageParameters[1];

        messageParameters[0] = IDS_CONFIRM_OK;

        if (numCorruptPacks == 1 && pFirstCorruptPack != NULL)
        {
            wchar_t messageText[132];

            swprintf(messageText, 132, L"%ls\n\n%%ls", pFirstCorruptPack->getName().c_str());

            gConsoleUIController.RequestErrorMessage(IDS_CORRUPT_DLC_TITLE, IDS_CORRUPT_DLC, messageParameters, 1,
                                                     gProfile->GetPrimaryPad(), NULL, NULL, messageText, true);
        }
        else
        {
            gConsoleUIController.RequestErrorMessage(IDS_CORRUPT_DLC_TITLE, 0x75E79C02, messageParameters, 1,
                                                     gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        }
    }

    m_bCorruptDLCFound = false;

    return numCorruptPacks;
}

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
static bool SwapPackWord(unsigned char* pData, unsigned int size, unsigned int offset, unsigned int* pValue)
{
    if (offset + 4 > size)
        return false;
    unsigned int* pWord = (unsigned int*)(pData + offset);
    *pWord = BigEndian::ToHost(*pWord);
    *pValue = *pWord;
    return true;
}

static bool SwapPackText(unsigned char* pData, unsigned int size, unsigned int offset, unsigned int length)
{
    if (offset + length * 2 > size)
        return false;
    unsigned short* pText = (unsigned short*)(pData + offset);
    for (unsigned int i = 0; i < length; i++)
        pText[i] = BigEndian::ToHost(pText[i]);
    return true;
}

static void ConvertPackToHostOrder(unsigned char* pData, unsigned int size, bool bAllowXMLVersion)
{
    if (size >= 8)
    {
        unsigned int hostVersion;
        unsigned int hostParameterCount;
        std::memcpy(&hostVersion, pData, 4);
        std::memcpy(&hostParameterCount, pData + 4, 4);
        if (hostVersion <= 3 && hostParameterCount < 0x10000)
            return;
    }

    unsigned int version;
    unsigned int parameterCount;
    if (!SwapPackWord(pData, size, 0, &version) || !SwapPackWord(pData, size, 4, &parameterCount))
        return;

    bool bHasXMLVersion = false;
    unsigned int offset = 8;
    for (unsigned int i = 0; i < parameterCount; i++)
    {
        unsigned int id;
        unsigned int length;
        if (!SwapPackWord(pData, size, offset, &id) || !SwapPackWord(pData, size, offset + 4, &length)
            || !SwapPackText(pData, size, offset + 8, length))
            return;
        if (bAllowXMLVersion && char16towstring((wchar_t*)(pData + offset + 8)) == L"XMLVERSION")
            bHasXMLVersion = true;
        offset += length * 2 + 12;
    }

    unsigned int value;
    if (bHasXMLVersion)
    {
        if (!SwapPackWord(pData, size, offset, &value))
            return;
        offset += 4;
    }

    unsigned int itemCount;
    if (!SwapPackWord(pData, size, offset, &itemCount))
        return;
    offset += 4;

    std::vector<unsigned int> itemSizes;
    std::vector<unsigned int> itemTypes;
    for (unsigned int i = 0; i < itemCount; i++)
    {
        unsigned int itemSize;
        unsigned int itemType;
        unsigned int length;
        if (!SwapPackWord(pData, size, offset, &itemSize) || !SwapPackWord(pData, size, offset + 4, &itemType)
            || !SwapPackWord(pData, size, offset + 8, &length) || !SwapPackText(pData, size, offset + 12, length))
            return;
        itemSizes.push_back(itemSize);
        itemTypes.push_back(itemType);
        offset += length * 2 + 16;
    }

    for (unsigned int i = 0; i < itemCount; i++)
    {
        unsigned int itemParameterCount;
        if (!SwapPackWord(pData, size, offset, &itemParameterCount))
            return;
        offset += 4;
        for (unsigned int j = 0; j < itemParameterCount; j++)
        {
            unsigned int id;
            unsigned int length;
            if (!SwapPackWord(pData, size, offset, &id) || !SwapPackWord(pData, size, offset + 4, &length)
                || !SwapPackText(pData, size, offset + 8, length))
                return;
            offset += length * 2 + 12;
        }
        if (offset + itemSizes[i] > size)
            return;
        if (itemTypes[i] == DLCManager::eDLCType_TexturePack || itemTypes[i] == DLCManager::eDLCType_SkinPack)
            ConvertPackToHostOrder(pData + offset, itemSizes[i], true);
        else if (itemTypes[i] == DLCManager::eDLCType_Audio)
            ConvertPackToHostOrder(pData + offset, itemSizes[i], false);
        offset += itemSizes[i];
    }
}
#endif

bool DLCManager::processDLCDataFile(unsigned int& numItemsAdded, unsigned char* pData, unsigned int size,
                                    DLCPack* pPack, bool bTakeDataOwnership)
{
    boost::unordered_map<int, EDLCParameterType> parameterTypes;
    bool bHasXMLVersion = false;
    unsigned int xmlVersion = 0;

    if (*(unsigned int*)pData < 3)
    {
        delete[] pData;
        return false;
    }

    pPack->SetDataPointer(pData, bTakeDataOwnership);

    unsigned int parameterCount = ((unsigned int*)pData)[1];
    unsigned int offset = 8;
    unsigned int* pParameter = (unsigned int*)(pData + offset);

    for (unsigned int i = 0; i < parameterCount; i++)
    {
        std::wstring parameterName = char16towstring((wchar_t*)(pParameter + 2));
        EDLCParameterType parameterType = getParameterType(parameterName);

        if (parameterType != eDLCParameterType_Invalid)
        {
            parameterTypes[(int)pParameter[0]] = parameterType;
        }

        offset += pParameter[1] * 2 + 12;
        pParameter = (unsigned int*)(pData + offset);

        if (parameterType == eDLCParameterType_XMLVersion)
        {
            bHasXMLVersion = true;
        }
    }

    if (bHasXMLVersion)
    {
        xmlVersion = *(unsigned int*)(pData + offset);
        offset += 4;
    }

    unsigned int itemCount = *(unsigned int*)(pData + offset);
    offset += 4;

    unsigned int dataOffset = offset;
    unsigned int* pItemData = (unsigned int*)(pData + dataOffset);

    for (unsigned int i = 0; i < itemCount; i++)
    {
        dataOffset += pItemData[2] * 2 + 16;
        pItemData = (unsigned int*)(pData + dataOffset);
    }

    while (itemCount != 0)
    {
        unsigned int* pItemHeader = (unsigned int*)(pData + offset);
        EDLCType itemType = (EDLCType)pItemHeader[1];

        DLCFile* pFile = NULL;
        DLCPack* pTexturePack = NULL;
        DLCPack* pChildPack = NULL;

        if (itemType == eDLCType_TexturePack)
        {
            pTexturePack = new DLCPack(pPack->getName(), pPack->getLicenseMask());
        }
        else if (itemType == eDLCType_SkinPack)
        {
            pChildPack = new DLCPack(pPack->getName(), pPack->getLicenseMask());
        }
        else if (itemType != eDLCType_Pack)
        {
            pFile = pPack->addFile(itemType, char16towstring((wchar_t*)(pItemHeader + 3)));

            if (itemType == eDLCType_Skin && bHasXMLVersion)
            {
                wchar_t versionText[64];

                swprintf(versionText, 64, L"%i", xmlVersion);

                pFile->addParameter(eDLCParameterType_XMLVersion, versionText);
            }
        }

        unsigned int itemParameterCount = pItemData[0];
        pItemData++;

        for (; itemParameterCount != 0; itemParameterCount--)
        {
            boost::unordered_map<int, EDLCParameterType>::iterator it = parameterTypes.find((int)pItemData[0]);

            if (it != parameterTypes.end())
            {
                if (itemType == eDLCType_Pack)
                {
                    pPack->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                    numItemsAdded++;
                }
                else if (pFile != NULL)
                {
                    pFile->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                }
                else if (pTexturePack != NULL)
                {
                    pTexturePack->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                }
                else if (pChildPack != NULL)
                {
                    pChildPack->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                }
            }

            pItemData = (unsigned int*)((unsigned char*)pItemData + pItemData[1] * 2 + 12);
        }

        if (pTexturePack != NULL)
        {
            unsigned int numChildItemsAdded = 0;
#ifdef MINECRAFT_PC
            bool bLoaded = *pItemData >= 3 && processDLCDataFile(numChildItemsAdded, (unsigned char*)pItemData,
                                                                 pItemHeader[0], pTexturePack, false);
#else
            bool bLoaded = processDLCDataFile(numChildItemsAdded, (unsigned char*)pItemData, pItemHeader[0],
                                              pTexturePack, false);
#endif

            pPack->SetDataPointer(NULL, false);

            if (!bLoaded || numChildItemsAdded == 0)
            {
#ifdef MINECRAFT_PC
                pTexturePack->SetDataPointer(NULL, false);
#endif
                delete pTexturePack;
                numItemsAdded++;
            }
            else
            {
                pPack->addChildPack(pTexturePack);

                if (pTexturePack->getDLCItemsCount(eDLCType_Texture) > 0)
                {
                    unsigned int packID = pTexturePack->GetPackId();

                    Minecraft::GetInstance()->mTexturePackRepository->addTexturePackFromDLC(pTexturePack, packID);
                }

                numItemsAdded++;
            }
        }
        else if (pChildPack != NULL)
        {
            unsigned int numChildItemsAdded = 0;
#ifdef MINECRAFT_PC
            bool bLoaded = *pItemData >= 3 && processDLCDataFile(numChildItemsAdded, (unsigned char*)pItemData,
                                                                 pItemHeader[0], pChildPack, false);
#else
            bool bLoaded = processDLCDataFile(numChildItemsAdded, (unsigned char*)pItemData, pItemHeader[0], pChildPack,
                                              false);
#endif

            pPack->SetDataPointer(NULL, false);

            if (!bLoaded || numChildItemsAdded == 0)
            {
#ifdef MINECRAFT_PC
                pChildPack->SetDataPointer(NULL, false);
#endif
                delete pChildPack;
                numItemsAdded++;
            }
            else
            {
                pPack->addChildPack(pChildPack);
                numItemsAdded++;
            }
        }
        else if (pFile != NULL)
        {
            pFile->addData((unsigned char*)pItemData, pItemHeader[0]);

            if (pItemHeader[1] == eDLCType_Skin)
            {
                gApp.m_dlcSkinNames.push_back(char16towstring((wchar_t*)(pItemHeader + 3)));
            }

            numItemsAdded++;
        }

        itemCount--;
        offset += pItemHeader[2] * 2 + 16;
        pItemData = (unsigned int*)((unsigned char*)pItemData + pItemHeader[0]);
    }

    if (pPack->getDLCItemsCount(eDLCType_GameRules) > 0 || pPack->getDLCItemsCount(eDLCType_GameRulesHeader) > 0)
    {
        gApp.mGameRuleManager.loadGameRules(pPack);
    }

    pPack->getDLCItemsCount(eDLCType_Audio);

    return true;
}

bool DLCManager::readDLCDataFile(unsigned int& numItemsAdded, const std::string& fileName, DLCPack* pPack,
                                 unsigned int packID, bool bTakeDataOwnership)
{
    std::wstring wideFileName = convStringToWstring(fileName);
    int file = CreateFileA(fileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == -1)
    {
        GetLastError();
        if (numItemsAdded == 0)
        {
            removePack(pPack);
        }

        return false;
    }

    const unsigned int fileSize = GetFileSize(file, NULL);
    unsigned char* pData = new unsigned char[fileSize];
    unsigned int bytesRead;
    if (ReadFile(file, pData, fileSize, &bytesRead, NULL) != 0)
    {
        CloseHandle(file);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
        ConvertPackToHostOrder(pData, bytesRead, true);
#endif
    }
    else
    {
        pPack->SetIsCorrupt(true);
        m_bCorruptDLCFound = true;

        return false;
    }

    return processDLCDataFile(numItemsAdded, pData, bytesRead, pPack, bTakeDataOwnership);
}

bool DLCManager::readDLCDataFile(unsigned int& numItemsAdded, const std::wstring& fileName, DLCPack* pPack,
                                 unsigned int packID, bool bTakeDataOwnership)
{
    std::string narrowFileName = wstringtofilename(fileName, s_dlcDataFileName);
    bool result = readDLCDataFile(numItemsAdded, narrowFileName, pPack, packID, bTakeDataOwnership);

    return result;
}

unsigned int DLCManager::retrievePackID(unsigned char* pData, unsigned int size)
{
    unsigned int packID = 0;
    std::wstring packName = L"";
    boost::unordered_map<int, EDLCParameterType> parameterTypes;
    bool bHasXMLVersion = false;

    if (*(unsigned int*)pData < 3)
    {
        return 0;
    }

    unsigned int parameterCount = ((unsigned int*)pData)[1];
    unsigned int offset = 8;
    unsigned int* pParameter = (unsigned int*)(pData + offset);

    for (unsigned int i = 0; i < parameterCount; i++)
    {
        std::wstring parameterName = char16towstring((wchar_t*)(pParameter + 2));
        EDLCParameterType parameterType = getParameterType(parameterName);

        if (parameterType != eDLCParameterType_Invalid)
        {
            parameterTypes[(int)pParameter[0]] = parameterType;
        }

        offset += pParameter[1] * 2 + 12;
        pParameter = (unsigned int*)(pData + offset);

        if (parameterType == eDLCParameterType_XMLVersion)
        {
            bHasXMLVersion = true;
        }
    }

    if (bHasXMLVersion)
    {
        offset += 4;
    }

    unsigned int itemCount = *(unsigned int*)(pData + offset);
    offset += 4;

    unsigned int dataOffset = offset;
    unsigned int* pItemData = (unsigned int*)(pData + dataOffset);

    for (unsigned int i = 0; i < itemCount; i++)
    {
        dataOffset += pItemData[2] * 2 + 16;
        pItemData = (unsigned int*)(pData + dataOffset);
    }

    while (itemCount != 0)
    {
        unsigned int* pItemHeader = (unsigned int*)(pData + offset);
        unsigned int itemParameterCount = pItemData[0];
        EDLCType itemType = (EDLCType)pItemHeader[1];
        pItemData++;

        for (; itemParameterCount != 0; itemParameterCount--)
        {
            boost::unordered_map<int, EDLCParameterType>::iterator it = parameterTypes.find((int)pItemData[0]);

            if (it != parameterTypes.end())
            {
                if (itemType == eDLCType_Pack)
                {
                    if (it->second == eDLCParameterType_PackID)
                    {
                        {
                            std::wstring packIDText = char16towstring((wchar_t*)(pItemData + 2));
                            std::wstringstream stream;

                            stream << std::dec << packIDText.c_str();
                            stream >> packID;
                        }

                        parameterTypes.clear();

                        return packID;
                    }
                }
                else if (itemType == eDLCType_Skin && packName.empty() && it->second == eDLCParameterType_DisplayName)
                {
                    packName = char16towstring((wchar_t*)(pItemData + 2));
                }
            }

            pItemData = (unsigned int*)((unsigned char*)pItemData + pItemData[1] * 2 + 12);
        }

        itemCount--;
        offset += pItemHeader[2] * 2 + 16;
        pItemData = (unsigned int*)((unsigned char*)pItemData + pItemHeader[0]);
    }

    parameterTypes.clear();

    return packID;
}

unsigned int DLCManager::retrievePackIDFromDLCDataFile(const std::string& fileName)
{
    std::wstring wideFileName = convStringToWstring(fileName);
    int file = CreateFileA(fileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == -1)
    {
        return 0;
    }

    const unsigned int fileSize = GetFileSize(file, NULL);
    unsigned char* pData = new unsigned char[fileSize];
    unsigned int bytesRead;
    if (ReadFile(file, pData, fileSize, &bytesRead, NULL) != 0)
    {
        CloseHandle(file);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
        ConvertPackToHostOrder(pData, bytesRead, true);
#endif
    }
    else
    {
        delete[] pData;

        return 0;
    }

    unsigned int packID = retrievePackID(pData, bytesRead);

    delete[] pData;

    return packID;
}

bool DLCManager::IsMiniGameTemporarilyFree(int miniGameID)
{
    bool bFree = false;
    long long now = System::currentUnixEpochTimeSeconds();

    boost::unordered_map<int, FreePackPeriod>::iterator it = m_freeMiniGamePeriods.find(miniGameID);

    if (it != m_freeMiniGamePeriods.end())
    {
        if (it->second.startTime <= now && it->second.endTime >= now)
        {
            bFree = true;
        }
    }

    return bFree;
}
