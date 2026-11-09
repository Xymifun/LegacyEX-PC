#ifndef GAME_CORE_DLCPACK_HPP
#define GAME_CORE_DLCPACK_HPP

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

#include "game/core/DLCManager.hpp"
#include "game/core/DLCFile.hpp"

class DLCLocalisationFile;
class StringTable;

class DLCPack
{
public:
    DLCPack(const std::wstring& name, unsigned int licenseMask);
    ~DLCPack();

    DLCFile* getFile(DLCManager::EDLCType type, unsigned int index, bool searchParent, bool searchChildren);
    unsigned int getDLCItemsCount(DLCManager::EDLCType type);
    DLCFile* getFile(DLCManager::EDLCType type, const std::wstring& name, bool searchParent, bool searchChildren);
    void UpdateLanguage();
    std::wstring getName();
    unsigned int GetPackId();
    DLCFile* getSkinFile(const std::wstring& name);
    bool doesPackContainFile(DLCManager::EDLCType type, const std::wstring& name, bool searchParent,
                             bool searchChildren);
    bool doesPackContainSkin(const std::wstring& name);
    bool hasPurchasedPack();
    bool IsCorrupt();
    void SetIsCorrupt(bool corrupt);
    void SetDataPointer(unsigned char* data, bool ownsData);
    unsigned int getLicenseMask();
    DLCFile* addFile(DLCManager::EDLCType type, const std::wstring& name);
    void SetPackId(unsigned int id);
#ifdef MINECRAFT_PC
    void PcSetLauncherPack();
#endif
    void SetPackVersion(unsigned int version);
    void addParameter(DLCManager::EDLCParameterType type, const std::wstring& value);
    void setParentPack(DLCPack* parent);
    void addChildPack(DLCPack* child);
    std::wstring getFullDataPath();
    unsigned int GetPackVersion();
    DLCPack* GetParentPack();
    unsigned int GetParentPackId();
    void SetDLCMountIndex(unsigned int index);
    unsigned int GetDLCMountIndex();
    void SetDLCDeviceID(int id);
    int GetDLCDeviceID();
    std::vector<DLCPack*>& getChildPacks();
    void AddCreditID(const std::wstring& value);
    void AddCreditString(const std::wstring& value);
    bool getParameterAsUInt(DLCManager::EDLCParameterType type, unsigned int& value);
    void updateLicenseMask(unsigned int licenseMask);
    unsigned int getPurchaseOfferId();
    int getFileIndexAt(DLCManager::EDLCType type, const std::wstring& name, bool& found);
    int getSkinCount();
    int getSkinIndexAt(const std::wstring& name, bool& found);
    DLCFile* getSkinFile(unsigned int index);
    std::wstring getLocalisedDisplayName();

private:
    friend bool pathCmp(DLCFile* file);

    std::vector<DLCFile*> m_files[DLCManager::eDLCType_NumTypes];
    std::vector<DLCPack*> m_childPacks;
    DLCPack* m_parentPack;

public:
    std::vector<std::wstring> m_creditIds;
    std::vector<std::wstring> m_creditStrings;

private:
    boost::unordered_map<int, std::wstring> m_parameters;
    std::wstring m_name;
    std::wstring m_dataPath;
    unsigned int m_licenseMask;
    unsigned int m_dlcMountIndex;
    int m_dlcDeviceId;
    unsigned int m_purchaseOfferId;
    bool m_isCorrupt;
    unsigned int m_packId;
#ifdef MINECRAFT_PC
    bool m_pcLauncherPack;
#endif
    unsigned int m_packVersion;
    unsigned char* m_data;
    bool m_ownsData;
};

#endif
