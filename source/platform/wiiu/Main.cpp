#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/C_AOC.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/Compression.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/client/render/Textures.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/core/LeaderboardManager.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiUFileSystemHelpers.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/FJ_WiiUDRC.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CriticalSection.hpp"

#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/memexpheap.h>
#include <coreinit/messagequeue.h>
#include <coreinit/thread.h>
#include <nn/ac/ac_cpp.h>
#include <nn/erreula/erreula_cpp.h>
#include <cstddef>
#include <cstdlib>
#include <cwchar>
#include <stdint.h>

extern "C" void OSBlockThreadsOnExit();
extern "C" int OSResumeThread(OSThread* thread);
extern "C" void OSInitMessageQueue(OSMessageQueue*, OSMessage*, int);
extern "C" int ProcUIInitEx(int (*saveCallback)(void*), void* context);
extern "C" int ProcUIRegisterCallback(int, unsigned int (*callback)(void*), void*, int);
extern "C" int ProcUIProcessMessages(int);
void DefineActions();
void RegisterAwardsWithProfileManager();
extern "C" void ProcUIDrawDoneRelease();
extern "C" void OSSavesDone_ReadyToRelease();
extern "C" void* memalign(std::size_t alignment, std::size_t size);
void ShutdownAndExit();
extern unsigned int CallbackAcquiredForeground(void*);
extern unsigned int CallbackReleaseForeground(void*);
extern unsigned int CallbackHomebuttonDenied(void*);
extern int altMainThread(int, void*);

extern CInput* g_pInput;
extern C4JStorage* gStorage;
extern C_4JProfile* gProfile;
extern Renderer* gRenderer;
extern CConsoleMinecraftApp gApp;
extern unsigned char s_altMainPending;
extern unsigned char s_altMainOKToRun;
extern unsigned char s_altMainExit;

namespace
{
struct MainStaticInit
{
    MainStaticInit()
    {
        if (gProfile == 0)
            gProfile = new CProfile();
    }
};
MainStaticInit s_mainStaticInit;
}

int SaveCallback(void*)
{
    CConsoleMinecraftApp::RequestToBackground();
    int ready = CConsoleMinecraftApp::ReadyToBackground();
    while (ready == 0)
    {
        gStorage->Tick();
        Sleep(10);
        ready = CConsoleMinecraftApp::ReadyToBackground();
    }
    OSSavesDone_ReadyToRelease();
    return 0;
}

void ReleaseCallback()
{
    CGameNetworkManager::sInstance.WiiU_ReleaseCallback();
    ProcUIDrawDoneRelease();
}

unsigned int CallbackHomebuttonDenied(void*)
{
    nn::erreula::HomeNixSignArg arg;
    nn::erreula::AppearHomeNixSign(arg);
    return 0;
}

OSMessageQueue gMainMessageQueue;
OSMessage gMainMessageBuffer;
OSFastMutex gMainMutex;
char* gHeapStart;
char* gHeapEnd;
char* gHeapTop;
static int s_drcIconDelay = 0x78;
static unsigned char s_trialTimerShown = 1;
static const unsigned int kStorageFullErrorCode = 1550100;
static unsigned char gMainThreadStack[0x20000];
static OSThread gMainThread;

int main()
{
    OSBlockThreadsOnExit();
    ShutdownManager::Initialise();
    CGameNetworkManager::sInstance.WiiU_SetBootFromFriendList();

    OSInitMessageQueue(&gMainMessageQueue, &gMainMessageBuffer, 1);
    InitializeCriticalSection(&gMainMutex);
    C4JThreadImpl::OSCreateThread4J(&gMainThread, altMainThread, 0, 0, gMainThreadStack + sizeof(gMainThreadStack),
                                    sizeof(gMainThreadStack), 0x11, 0);
    OSResumeThread(&gMainThread);
    C4JThreadImpl::SetMainThreadID();
    Compression::CreateNewThreadStorage();
    gRenderer->Initialise();
    g_pInput->Initialise(1, 0x3d, 0x17);

    DefineActions();
    g_pInput->SetJoypadMapArraySetupComplete();
    g_pInput->SetKeyRepeatRate(0.3f, 0.2f);
    g_pInput->SetDeadzoneAndMovementRange(0, 20000, 0x7fff);
    ProcUIInitEx(SaveCallback, 0);
    ProcUIRegisterCallback(0, CallbackAcquiredForeground, 0, 100);
    ProcUIRegisterCallback(1, CallbackReleaseForeground, 0, 100);
    ProcUIRegisterCallback(5, CallbackHomebuttonDenied, 0, 100);
    nn::ac::Initialize();
    WiiUInitialise();

    gApp.loadMediaArchive();
    gApp.InitialiseDLCInfo();
    if (gApp.ReadProductCodes() == 0)
        gApp.FatalLoadError(L"ReadProductCodes");

    switch (gApp.GetProductSKU())
    {
    case 0:
        gProfile->SetMinimumAge(7);
        break;
    case 1:
        gProfile->SetMinimumAge(10);
        break;
    case 2:
        gProfile->SetMinimumAge(0);
        break;
    }
    L10N::loadStringTable();
    g_pInput->SetCircleCrossSwapped(true);

    int uiWidth;
    int uiHeight;
    if (gRenderer->IsWidescreen())
    {
        uiWidth = 1280;
        uiHeight = 720;
    }
    else
    {
        uiWidth = 640;
        uiHeight = 480;
    }
    gConsoleUIController.init(uiWidth, uiHeight);
    gApp.CommerceInit();
    gApp.InitTime();
    static unsigned int s_profileDefaults[5];
    gProfile->Initialise(L"", L"", 0x22, 5, 4, s_profileDefaults, 0x1e60, &gApp.mGameDefinedDataChangedBitmask);
    gProfile->SetSignInChoicesCallback(CConsoleMinecraftApp::RequestSignInUIChoices);
    gProfile->SetOnAwardHandler(PopupToast, 0);
    RegisterAwardsWithProfileManager();

    bool noSpaceForSaveQuota;
    gStorage->Init(0, L10N::GetString(0xc0b26be6), "savegame.dat", 51000000,
                   CConsoleMinecraftApp::DisplaySavingMessageWiiU, &gApp, "", &noSpaceForSaveQuota);
    gApp.SetNoSpaceForSaveQuota(noSpaceForSaveQuota);
    gProfile->LoadAllMiiData();
    gProfile->SetSignInChangeCallback((CProfile::SignInChangeHandler)CMinecraftApp::SignInChangeCallback, &gApp);
    gProfile->SetProfileReadErrorCallback(CMinecraftApp::ProfileReadErrorCallback, &gApp);

    gStorage->InitialiseGenericData();
    gStorage->InitialiseProfileData(0x22, 5, 4, s_profileDefaults, 0x1e60, &gApp.mGameDefinedDataChangedBitmask);
    gStorage->SetDefaultOptionsCallback(CMinecraftApp::DefaultOptionsCallback, &gApp);
    gStorage->SetOptionsDataCallback(CMinecraftApp::OptionsDataCallback, &gApp);
    gStorage->SetOldProfileVersionCallback(GameSettings::OldProfileVersionCallback, &gApp);
    gStorage->SetGameSaveFolderPrefix("");
    gStorage->SetMaxSaves(99);
    arrayWithLength<unsigned char> thumbnail = gApp.getArchiveFile(L"DefaultSaveThumbnail64x64.png", false);
    gStorage->SetDefaultImages(0, 0, 0, 0, thumbnail.data, thumbnail.length);
    gStorage->SetIncompleteSaveCallback(CConsoleMinecraftApp::Callback_SaveGameIncomplete, &gApp);
    if (thumbnail.data != 0)
        delete[] thumbnail.data;
    gStorage->SetSaveTitle(L"Default Save");
    gStorage->SetOpenDirFn(OpenDir);
    gStorage->SetReadDirFn(ReadDir);
    gStorage->SetCloseDirFn(CloseDir);
    gStorage->SetDLCPackageRoot("DLCDrive");

    Tesselator::CreateNewThreadStorage(0x100000);
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    Level::enableLightingCache();
    Block::CreateNewThreadStorage();
    FireworksRecipe::CreateNewThreadStorage();
    LevelChunk::CreateNewThreadStorage();
    ElementStorage::staticCtor();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Main);

    Minecraft::main();
    Minecraft* minecraft = Minecraft::GetInstance();
    gApp.InitGameSettings();
    gStorage->ReadFromProfile(0, 0);
    gProfile->SetUpsellCallback(reinterpret_cast<CProfile::UpsellHandler>(CMinecraftApp::UpsellReturnedCallback),
                                &gApp);
    gApp.InitialiseTips();
    g_pLeaderboardManager->OpenSession();
    CGameNetworkManager::sInstance.Initialise();
    CGameNetworkManager::sInstance.SetLocalGame(true);
    if (noSpaceForSaveQuota)
        g_pInput->RequestErrorViewer(1, kStorageFullErrorCode,
                                     reinterpret_cast<C_4JInput::ErrorViewerCallback>(DeviceFullErrorReturn), 0);
    C_AOC::getInstance()->Init();

    float timingColour = 0.0f;
    while (true)
    {
        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Frame);
        if (s_altMainPending != 0)
        {
            s_altMainOKToRun = 1;
            Sleep(100);
        }
        EnterCriticalSection(&gMainMutex);
        LeaveCriticalSection(&gMainMutex);
        if (s_altMainExit != 0)
            return 0;
        int processResult = ProcUIProcessMessages(1);
        if (processResult == 3)
        {
            ShutdownAndExit();
            return 0;
        }
        if (processResult == 2)
            ReleaseCallback();

        gRenderer->StartFrame();
        gApp.UpdateTime();
        PIXBeginNamedEvent(timingColour, "Input manager tick");
        g_pInput->Tick();
        PIXEndNamedEvent();
        HandleDRCStatusChanges();
        PIXBeginNamedEvent(timingColour, "Profile manager tick");
        gProfile->Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Storage manager tick");
        gStorage->Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Render manager tick");
        gRenderer->Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Sentient tick");
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Network manager do work #1");
        CGameNetworkManager::sInstance.DoWork();
        PIXEndNamedEvent();

        if (gApp.GetDRCOutputEnabled() != 0 || gApp.GetDRCIconEnabled() != 0
            || !CGameNetworkManager::sInstance.IsInGameplay() || gApp.GetDRCCheckPlayerCountEnabled() == 0
            || gApp.GetLocalPlayerCount() != 1)
        {
            s_drcIconDelay = 0x77;
        }
        else if (s_drcIconDelay != 0)
        {
            --s_drcIconDelay;
        }
        if (s_drcIconDelay == 0)
            gApp.SetDRCIconEnabled(true);
        g_pLeaderboardManager->Tick();

        if (gApp.GetGameStarted() != 0)
        {
            minecraft->run_middle();
            gApp.SetAppPaused(CGameNetworkManager::sInstance.IsLocalGame()
                              && CGameNetworkManager::sInstance.GetPlayerCount() == 1
                              && gConsoleUIController.IsPauseMenuDisplayed(gProfile->GetPrimaryPad()));
        }
        else
        {
            minecraft->mSoundEngine->tick(NULL, 0.0f);
            minecraft->mTextures->tick(true, false, false);
            IntCache::Reset();
            if (gApp.GetReallyChangingSessionType() != 0)
                minecraft->tickAllConnections();
        }

        minecraft->mSoundEngine->playMusicTick();
        gApp.GetCommerceInstance()->tick();
        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_UITick);
        gConsoleUIController.tick();
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_UITick);
        gRenderer->ResetRenderState();
        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_UIRender);
        gConsoleUIController.render();
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_UIRender);
        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Frame);
        PIXBeginNamedEvent(timingColour, "Frame present");
        gRenderer->Present();
        PIXEndNamedEvent();
        gRenderer->Set_matrixDirty();
        gConsoleUIController.CheckMenuDisplayed();
        PIXBeginNamedEvent(timingColour, "Profile load check");
        if (gApp.mGameDefinedDataChangedBitmask != 0)
        {
            for (int player = 0; player < 4; ++player)
            {
                if ((gApp.mGameDefinedDataChangedBitmask & (1u << player)) != 0)
                {
                    unsigned char* data = gStorage->GetGameDefinedProfileData(player);
                    GameSettings::ClearGameSettingsChangedFlag(player);
                    GameSettings::ApplyGameSettingsChanged(player);
                    minecraft->mStats[player]->clear();
                    minecraft->mStats[player]->parse(data);
                }
            }
            gApp.mGameDefinedDataChangedBitmask = 0;
        }
        PIXEndNamedEvent();
        PIXBeginNamedEvent(timingColour, "Network manager do work #2");
        CGameNetworkManager::sInstance.DoWork();
        PIXEndNamedEvent();
        gApp.HandleXuiActions();
        if (!gProfile->IsFullVersion())
        {
            if (gApp.GetGameStarted() != 0)
            {
                if (gApp.IsAppPaused() != 0)
                    gApp.UpdateTrialPausedTimer();
                gConsoleUIController.UpdateTrialTimer(gProfile->GetPrimaryPad());
            }
        }
        else if (s_trialTimerShown != 0)
        {
            gConsoleUIController.ShowTrialTimer(false);
            s_trialTimerShown = 0;
        }
        Vec3::resetPool();
    }
}

static const int kHeapArenaAlignment = 4;
static const uintptr_t kHeapPageSize = 4096;
static const int kCacheLineAlignment = 64;

OSDynLoad_Error CoreUser_DynLoad_DefaultAlloc(int size, int alignment, void** result)
{
    void* block;

    if (result == 0)
    {
        return OS_DYNLOAD_INVALID_ALLOCATOR_PTR;
    }

    if (alignment < 0)
    {
        if (alignment > -4)
        {
            alignment = -4;
        }
    }
    else if (alignment < 4)
    {
        alignment = 4;
    }

    block = MEMAllocFromDefaultHeapEx(static_cast<unsigned int>(size), alignment);
    *result = block;
    return block != 0 ? OS_DYNLOAD_OK : OS_DYNLOAD_OUT_OF_MEMORY;
}

void CoreUser_DynLoad_DefaultFree(void* block)
{
    MEMFreeToDefaultHeap(block);
}

void* sMEMAllocFromDefaultHeap(unsigned int size)
{
    return malloc(size);
}

void* sMEMAllocFromDefaultHeapEx(unsigned int size, int alignment)
{
    unsigned int allocSize = size;
    if (alignment == kCacheLineAlignment)
        allocSize = (size + (kCacheLineAlignment - 1)) & ~(kCacheLineAlignment - 1);
    return memalign(alignment, allocSize);
}

void sMEMFreeToDefaultHeap(void* block)
{
    free(block);
}

extern "C" void __preinit_user(MEMHeapHandle* mem1, MEMHeapHandle* foreground, MEMHeapHandle* mem2)
{
    OSReport("Replace coreinit.rpl's default heap in the RPX.\n");
    unsigned int size = MEMGetAllocatableSizeForExpHeapEx(*mem2, kHeapArenaAlignment);
    char* arena = static_cast<char*>(MEMAllocFromDefaultHeapEx(size, kHeapArenaAlignment));
    MEMAllocFromDefaultHeap = sMEMAllocFromDefaultHeap;
    MEMAllocFromDefaultHeapEx = sMEMAllocFromDefaultHeapEx;
    MEMFreeToDefaultHeap = sMEMFreeToDefaultHeap;
    gHeapStart = arena;
    gHeapEnd = arena + size;
    gHeapTop = reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(arena) + (kHeapPageSize - 1))
                                       & ~(kHeapPageSize - 1));
    OSDynLoad_SetAllocator(CoreUser_DynLoad_DefaultAlloc, CoreUser_DynLoad_DefaultFree);
}
