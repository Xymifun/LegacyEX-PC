#ifndef GAME_CORE_DLCMANAGER_HPP
#define GAME_CORE_DLCMANAGER_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

class ArchiveFile;
class DLCFile;
class DLCPack;

class DLCManager
{
public:
    enum EDLCType
    {
        eDLCType_Skin,
        eDLCType_Cape,
        eDLCType_Texture,
        eDLCType_UIData,
        eDLCType_Pack,
        eDLCType_TexturePack,
        eDLCType_Localisation,
        eDLCType_GameRules,
        eDLCType_Audio,
        eDLCType_ColourTable,
        eDLCType_GameRulesHeader,
        eDLCType_SkinPack,
        eDLCType_Models,
        eDLCType_Behaviours,
        eDLCType_Material,
        eDLCType_NumTypes,
        eDLCType_All
    };

    enum EDLCParameterType
    {
        eDLCParameterType_Invalid = -1,
        eDLCParameterType_DisplayName = 0,
        eDLCParameterType_ThemeName,
        eDLCParameterType_LocKey,
        eDLCParameterType_Free,
        eDLCParameterType_Credit,
        eDLCParameterType_CapePath,
        eDLCParameterType_Box,
        eDLCParameterType_Anim,
        eDLCParameterType_GameFlags,
        eDLCParameterType_PackID,
        eDLCParameterType_NetherParticleColour,
        eDLCParameterType_EnchantTextColour,
        eDLCParameterType_EnchantTextFocusColour,
        eDLCParameterType_DataPath,
        eDLCParameterType_PackVersion,
        eDLCParameterType_DisplayNameID,
        eDLCParameterType_ThemeNameID,
        eDLCParameterType_CloudHeight,
        eDLCParameterType_CreditID,
        eDLCParameterType_DisableMipMap,
        eDLCParameterType_Offset,
        eDLCParameterType_XMLVersion,
        eDLCParameterType_NumTypes
    };

    struct FreePackPeriod
    {
        long long startTime;
        long long endTime;
    };

    DLCManager();
    ~DLCManager();

    static EDLCParameterType getParameterType(const std::wstring& name);

    int getPackCount(EDLCType type);
    void addPack(DLCPack* pPack);
    void removePack(DLCPack* pPack);
    void removeAllPacks();
    void LanguageChanged();

    DLCPack* getPack(const std::wstring& name);
    DLCPack* getPackFromPackID(int packID);
#ifdef MINECRAFT_PC
    bool PcPackIdFree(DLCPack* pack, unsigned int id);
    void PcGiveLauncherPackId(DLCPack* pack);
#endif
    DLCPack* getPack(unsigned int index, EDLCType type);
    unsigned int getPackIndex(DLCPack* pPack, bool& bFound, EDLCType type);

    DLCFile* getSkinFile(const std::wstring& fileName);
    DLCFile* getSkinFile(int skinIndex, bool& bIsDefaultSkin, bool& bHasPurchasedPack);
    unsigned int getPackIndexContainingSkin(const std::wstring& fileName, bool& bFound);
    DLCPack* getPackContainingSkin(const std::wstring& fileName);

    bool IsDlcPackTemporarilyFree(int packID);
    bool IsMiniGameTemporarilyFree(int miniGameID);
    void UpdateTemporarilyFreePacks(ArchiveFile* pArchiveFile);

    int checkForCorruptDLCAndAlert(bool bShowMessage);

    bool processDLCDataFile(unsigned int& numItemsAdded, unsigned char* pData, unsigned int size, DLCPack* pPack,
                            bool bTakeDataOwnership);
    bool readDLCDataFile(unsigned int& numItemsAdded, const std::string& fileName, DLCPack* pPack, unsigned int packID,
                         bool bTakeDataOwnership);
    bool readDLCDataFile(unsigned int& numItemsAdded, const std::wstring& fileName, DLCPack* pPack, unsigned int packID,
                         bool bTakeDataOwnership);

    unsigned int retrievePackID(unsigned char* pData, unsigned int size);
    unsigned int retrievePackIDFromDLCDataFile(const std::string& fileName);

private:
    friend class Minecraft;
    friend class CMinecraftApp;

    std::vector<DLCPack*> m_packs;
    bool m_bCorruptDLCFound;
    int m_numCorruptPacks;
    boost::unordered_map<int, FreePackPeriod> m_freeDLCPackPeriods;
    boost::unordered_map<int, FreePackPeriod> m_freeMiniGamePeriods;
};

#endif
