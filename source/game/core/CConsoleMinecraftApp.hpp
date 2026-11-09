#ifndef GAME_CORE_CCONSOLEMINECRAFTAPP_HPP
#define GAME_CORE_CCONSOLEMINECRAFTAPP_HPP

#include "game/core/CMinecraftApp.hpp"
#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include <stdint.h>
#include <string>

class ConsoleUIController;
class cNintendoDLCInfo;
class cCommerceNintendoWiiU;
struct MiiDataEntry;

class CConsoleMinecraftApp : public CMinecraftApp
{
public:
    CConsoleMinecraftApp();
    bool IsAmericanSKU();
    cCommerceNintendoWiiU* GetCommerceInstance();
    static void RequestToBackground();
    static uint8_t ReadyToBackground();
    bool GetDRCIconEnabled();
    void SetDRCOuputEnabled(bool);
    bool GetDRCOutputEnabled();
    uint8_t GetDRCCheckPlayerCountEnabled();
    void FatalLoadError(const wchar_t*, ...);
    void InitialiseDLCInfo();
    uint32_t ReadProductCodes();
    uint32_t GetProductSKU();
    void CommerceInit();
    static int RequestSignInUIChoices(int);
    static int DisplaySavingMessageWiiU(void*, C4JStorage::ESavingMessage, int);
    void SetNoSpaceForSaveQuota(bool);
    static int Callback_SaveGameIncompleteMessageBoxReturned(void*, int, C4JStorage::EMessageResult);
    static void Callback_SaveGameIncomplete(void*, C4JStorage::ESaveIncompleteType);
    cNintendoDLCInfo* GetDLCInfoInstance();
    void HandleDLCLicenseChange(std::wstring&);
    void SetDRCIconEnabled(bool);
    void SetRichPresenceContext(int, int);
    uint8_t HasNoSpaceForSaveQuota();
    void SetDRCCheckPlayerCountEnabled(bool);
    bool getIgnoreMainMenuInputDuringInvite();
    void ReleaseSaveThumbnail();
    void ExitGame();
    void StoreLaunchData();
    void CaptureSaveThumbnail();
    void GetSaveThumbnail(unsigned char**, unsigned int*);
    void GetScreenshot(int, unsigned char**, unsigned int*);
    uint32_t LoadLocalTMSFile(wchar_t*);
    uint32_t LoadLocalTMSFile(wchar_t*, eFileExtensionType);
    void FreeLocalTMSFiles(eTMSFileType);
    uint32_t GetLocalTMSFileIndex(wchar_t*, bool, eFileExtensionType);
    virtual void TemporaryCreateGameStart();
    uint32_t UpgradeTrial();
    void RequestSignInUIWiiUReturnedSelected(MiiDataEntry*, int, int);
    uint8_t IsDrcPreventingSplitscreen();
    void setIgnoreMainMenuInputDuringInvite(bool);
    void LoadLocalDLCImages();
    void FreeLocalDLCImages();
    void ReadBannedList(int, eTMSAction, bool);
    uint32_t LoadLocalTMSFile(char*);
    uint32_t GetProductCode();
    const wchar_t* GetSaveFolderPrefix();
    const char* GetCommerceCategory();
    const char* GetTexturePacksCategoryID();
    const char* GetUpgradeKey();
    bool IsJapaneseSKU();
    bool IsEuropeanSKU();
    bool IsChinaSKU();
    void SystemServiceTick();
    void SaveDataDialogTick();
    void CommerceTick();
    uint32_t GetCommerceState();
    uint32_t Checkout(char*);
    uint32_t DownloadAlreadyPurchased(char*);
    bool DLCAlreadyPurchased(char*);
    static uint32_t CommerceInitCallback(void*, int);
    static uint32_t CommerceGetCategoriesCallback(void*, int);
    static uint32_t CommerceGetProductListCallback(void*, int);
    static uint32_t CommerceAddDetailedProductInfoCallback(void*, int);
    static uint32_t CommerceCheckoutCallback(void*, int);
    static uint32_t CheckoutSessionStartedCallback(void*, int);
    static uint32_t DownloadAlreadyPurchasedSessionStartedCallback(void*, int);
    static uint32_t UpgradeTrialSessionStartedCallback(void*, int);
    void ShowPatchAvailableError();
    void PatchAvailableDialogTick();

private:
    friend class ConsoleUIController;
    ImageFileBuffer m_saveThumbnail;
    unsigned char m_signInReserved[12];
    unsigned char m_signInResult;
    unsigned char m_dlcReserved[3];
    cNintendoDLCInfo* m_dlcInfo;
    bool m_ignoreMainMenuInputDuringInvite;
    unsigned char m_inviteReserved[3];
    unsigned char m_commerceInitialized;
    unsigned char m_commerceReserved[48];
    int m_commerceState;
    int m_commerceError;
    int m_commerceResult;
    int m_commerceProduct;
    int m_commerceCategory;
    int m_commerceInstanceState;
    unsigned char m_noSpaceForSaveQuota;
    bool m_drcIconEnabled;
    bool m_drcOutputEnabled;
    unsigned char m_drcCheckPlayerCountEnabled;
    char m_productCode[12];
    int m_productSKU;
    unsigned char m_instanceReserved[8];
    cCommerceNintendoWiiU* m_commerce;
};

extern CConsoleMinecraftApp theApp;

void RegisterAwardsWithProfileManager();
void ShutdownAndExit();

#endif
