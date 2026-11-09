#include "game/core/DLCPack.hpp"

#include <algorithm>
#include <cstdlib>
#include <cwchar>
#include <sstream>
#include <vector>

#include "game/client/audio/DLCAudioFile.hpp"
#include "game/client/render/DLCModelsFile.hpp"
#include "game/client/render/DLCTextureFile.hpp"
#include "game/core/DLCCapeFile.hpp"
#include "game/core/DLCBehavioursFile.hpp"
#include "game/core/DLCColourTableFile.hpp"
#include "game/core/DLCGameRulesFile.hpp"
#include "game/core/DLCGameRulesHeader.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCUIDataFile.hpp"
#include "game/data/StringTable.hpp"
#include "game/world/block/DLCMaterialFile.hpp"
#include "game/core/StringUtils.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

bool pathCmp(DLCFile* file);
unsigned int g_pathCompareTlsIndex = TlsAlloc();

bool pathCmp(DLCFile* file)
{
    std::wstring* path = static_cast<std::wstring*>(TlsGetValue(g_pathCompareTlsIndex));
    return *path == file->getPath();
}

DLCPack::DLCPack(const std::wstring& name, unsigned int licenseMask)
{
    m_dataPath = L"";
    m_name = name;
    m_licenseMask = licenseMask;
    m_dlcMountIndex = 0xffffffffu;
    m_purchaseOfferId = 0;
    m_isCorrupt = false;
    m_packId = 0;
#ifdef MINECRAFT_PC
    m_pcLauncherPack = false;
#endif
    m_packVersion = 0;
    m_data = NULL;
    m_ownsData = false;
    m_parentPack = NULL;
}

DLCPack::~DLCPack()
{
    for (std::vector<DLCPack*>::iterator child = m_childPacks.begin(); child != m_childPacks.end(); ++child)
        delete *child;

    int i;
    for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
    {
        std::vector<DLCFile*>::iterator it;
        for (it = m_files[i].begin(); it != m_files[i].end(); ++it)
            delete *it;
    }

    m_creditIds.clear();

    if (m_data != NULL && m_parentPack == NULL)
        delete[] m_data;
}

DLCFile* DLCPack::getFile(DLCManager::EDLCType type, unsigned int index, bool searchParent, bool searchChildren)
{
    DLCFile* file = NULL;
    if (type == DLCManager::eDLCType_All)
    {
        int i;
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
        {
            DLCFile* result = getFile(static_cast<DLCManager::EDLCType>(i), index, true, true);
            if (result != NULL)
                return result;
        }
    }
    else
    {
        if (index < m_files[type].size())
            file = m_files[type][index];
        if (file == NULL)
        {
            if (m_parentPack != NULL && searchParent)
                file = m_parentPack->getFile(type, index, true, false);
            if (file == NULL && searchChildren)
            {
                for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
                {
                    file = (*it)->getFile(type, index, false, true);
                    if (file != NULL)
                        return file;
                }
            }
        }
    }
    return file;
}

unsigned int DLCPack::getDLCItemsCount(DLCManager::EDLCType type)
{
    unsigned int total;
    if (type == DLCManager::eDLCType_All)
    {
        total = 0;
        unsigned int i;
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
            total += getDLCItemsCount(static_cast<DLCManager::EDLCType>(i));
    }
    else
    {
        total = static_cast<unsigned int>(m_files[type].size());
        for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
            total += (*it)->getDLCItemsCount(type);
    }
    return total;
}

DLCFile* DLCPack::getFile(DLCManager::EDLCType type, const std::wstring& name, bool searchParent, bool searchChildren)
{
    if (type == DLCManager::eDLCType_All)
    {
        int i;
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
        {
            DLCFile* result = getFile(static_cast<DLCManager::EDLCType>(i), name, true, true);
            if (result != NULL)
                return result;
        }
        return 0;
    }

    TlsSetValue(g_pathCompareTlsIndex, const_cast<std::wstring*>(&name));
    std::vector<DLCFile*>::iterator file = std::find_if(m_files[type].begin(), m_files[type].end(), pathCmp);
    if (file != m_files[type].end() && *file != NULL)
        return *file;

    if (m_parentPack != NULL && searchParent)
    {
        DLCFile* result = m_parentPack->getFile(type, name, true, false);
        if (result != NULL)
            return result;
    }
    if (searchChildren)
        for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
        {
            DLCFile* result = (*it)->getFile(type, name, false, true);
            if (result != NULL)
                return result;
        }
    return 0;
}

void DLCPack::UpdateLanguage()
{
    if (!m_files[DLCManager::eDLCType_Localisation].empty())
    {
        DLCFile* file = getFile(DLCManager::eDLCType_Localisation,
                                m_files[DLCManager::eDLCType_Localisation][0]->getPath(), true, true);
        static_cast<DLCLocalisationFile*>(file)->getStringTable()->ReloadStringTable();
    }
}
std::wstring DLCPack::getName()
{
    return m_name;
}
unsigned int DLCPack::GetPackId()
{
    return m_packId;
}
DLCFile* DLCPack::getSkinFile(const std::wstring& name)
{
    return getFile(DLCManager::eDLCType_Skin, name, true, true);
}
bool DLCPack::doesPackContainFile(DLCManager::EDLCType type, const std::wstring& name, bool searchParent,
                                  bool searchChildren)
{
    if (type == DLCManager::eDLCType_All)
    {
        int i;
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
        {
            if (doesPackContainFile(static_cast<DLCManager::EDLCType>(i), name, true, true))
                return true;
        }
        return false;
    }

    TlsSetValue(g_pathCompareTlsIndex, const_cast<std::wstring*>(&name));
    std::vector<DLCFile*>::iterator file = std::find_if(m_files[type].begin(), m_files[type].end(), pathCmp);
    if (file != m_files[type].end())
        return true;

    if (m_parentPack != NULL && searchParent && m_parentPack->doesPackContainFile(type, name, true, false))
        return true;

    if (searchChildren)
    {
        std::vector<DLCPack*>::iterator child;
        for (child = m_childPacks.begin(); child != m_childPacks.end(); ++child)
        {
            if ((*child)->doesPackContainFile(type, name, false, true))
                return true;
        }
    }
    return false;
}
bool DLCPack::doesPackContainSkin(const std::wstring& name)
{
    return doesPackContainFile(DLCManager::eDLCType_Skin, name, true, true);
}
bool DLCPack::hasPurchasedPack()
{
    if (m_licenseMask == 0)
    {
        unsigned int packId = GetPackId();
        if (packId < 0x1009)
        {
            if (packId != 0x1001 && packId != 0x1007)
                return gApp.mDLCManager.IsDlcPackTemporarilyFree(packId);
        }
        else if (packId > 0x100a && packId != 0x100f)
        {
            if (packId < 0x1014 || packId > 0x1015)
                return gApp.mDLCManager.IsDlcPackTemporarilyFree(packId);
        }
    }
    return true;
}
bool DLCPack::IsCorrupt()
{
    return m_isCorrupt;
}
void DLCPack::SetIsCorrupt(bool corrupt)
{
    m_isCorrupt = corrupt;
}
void DLCPack::SetDataPointer(unsigned char* data, bool ownsData)
{
    m_data = data;
    m_ownsData = ownsData;
}
unsigned int DLCPack::getLicenseMask()
{
    return m_licenseMask;
}
DLCFile* DLCPack::addFile(DLCManager::EDLCType type, const std::wstring& name)
{
    DLCFile* file = NULL;
    std::wstring path;

    switch (type)
    {
    case DLCManager::eDLCType_Skin:
    case DLCManager::eDLCType_Cape:
        path = replaceAll(name, L"\\", L"/");
        {
            std::vector<std::wstring> parts = stringSplit(path, L'/');
            path = parts.back();
        }
        if (type == DLCManager::eDLCType_Skin)
        {
            DLCSkinFile* skin = new DLCSkinFile(path);
            if (skin->isFiltered())
            {
                delete skin;
                return NULL;
            }
            file = skin;
        }
        else
        {
            file = new DLCCapeFile(path);
        }
        break;
    case DLCManager::eDLCType_Texture:
        file = new DLCTextureFile(name);
        break;
    case DLCManager::eDLCType_UIData:
        file = new DLCUIDataFile(name);
        break;
    case DLCManager::eDLCType_Localisation:
        file = new DLCLocalisationFile(name);
        break;
    case DLCManager::eDLCType_GameRules:
        file = new DLCGameRulesFile(name);
        break;
    case DLCManager::eDLCType_Audio:
        file = new DLCAudioFile(name);
        break;
    case DLCManager::eDLCType_ColourTable:
        file = new DLCColourTableFile(name);
        break;
    case DLCManager::eDLCType_GameRulesHeader:
        file = new DLCGameRulesHeader(name);
        break;
    case DLCManager::eDLCType_Models:
        file = new DLCModelsFile(name);
        break;
    case DLCManager::eDLCType_Behaviours:
        file = new DLCBehavioursFile(name);
        break;
    case DLCManager::eDLCType_Material:
        file = new DLCMaterialFile(name);
        break;
    case DLCManager::eDLCType_Pack:
    case DLCManager::eDLCType_TexturePack:
    case DLCManager::eDLCType_SkinPack:
        return NULL;
    default:
        return NULL;
    }

    if (file != NULL)
    {
        m_files[type].push_back(file);
        file->m_pPack = this;
    }
    return file;
}
#ifdef MINECRAFT_PC
void DLCPack::PcSetLauncherPack()
{
    m_pcLauncherPack = true;
}

#endif
void DLCPack::SetPackId(unsigned int id)
{
    m_packId = id;
}
void DLCPack::SetPackVersion(unsigned int version)
{
    m_packVersion = version;
}
void DLCPack::addParameter(DLCManager::EDLCParameterType type, const std::wstring& value)
{
    if (type == DLCManager::eDLCParameterType_DisplayName)
    {
        m_name = value;
    }
    else if (type == DLCManager::eDLCParameterType_PackID)
    {
        unsigned int packId = 0;
        std::wstringstream stream;
        stream.setf(std::ios_base::dec, std::ios_base::basefield);
        stream << value.c_str();
        stream >> packId;
#ifdef MINECRAFT_PC
        if (m_pcLauncherPack && !gApp.mDLCManager.PcPackIdFree(this, packId))
        {
            printf("[pc] dlc: %ls declares pack id %u, which is in use or over 24 bits; it keeps %u\n",
                   m_name.c_str(), packId, m_packId);
            fflush(stdout);
            return;
        }
#endif
        SetPackId(packId);
    }
    else if (type == DLCManager::eDLCParameterType_DataPath)
    {
        m_dataPath = value;
    }
    else if (type == DLCManager::eDLCParameterType_PackVersion)
    {
        unsigned int version = 0;
        std::wstringstream stream;
        stream.setf(std::ios_base::dec, std::ios_base::basefield);
        stream << value.c_str();
        stream >> version;
        SetPackVersion(version);
    }
    else
    {
        m_parameters[static_cast<int>(type)] = value;
    }
}
void DLCPack::setParentPack(DLCPack* parent)
{
    m_parentPack = parent;
}
void DLCPack::addChildPack(DLCPack* child)
{
    child->SetPackId((child->GetPackId() << 24) | m_packId);
    m_childPacks.push_back(child);
    child->setParentPack(this);
    child->m_name = m_name + child->getName();
}
std::wstring DLCPack::getFullDataPath()
{
    return m_dataPath;
}
DLCPack* DLCPack::GetParentPack()
{
    return m_parentPack;
}
unsigned int DLCPack::GetParentPackId()
{
    return m_parentPack->m_packId;
}
void DLCPack::SetDLCMountIndex(unsigned int index)
{
    m_dlcMountIndex = index;
}
unsigned int DLCPack::GetDLCMountIndex()
{
    DLCPack* pack = m_parentPack;
    if (pack != NULL)
    {
        DLCPack* parent;
        do
        {
            parent = pack;
            pack = parent->m_parentPack;
        } while (pack != NULL);
        return parent->m_dlcMountIndex;
    }
    return m_dlcMountIndex;
}
void DLCPack::SetDLCDeviceID(int id)
{
    m_dlcDeviceId = id;
}
std::vector<DLCPack*>& DLCPack::getChildPacks()
{
    return m_childPacks;
}
void DLCPack::AddCreditID(const std::wstring& value)
{
    m_creditIds.push_back(value);
}
void DLCPack::AddCreditString(const std::wstring& value)
{
    m_creditStrings.push_back(value);
}
void DLCPack::updateLicenseMask(unsigned int licenseMask)
{
    unsigned int packId = m_packId;
    if (packId < 0x40a)
    {
        if (packId == 0x208 || packId == 0x20a || packId == 0x405)
        {
            m_licenseMask = 1;
            return;
        }
        m_licenseMask = licenseMask;
        return;
    }
    if (packId == 0x40a || packId == 0x803 || (packId >= 0x805 && packId <= 0x806))
    {
        m_licenseMask = 1;
        return;
    }
    m_licenseMask = licenseMask;
}
int DLCPack::getFileIndexAt(DLCManager::EDLCType type, const std::wstring& name, bool& found)
{
    if (type == DLCManager::eDLCType_All)
        return 0;
    found = false;
    int index = 0;
    for (std::vector<DLCFile*>::iterator file = m_files[type].begin(); file != m_files[type].end(); ++file)
    {
        if (name == (*file)->getPath())
        {
            found = true;
            return index;
        }
        ++index;
    }
    if (!found)
    {
        for (std::vector<DLCPack*>::iterator child = m_childPacks.begin(); child != m_childPacks.end(); ++child)
        {
            int childIndex = (*child)->getFileIndexAt(type, name, found);
            if (found)
                return childIndex;
        }
    }
    return 0;
}
int DLCPack::getSkinCount()
{
    return getDLCItemsCount(DLCManager::eDLCType_Skin);
}
int DLCPack::getSkinIndexAt(const std::wstring& name, bool& found)
{
    return getFileIndexAt(DLCManager::eDLCType_Skin, name, found);
}
DLCFile* DLCPack::getSkinFile(unsigned int index)
{
    return getFile(DLCManager::eDLCType_Skin, index, true, true);
}
std::wstring DLCPack::getLocalisedDisplayName()
{
    DLCLocalisationFile* localisation = static_cast<DLCLocalisationFile*>(
        getFile(DLCManager::eDLCType_Localisation, L"localisation.loc", true, true));
    if (localisation == NULL)
        localisation = static_cast<DLCLocalisationFile*>(
            getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true));

    if (localisation != NULL)
    {
        StringTable* table = localisation->getStringTable();
        if (table != NULL)
        {
            std::wstring displayName = table->getString(L"IDS_DISPLAY_NAME");
            if (displayName == L"")
                return table->getString(L"title");
            return displayName;
        }
    }
    return getName();
}
