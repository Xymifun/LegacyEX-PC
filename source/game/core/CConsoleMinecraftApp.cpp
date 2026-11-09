#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/BigEndian.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/ui/UIScene_AchievementsMenu.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Platform.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include <stdlib.h>
#include <string.h>
#include <sysapp/launch.h>
#if defined(RECON_NATIVE)
#include <stdarg.h>
#include <wchar.h>
#endif

extern "C" void OSReport(const char*, ...);
extern "C" void ProcUIShutdown();

extern void altMain();
extern ConsoleUIController gConsoleUIController;
extern C4JStorage* gStorage;

static unsigned char s_requestBackground = 0;
static unsigned char s_signInRequested = 0;

unsigned char s_altMainPending = 0;
unsigned char s_altMainOKToRun = 0;
unsigned char s_altMainExit = 0;

void setAltMainOKToRun()
{
    s_altMainOKToRun = 1;
}

void RegisterAwardsWithProfileManager()
{
    for (int i = 0; i < UIScene_AchievementsMenu::DISPLAY_COUNT; ++i)
    {
        const AchievementInfo* info = UIScene_AchievementsMenu::GetInfo(UIScene_AchievementsMenu::sDisplayList[i]);
        gProfile->RegisterAward(info->id, info->registrationId, eAwardType_None, info->title, info->description);
    }

    gProfile->RichPresenceInit(-1, -1);
    gProfile->SetRichPresenceGetStringCallback(GetRichPresenceStringFromId);
    gProfile->RichPresenceRegisterContext(0, 0x0d21c173);

    gProfile->RichPresenceRegisterPresenceString(0, 0x0d371d4e);
    gProfile->RichPresenceRegisterPresenceString(1, 0xb0e2d942);
    gProfile->RichPresenceRegisterPresenceString(2, 0xeb1a4a80);
    gProfile->RichPresenceRegisterPresenceString(3, 0x7703ee79);
    gProfile->RichPresenceRegisterPresenceString(4, 0x32d9dd56);
    gProfile->RichPresenceRegisterPresenceString(5, 0x28b67b1f);
    gProfile->RichPresenceRegisterContextString(0, 0, 0x2292bf7f);
    gProfile->RichPresenceRegisterContextString(0, 1, 0x01e201d3);
    gProfile->RichPresenceRegisterContextString(0, 2, 0x05c4c95e);
    gProfile->RichPresenceRegisterContextString(0, 3, 0xb231c7f5);
    gProfile->RichPresenceRegisterContextString(0, 4, 0xc5036f69);
    gProfile->RichPresenceRegisterContextString(0, 5, 0x074e6d4f);
    gProfile->RichPresenceRegisterContextString(0, 6, 0x711f8b99);
    gProfile->RichPresenceRegisterContextString(0, 7, 0x3a2982d5);
    gProfile->RichPresenceRegisterContextString(0, 8, 0xa6dd54da);
    gProfile->RichPresenceRegisterContextString(0, 9, 0xbe2f93d1);
    gProfile->RichPresenceRegisterContextString(0, 10, 0x559d1b96);
    gProfile->RichPresenceRegisterContextString(0, 11, 0x123e6b5f);
    gProfile->RichPresenceRegisterContextString(0, 12, 0x59c6e2b1);
    gProfile->RichPresenceRegisterContextString(0, 13, 0xddd337be);
}

void ShutdownAndExit()
{
    gRenderer->WiiUWaitDrawDone();
    ShutdownManager::MainThreadHandleShutdown();
    ProcUIShutdown();
    _Exit(0);
}

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
    memset(m_productCode, 0, sizeof(m_productCode) + sizeof(m_productSKU));
    m_noSpaceForSaveQuota = false;
    m_drcCheckPlayerCountEnabled = true;
    m_signInResult = 0;
    m_drcOutputEnabled = true;
    m_drcIconEnabled = false;
}
bool CConsoleMinecraftApp::IsAmericanSKU()
{
    return m_productSKU == 1;
}
cCommerceNintendoWiiU* CConsoleMinecraftApp::GetCommerceInstance()
{
    return m_commerce;
}
void CConsoleMinecraftApp::RequestToBackground()
{
    s_requestBackground = 1;
}
uint8_t CConsoleMinecraftApp::ReadyToBackground()
{
    return (uint8_t)(s_requestBackground ^ 1);
}
bool CConsoleMinecraftApp::GetDRCIconEnabled()
{
    return m_drcIconEnabled;
}
void CConsoleMinecraftApp::SetDRCOuputEnabled(bool value)
{
    m_drcOutputEnabled = value;
}
bool CConsoleMinecraftApp::GetDRCOutputEnabled()
{
    return m_drcOutputEnabled;
}
uint8_t CConsoleMinecraftApp::GetDRCCheckPlayerCountEnabled()
{
    return m_drcCheckPlayerCountEnabled;
}
#if defined(RECON_NATIVE)
extern "C" void native_report_stack(const char* tag);
#endif
void CConsoleMinecraftApp::FatalLoadError(const wchar_t* message, ...)
{
#if defined(RECON_NATIVE)
    {
        wchar_t text[512];
        va_list args;
        va_start(args, message);
        vswprintf(text, sizeof(text) / sizeof(text[0]), message, args);
        va_end(args);
        OSReport("[native-fatal-load] CConsoleMinecraftApp::FatalLoadError caller=%p %ls\n",
                 __builtin_return_address(0), text);
        native_report_stack("native-fatal-load");
    }
#endif
    gRenderer->InitialiseContext();
    unsigned int errorCode = GetLastFileErrorCodeForViewer();
    g_pInput->RequestErrorViewer(1, errorCode, NULL, NULL);
    altMain();
    for (;;)
    {
        if (s_altMainPending != 0)
            s_altMainOKToRun = 1;
        Sleep(10);
        if (s_altMainExit != 0 && C4JThread::isMainThread())
            ShutdownAndExit();
    }
}
void CConsoleMinecraftApp::InitialiseDLCInfo()
{
    m_dlcInfo = new cNintendoDLCInfo;
}
uint32_t CConsoleMinecraftApp::ReadProductCodes()
{
    unsigned int bytesRead = 0;
    int handle = CreateFileA("WiiU/WiiUProductCodes.bin", 0x80000000U, 0, 0, 3, 0x80, 0);
    if (handle == -1)
        return 0;

    unsigned int fileSize = 0;
    if (GetFileSize(handle, &fileSize) != 0)
    {
        if (ReadFile(handle, m_productCode, 9, &bytesRead, 0) == 0)
            return 0;

        unsigned int dlcCount;
        if (ReadFile(handle, &dlcCount, 4, &bytesRead, 0) == 0)
            return 0;
        dlcCount = BigEndian::ToHost(dlcCount);

        for (unsigned int index = 0; index < dlcCount; ++index)
        {
            DLC_INFO* dlcInfo = new DLC_INFO;
            memset(dlcInfo, 0, sizeof(DLC_INFO));

            unsigned int stringLength;
            if (ReadFile(handle, &dlcInfo->ContentType, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->PackType, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->CatalogIndex, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &stringLength, 4, &bytesRead, 0) == 0)
                return 0;
            stringLength = BigEndian::ToHost(stringLength);
            if (ReadFile(handle, dlcInfo->PackName, stringLength, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &stringLength, 4, &bytesRead, 0) == 0)
                return 0;
            stringLength = BigEndian::ToHost(stringLength);
            if (ReadFile(handle, dlcInfo->BannerName, stringLength, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->PackID, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->SortIndex, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, dlcInfo->RomName, 5, &bytesRead, 0) == 0)
                return 0;
            dlcInfo->ContentType = BigEndian::ToHost(dlcInfo->ContentType);
            dlcInfo->PackType = BigEndian::ToHost(dlcInfo->PackType);
            dlcInfo->CatalogIndex = BigEndian::ToHost(dlcInfo->CatalogIndex);
            dlcInfo->PackID = BigEndian::ToHost(dlcInfo->PackID);
            dlcInfo->SortIndex = BigEndian::ToHost(dlcInfo->SortIndex);

            m_dlcInfo->AddDLCInfo(dlcInfo);
        }
        CloseHandle(handle);
    }

    if (strcmp(m_productCode, "AUMP01D75") == 0)
        m_productSKU = 0;
    else if (strcmp(m_productCode, "AUME01D9D") == 0)
        m_productSKU = 1;
    else if (strcmp(m_productCode, "AUMJ01DBE") == 0)
        m_productSKU = 2;
    return 1;
}
uint32_t CConsoleMinecraftApp::GetProductSKU()
{
    return (uint32_t)m_productSKU;
}
void CConsoleMinecraftApp::CommerceInit()
{
    m_commerceInitialized = 0;
    m_commerce = new cCommerceNintendoWiiU;
    m_commerceResult = 0;
    m_commerceProduct = 0;
    m_commerceCategory = 0;
    m_commerceError = 0;
    m_commerceState = 0;
    m_commerceInstanceState = 0;
    memset(m_commerceReserved, 0, sizeof(m_commerceReserved));
}
int CConsoleMinecraftApp::RequestSignInUIChoices(int playerId)
{
    gConsoleUIController.NavigateToScene(playerId, EUIScene_AvatarSelectMenu, NULL, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
    s_signInRequested = 1;
    return 0;
}
int CConsoleMinecraftApp::DisplaySavingMessageWiiU(void*, C4JStorage::ESavingMessage message, int playerId)
{
    if (s_requestBackground != 0 && message == C4JStorage::ESavingMessage_0)
        s_requestBackground = 0;
    gConsoleUIController.ShowSavingMessage((unsigned int)playerId, message);
    return 0;
}
void CConsoleMinecraftApp::SetNoSpaceForSaveQuota(bool value)
{
    m_noSpaceForSaveQuota = value;
}
int CConsoleMinecraftApp::Callback_SaveGameIncompleteMessageBoxReturned(void* context, int playerId,
                                                                        C4JStorage::EMessageResult result)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (static_cast<unsigned int>(result) < C4JStorage::EMessageResult_Cancelled)
        return 0;
    if (result != C4JStorage::EMessageResult_Cancelled)
    {
        if (static_cast<unsigned int>(result) < C4JStorage::EMessageResult_Decline)
        {
            app->LeaveSaveNotificationSection();
            gStorage->ContinueIncompleteOperation();
            return 0;
        }
        if (result != C4JStorage::EMessageResult_Decline)
        {
            if (result != C4JStorage::EMessageResult_ThirdOption)
                return 0;
            gConsoleUIController.NavigateToScene(playerId, EUIScene_InGameSaveManagementMenu, NULL, EUILayer_Alert,
                                                 EUIGroup_Fullscreen);
            return 0;
        }
    }
    app->LeaveSaveNotificationSection();
    gStorage->CancelIncompleteOperation();
    return 0;
}
void CConsoleMinecraftApp::Callback_SaveGameIncomplete(void* context, C4JStorage::ESaveIncompleteType type)
{
    if (type == C4JStorage::ESaveIncompleteType_1 || type == C4JStorage::ESaveIncompleteType_2)
    {
        CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
        app->EnterSaveNotificationSection();
        int messageParameters[] = {(int)IDS_SAVE_INCOMPLETE_RETRY_SAVING, (int)0xce9c596c,
                                   (int)IDS_SAVE_INCOMPLETE_DELETE_SAVES};
        if (gConsoleUIController.RequestErrorMessage(IDS_SAVE_INCOMPLETE_TITLE, 0x92727461, messageParameters, 3, 0xff,
                                                     Callback_SaveGameIncompleteMessageBoxReturned, context, NULL, true)
            != 1)
            return;
    }
    Callback_SaveGameIncompleteMessageBoxReturned(context, 0, C4JStorage::EMessageResult_Cancelled);
}
cNintendoDLCInfo* CConsoleMinecraftApp::GetDLCInfoInstance()
{
    return m_dlcInfo;
}
void CConsoleMinecraftApp::HandleDLCLicenseChange(std::wstring& packName)
{
    DLCPack* pack = gApp.mDLCManager.getPack(packName);
    if (pack != NULL)
        pack->updateLicenseMask(1);
}
void CConsoleMinecraftApp::SetDRCIconEnabled(bool value)
{
    m_drcIconEnabled = value;
}
void CConsoleMinecraftApp::SetRichPresenceContext(int context, int value)
{
    gProfile->SetRichPresenceContextValue(context, 0, value);
}
uint8_t CConsoleMinecraftApp::HasNoSpaceForSaveQuota()
{
    return m_noSpaceForSaveQuota;
}
void CConsoleMinecraftApp::SetDRCCheckPlayerCountEnabled(bool value)
{
    m_drcCheckPlayerCountEnabled = value;
}
bool CConsoleMinecraftApp::getIgnoreMainMenuInputDuringInvite()
{
    return m_ignoreMainMenuInputDuringInvite;
}
void CConsoleMinecraftApp::ExitGame()
{
    SYSLaunchMenu();
}

void CConsoleMinecraftApp::ReleaseSaveThumbnail()
{
    if (m_saveThumbnail.mData != NULL)
    {
        free(m_saveThumbnail.mData);
        m_saveThumbnail.mData = NULL;
    }
}
void CConsoleMinecraftApp::StoreLaunchData()
{
}
void CConsoleMinecraftApp::CaptureSaveThumbnail()
{
    gRenderer->CaptureThumbnail(&m_saveThumbnail);
}
void CConsoleMinecraftApp::GetSaveThumbnail(unsigned char** data, unsigned int* size)
{
    if (m_saveThumbnail.mData != NULL)
    {
        if (data != NULL)
        {
            *data = new unsigned char[m_saveThumbnail.mSize];
            *size = m_saveThumbnail.mSize;
            memcpy(*data, m_saveThumbnail.mData, *size);
        }
        free(m_saveThumbnail.mData);
        m_saveThumbnail.mData = NULL;
    }
    else if (data != NULL)
    {
        unsigned char* thumbnail;
        unsigned int thumbnailSize;
        gStorage->GetDefaultSaveThumbnail(&thumbnail, &thumbnailSize);
        *data = new unsigned char[thumbnailSize];
        *size = thumbnailSize;
        memcpy(*data, thumbnail, thumbnailSize);
    }
}
void CConsoleMinecraftApp::GetScreenshot(int, unsigned char**, unsigned int*)
{
}
uint32_t CConsoleMinecraftApp::LoadLocalTMSFile(wchar_t*)
{
    return 0xffffffffU;
}
uint32_t CConsoleMinecraftApp::LoadLocalTMSFile(wchar_t*, eFileExtensionType)
{
    return 0xffffffffU;
}
void CConsoleMinecraftApp::FreeLocalTMSFiles(eTMSFileType)
{
}
uint32_t CConsoleMinecraftApp::GetLocalTMSFileIndex(wchar_t*, bool, eFileExtensionType)
{
    return 0xffffffffU;
}
void CConsoleMinecraftApp::TemporaryCreateGameStart()
{
}
uint32_t CConsoleMinecraftApp::UpgradeTrial()
{
    if (m_commerceState == 1)
    {
        m_commerceState = 20;
        return 1;
    }
    if (m_commerceState == 2)
    {
        int messageParameter = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(0xa561a8df, 0x84f3e8d2, &messageParameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return 1;
    }
    return 0;
}
void CConsoleMinecraftApp::RequestSignInUIWiiUReturnedSelected(MiiDataEntry* mii, int choice, int pad)
{
    if (mii == NULL)
    {
        gProfile->SetSignInChoicesComplete(false, choice, pad);
    }
    else
    {
        gProfile->SignInUser(mii, pad);
        gProfile->SetSignInChoicesComplete(true, choice, pad);
    }
    m_signInResult = 0;
}
uint8_t CConsoleMinecraftApp::IsDrcPreventingSplitscreen()
{
#ifdef MINECRAFT_PC
    return 0;
#else
    return m_drcOutputEnabled;
#endif
}
void CConsoleMinecraftApp::setIgnoreMainMenuInputDuringInvite(bool value)
{
    m_ignoreMainMenuInputDuringInvite = value;
}
void CConsoleMinecraftApp::LoadLocalDLCImages()
{
    m_dlcInfo->LoadLocalDLCImages();
}
void CConsoleMinecraftApp::FreeLocalDLCImages()
{
    m_dlcInfo->FreeLocalDLCImages();
}
void CConsoleMinecraftApp::ReadBannedList(int, eTMSAction, bool)
{
}
