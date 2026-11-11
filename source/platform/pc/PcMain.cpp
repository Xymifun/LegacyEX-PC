#include "platform/pc/PcAccount.hpp"
#include "platform/pc/PcAudio.hpp"
#include "platform/pc/PcContent.hpp"
#include "platform/pc/PcCrashHandler.hpp"
#include "platform/pc/PcDebugOverlay.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcMods.hpp"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcVoice.hpp"
#include "platform/pc/PcKeyBindings.hpp"
#include "platform/pc/PcChat.hpp"
#include "platform/pc/PcMultiplayerMenu.hpp"
#include "platform/pc/PcLanTransport.hpp"
#include "platform/pc/PcMusicTest.hpp"
#include "platform/pc/PcDebug.hpp"
#include "platform/pc/PcOnlineConnection.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"
#include "platform/pc/PcProfiler.hpp"
#include "platform/pc/PcScripts.hpp"
#include "platform/pc/PcWorldLauncher.hpp"

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/C_AOC.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/FoodData.hpp"
#include "game/item/Inventory.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Compression.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/L10N.hpp"
#include "game/core/LeaderboardManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiUFileSystemHelpers.hpp"

#include <new>
#include <stdio.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>

void DefineActions();
void RegisterAwardsWithProfileManager();

extern CInput* g_pInput;
extern C4JStorage* gStorage;
extern C_4JProfile* gProfile;
extern CConsoleMinecraftApp gApp;

#ifdef _WIN32
extern "C" void PcStartupLog(const char* text);
extern char* gHeapStart;
extern char* gHeapEnd;
#endif

namespace
{
const unsigned int kStorageFullErrorCode = 1550100;

struct ScriptedKey
{
    std::string name;
    int key;
    bool mouse;
    int wheel;
    double start;
    double end;
    bool pressed;
};

struct ScriptedPad
{
    unsigned int buttons;
    double start;
    double end;
};

struct ScriptedCursor
{
    float x;
    float y;
    double time;
    bool applied;
};

struct ScriptedShot
{
    double time;
    std::string path;
    bool taken;
};

struct ScriptedText
{
    double time;
    std::string text;
    bool typed;
};

struct LaunchOptions
{
    LaunchOptions() : seconds(-1), serverDistance(0), profile(false), profileSlowFrame(50.0), profileSyncGpu(false) {}
    std::string connectAddress;
    int seconds;
    int serverDistance;
    bool profile;
    double profileSlowFrame;
    bool profileSyncGpu;
    std::string screenshot;
    std::vector<ScriptedKey> keys;
    std::vector<ScriptedPad> pads;
    std::vector<ScriptedCursor> cursors;
    std::vector<ScriptedShot> shots;
    std::vector<ScriptedText> texts;
};

struct KeyName
{
    const char* name;
    int key;
    bool mouse;
    int wheel;
};

const KeyName kKeyNames[] = {{"space", 0x20, false},  {"enter", 0x0D, false},    {"escape", 0x1B, false},
                             {"backspace", 0x08, false}, {"tab", 0x09, false},   {"shift", 0x10, false},
                             {"left", 0x25, false},   {"up", 0x26, false},       {"right", 0x27, false},
                             {"down", 0x28, false},   {"f5", 0x74, false},       {"f2", 0x71, false},       {"f6", 0x75, false},       {"f7", 0x76, false},       {"f8", 0x77, false},       {"f3", 0x72, false},
                             {"control", 0x11, false}, {"pageup", 0x21, false}, {"pagedown", 0x22, false}, {"mouse1", 0, true},
                             {"mouse2", 1, true},     {"wheelup", 0, false, 1},  {"wheeldown", 0, false, -1},
                             {"home", 0x24, false},   {"end", 0x23, false},      {"delete", 0x2E, false}};

bool ParseScriptedKey(const char* s, ScriptedKey* key)
{
    char name[32];
    double start = 0.0;
    double end = 0.0;
    if (sscanf(s, "%31[^:]:%lf:%lf", name, &start, &end) != 3)
        return false;
    key->name = name;
    key->start = start;
    key->end = end;
    key->pressed = false;
    key->mouse = false;
    key->wheel = 0;
    for (unsigned int i = 0; i < sizeof(kKeyNames) / sizeof(kKeyNames[0]); ++i)
    {
        if (strcmp(name, kKeyNames[i].name) == 0)
        {
            key->key = kKeyNames[i].key;
            key->mouse = kKeyNames[i].mouse;
            key->wheel = kKeyNames[i].wheel;
            return true;
        }
    }
    if (strlen(name) == 1 && name[0] >= 'a' && name[0] <= 'z')
    {
        key->key = name[0] - 'a' + 'A';
        return true;
    }
    if (strlen(name) == 1 && name[0] >= '0' && name[0] <= '9')
    {
        key->key = name[0];
        return true;
    }
    return false;
}

bool ParseScriptedPad(const char* s, ScriptedPad* pad)
{
    static const struct
    {
        const char* name;
        unsigned int buttons;
    } kPadNames[] = {{"south", ePcOsGamepad_South}, {"east", ePcOsGamepad_East},
                     {"west", ePcOsGamepad_West},   {"north", ePcOsGamepad_North},
                     {"lb", ePcOsGamepad_LeftShoulder}, {"rb", ePcOsGamepad_RightShoulder},
                     {"start", ePcOsGamepad_Start}, {"back", ePcOsGamepad_Back},
                     {"up", ePcOsGamepad_Up},       {"down", ePcOsGamepad_Down},
                     {"left", ePcOsGamepad_Left},   {"right", ePcOsGamepad_Right}};
    char name[32];
    if (sscanf(s, "%31[^:]:%lf:%lf", name, &pad->start, &pad->end) != 3)
        return false;
    for (unsigned int i = 0; i < sizeof(kPadNames) / sizeof(kPadNames[0]); ++i)
    {
        if (strcmp(name, kPadNames[i].name) == 0)
        {
            pad->buttons = kPadNames[i].buttons;
            return true;
        }
    }
    return false;
}

void RunScriptedInput(LaunchOptions& opts, double t)
{
    if (!opts.pads.empty())
    {
        unsigned int buttons = 0;
        for (unsigned int i = 0; i < opts.pads.size(); ++i)
        {
            if (t >= opts.pads[i].start && t < opts.pads[i].end)
                buttons |= opts.pads[i].buttons;
        }
        PcInputSetScriptedPad(buttons);
    }
    for (unsigned int i = 0; i < opts.texts.size(); ++i)
    {
        ScriptedText& s = opts.texts[i];
        if (s.typed || t < s.time)
            continue;
        s.typed = true;
        for (size_t character = 0; character < s.text.size(); ++character)
        {
            PcOsWindowEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.type = ePcOsWindowEvent_Text;
            ev.character = (unsigned char)s.text[character];
            PcInputHandleEvent(ev);
        }
    }
    for (unsigned int i = 0; i < opts.cursors.size(); ++i)
    {
        ScriptedCursor& cursor = opts.cursors[i];
        if (!cursor.applied && t >= cursor.time)
        {
            cursor.applied = true;
            PcInputSetScriptedCursor(cursor.x, cursor.y);
        }
    }
    for (unsigned int i = 0; i < opts.keys.size(); ++i)
    {
        ScriptedKey& key = opts.keys[i];
        bool down = t >= key.start && t < key.end;
        if (down == key.pressed)
            continue;
        key.pressed = down;
        PcOsWindowEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = key.mouse ? ePcOsWindowEvent_MouseButton : ePcOsWindowEvent_Key;
        ev.key = key.key;
        ev.down = down;
        if (key.wheel != 0)
        {
            if (!down)
                continue;
            ev.type = ePcOsWindowEvent_MouseWheel;
            ev.delta = key.wheel;
        }
        PcInputHandleEvent(ev);
    }
    for (unsigned int i = 0; i < opts.shots.size(); ++i)
    {
        ScriptedShot& shot = opts.shots[i];
        if (!shot.taken && t >= shot.time)
        {
            shot.taken = true;
            PcRequestScreenshot(shot.path.c_str());
        }
    }
}

bool FileExists(const std::string& path)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return false;
    fclose(fp);
    return true;
}

void FindContentRoot()
{
    static const char* const kContentRoots[] = {"content", "assets", "../../assets"};
    for (unsigned int i = 0; i < sizeof(kContentRoots) / sizeof(kContentRoots[0]); ++i)
    {
        if (FileExists(std::string(kContentRoots[i]) + "/WiiU/WiiUProductCodes.bin"))
        {
            PcSetContentRoot(kContentRoots[i]);
            return;
        }
    }
}

void ApplyLauncherName()
{
    FILE* fp = fopen("username.txt", "rb");
    if (fp == 0)
    {
        const char* account = getenv("USERNAME");
        if (account == 0 || account[0] == 0)
            account = getenv("USER");
        if (account != 0 && account[0] != 0)
            PcSetPlayerName(account);
        return;
    }
    char name[64];
    size_t len = fread(name, 1, sizeof(name) - 1, fp);
    fclose(fp);
    name[len] = 0;
    while (len > 0 && (name[len - 1] == '\n' || name[len - 1] == '\r' || name[len - 1] == ' '))
        name[--len] = 0;
    if (len > 0)
        PcSetPlayerName(name);
}

bool noHints = false;

bool ParseArguments(int argc, char** argv, LaunchOptions* opts)
{
    FindContentRoot();
    bool ownOptions = false;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--no-lan") == 0 || strcmp(argv[i], "--terminal") == 0)
            continue;
        if (strcmp(argv[i], "--connect") == 0)
        {
            ++i;
            continue;
        }
        ownOptions = ownOptions || strncmp(argv[i], "--", 2) == 0;
    }
    if (!ownOptions)
        ApplyLauncherName();
    std::string launcherIp;
    std::string launcherPort;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-' && argv[i][1] != '-')
        {
            if (strcmp(argv[i], "-name") == 0 && i + 1 < argc)
                PcSetPlayerName(argv[++i]);
            else if (strcmp(argv[i], "-ip") == 0 && i + 1 < argc)
                launcherIp = argv[++i];
            else if (strcmp(argv[i], "-port") == 0 && i + 1 < argc)
                launcherPort = argv[++i];
            else if (strcmp(argv[i], "-token") == 0 && i + 1 < argc)
                ++i;
            else if (strcmp(argv[i], "-quitondisconnect") == 0)
                PcRequestQuitOnDisconnect();
            else
                printf("[pc] ignoring launcher option %s\n", argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--content") == 0 && i + 1 < argc)
            PcSetContentRoot(argv[++i]);
        else if (strcmp(argv[i], "--save") == 0 && i + 1 < argc)
            PcSetSaveRoot(argv[++i]);
        else if (strcmp(argv[i], "--mods") == 0 && i + 1 < argc)
            PcModsSetDirectory(argv[++i]);
        else if (strcmp(argv[i], "--no-mods") == 0)
            PcModsSetEnabled(false);
        else if (strcmp(argv[i], "--no-native-mods") == 0)
            PcNativeModsSetEnabled(false);
        else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc)
            PcSetPlayerName(argv[++i]);
        else if (strcmp(argv[i], "--identity") == 0 && i + 1 < argc)
            PcSetIdentityDirectory(argv[++i]);
        else if (strcmp(argv[i], "--seconds") == 0 && i + 1 < argc)
            opts->seconds = atoi(argv[++i]);
        else if (strcmp(argv[i], "--world") == 0 && i + 1 < argc)
            PcRequestWorld(argv[++i]);
        else if (strcmp(argv[i], "--music-test") == 0)
            PcMusicTestStart();
        else if (strcmp(argv[i], "--creative") == 0)
            PcRequestWorldCreative();
        else if (strcmp(argv[i], "--friends-only") == 0)
            PcRequestWorldVisibility(1);
        else if (strcmp(argv[i], "--friends-of-friends") == 0)
            PcRequestWorldVisibility(2);
        else if (strcmp(argv[i], "--no-hints") == 0)
            noHints = true;
        else if (strcmp(argv[i], "--debug") == 0)
            PcSetDebugMode(true);
        else if (strcmp(argv[i], "--debug-free-crafting") == 0)
            PcSetDebugFreeCrafting(true);
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
            PcRequestWorldSeed(strtoll(argv[++i], NULL, 10));
        else if (strcmp(argv[i], "--roam") == 0 && i + 1 < argc)
            PcRequestWorldRoam(atoi(argv[++i]));
        else if (strcmp(argv[i], "--dedicated") == 0)
        {
            PcSetDedicatedServer(true);
            PcRequestHost();
        }
        else if (strcmp(argv[i], "--game-port") == 0 && i + 1 < argc)
            PcNetworkSession::SetGamePort(atoi(argv[++i]));
        else if (strcmp(argv[i], "--save-minutes") == 0 && i + 1 < argc)
            PcSetDedicatedSaveMinutes(atoi(argv[++i]));
        else if (strcmp(argv[i], "--phantoms") == 0 && i + 1 < argc)
        {
            const char* v = argv[++i];
            if (strcmp(v, "on") == 0 || strcmp(v, "off") == 0)
                PcSetPhantomsEnabled(strcmp(v, "on") == 0);
            else
                fprintf(stderr, "--phantoms must be on or off\n");
        }
        else if (strcmp(argv[i], "--server-distance") == 0 && i + 1 < argc)
            opts->serverDistance = atoi(argv[++i]);
        else if (strcmp(argv[i], "--world-size") == 0 && i + 1 < argc)
        {
            if (!PcRequestWorldSize(argv[++i]))
                fprintf(stderr, "--world-size must be classic, small, medium, large or massive\n");
        }
        else if (strcmp(argv[i], "--accept-eula") == 0)
            PcAcceptEula();
        else if (strcmp(argv[i], "--reset-nether") == 0)
            PcRequestResetNether();
        else if (strcmp(argv[i], "--reset-end") == 0)
            PcRequestResetEnd();
        else if (strcmp(argv[i], "--hidden") == 0)
            PcSetDisplayMode(ePcDisplay_Hidden);
        else if (strcmp(argv[i], "--no-lan") == 0)
            PcLanDiscovery::SetLanDisabled(true);
        else if (strcmp(argv[i], "--terminal") == 0)
            continue;
        else if (strcmp(argv[i], "--connect") == 0 && i + 1 < argc)
            opts->connectAddress = argv[++i];
        else if (strcmp(argv[i], "--headless") == 0)
            PcSetDisplayMode(ePcDisplay_Headless);
        else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc)
            opts->screenshot = argv[++i];
        else if (strcmp(argv[i], "--key") == 0 && i + 1 < argc)
        {
            ScriptedKey key;
            if (!ParseScriptedKey(argv[++i], &key))
                return false;
            opts->keys.push_back(key);
        }
        else if (strcmp(argv[i], "--pad") == 0 && i + 1 < argc)
        {
            ScriptedPad pad;
            if (!ParseScriptedPad(argv[++i], &pad))
                return false;
            opts->pads.push_back(pad);
        }
        else if (strcmp(argv[i], "--cursor") == 0 && i + 1 < argc)
        {
            ScriptedCursor cursor;
            if (sscanf(argv[++i], "%f:%f:%lf", &cursor.x, &cursor.y, &cursor.time) != 3)
                return false;
            cursor.applied = false;
            opts->cursors.push_back(cursor);
        }
        else if (strcmp(argv[i], "--shot") == 0 && i + 2 < argc)
        {
            ScriptedShot shot;
            shot.time = atof(argv[++i]);
            shot.path = argv[++i];
            shot.taken = false;
            opts->shots.push_back(shot);
        }
        else if (strcmp(argv[i], "--text") == 0 && i + 1 < argc)
        {
            const char* v = argv[++i];
            const char* separator = strrchr(v, ':');
            if (separator == 0)
                return false;
            ScriptedText s;
            s.text.assign(v, separator);
            s.time = atof(separator + 1);
            s.typed = false;
            opts->texts.push_back(s);
        }
        else if (strcmp(argv[i], "--audio-capture") == 0 && i + 1 < argc)
            PcSetAudioCapture(argv[++i]);
        else if (strcmp(argv[i], "--mic-file") == 0 && i + 1 < argc)
            PcVoiceSetFileInput(argv[++i]);
        else if (strcmp(argv[i], "--mic-test") == 0)
            PcVoiceRequestTestAtStart();
        else if (strcmp(argv[i], "--voice-chat") == 0)
            PcVoiceForceChat();
        else if (strcmp(argv[i], "--profile") == 0)
        {
            opts->profile = true;
            if (i + 1 < argc && argv[i + 1][0] != '-')
                opts->profileSlowFrame = atof(argv[++i]);
        }
        else if (strcmp(argv[i], "--profile-gpu") == 0)
        {
            opts->profile = true;
            opts->profileSyncGpu = true;
        }
        else if (strcmp(argv[i], "--fullscreen") == 0)
            PcSetStartFullscreen(true);
        else if (strcmp(argv[i], "--letterbox") == 0)
            PcSetLetterbox(true);
        else if (strcmp(argv[i], "--window-size") == 0 && i + 1 < argc)
        {
            int w = 0;
            int h = 0;
            if (sscanf(argv[++i], "%dx%d", &w, &h) != 2)
                return false;
            PcSetWindowSize(w, h);
        }
        else if (strcmp(argv[i], "--render-height") == 0 && i + 1 < argc)
            PcSetRenderHeight(atoi(argv[++i]));
        else if (strcmp(argv[i], "--button-glyphs") == 0 && i + 1 < argc)
        {
            const char* v = argv[++i];
            if (strcmp(v, "wiiu") == 0)
                PcInputSetGlyphSet(ePcGlyphSet_WiiU);
            else if (strcmp(v, "keyboard") == 0)
                PcInputSetGlyphSet(ePcGlyphSet_Keyboard);
            else if (strcmp(v, "ps3") == 0)
                PcInputSetGlyphSet(ePcGlyphSet_PS3);
            else if (strcmp(v, "ps4") == 0 || strcmp(v, "playstation") == 0)
                PcInputSetGlyphSet(ePcGlyphSet_PS4);
            else if (strcmp(v, "xbox360") == 0)
                PcInputSetGlyphSet(ePcGlyphSet_Xbox360);
            else if (strcmp(v, "xboxone") == 0 || strcmp(v, "xbox") == 0)
                PcInputSetGlyphSet(ePcGlyphSet_XboxOne);
            else if (strcmp(v, "auto") != 0)
                return false;
        }
        else if (strcmp(argv[i], "--nintendo-buttons") == 0)
            PcInputSetNintendoButtons(true);
        else if (strcmp(argv[i], "--background-input") == 0)
            PcInputSetBackgroundInput(true);
        else if (strcmp(argv[i], "--log-input") == 0)
            PcInputSetLogging(true);
        else if (strcmp(argv[i], "--host") == 0)
            PcRequestHost();
        else if (strcmp(argv[i], "--leave") == 0 && i + 1 < argc)
            PcRequestLeaveAt(atof(argv[++i]));
        else if (strcmp(argv[i], "--join") == 0)
            PcRequestJoin(i + 1 < argc && argv[i + 1][0] != '-' ? argv[++i] : "");
        else
            return false;
    }
    if (!launcherIp.empty())
        PcRequestJoin(launcherPort.empty() ? launcherIp : launcherIp + ":" + launcherPort);
    return true;
}

void CreateMainThreadStorage()
{
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
}

void MarkAddOnsOwned()
{
    cNintendoDLCInfo* catalog = gApp.GetDLCInfoInstance();
    unsigned int n = catalog->GetDLCInfoCount();
    for (unsigned int i = 0; i < n; ++i)
    {
        DLC_INFO* info = catalog->GetDLCInfoByIndex(i);
        if (info != 0)
            info->Purchased = true;
    }
}

void Boot(bool* noSpaceForSaveQuota)
{
    ShutdownManager::Initialise();
    C4JThreadImpl::SetMainThreadID();
    Compression::CreateNewThreadStorage();
    gRenderer->Initialise();
    g_pInput->Initialise(1, 0x3d, 0x17);

    DefineActions();
    g_pInput->SetJoypadMapArraySetupComplete();
    g_pInput->SetKeyRepeatRate(0.3f, 0.2f);
    g_pInput->SetDeadzoneAndMovementRange(0, 20000, 0x7fff);

    gApp.loadMediaArchive();
    gApp.InitialiseDLCInfo();
    if (gApp.ReadProductCodes() == 0)
        gApp.FatalLoadError(L"ReadProductCodes");
    MarkAddOnsOwned();

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

    gConsoleUIController.init(gRenderer->IsWidescreen() ? 1280 : 640, gRenderer->IsWidescreen() ? 720 : 480);
    gApp.CommerceInit();
    gApp.InitTime();
    static unsigned int s_profileDefaults[5];
    gProfile->Initialise(L"", L"", 0x22, 5, 4, s_profileDefaults, 0x1e60, &gApp.mGameDefinedDataChangedBitmask);
    gProfile->SetSignInChoicesCallback(CConsoleMinecraftApp::RequestSignInUIChoices);
    gProfile->SetOnAwardHandler(PopupToast, 0);
    RegisterAwardsWithProfileManager();

    gStorage->Init(0, L10N::GetString(0xc0b26be6), "savegame.dat", 51000000,
                   CConsoleMinecraftApp::DisplaySavingMessageWiiU, &gApp, "", noSpaceForSaveQuota);
    gApp.SetNoSpaceForSaveQuota(*noSpaceForSaveQuota);
    gProfile->LoadAllMiiData();
    gProfile->SetSignInChangeCallback((CProfile::SignInChangeHandler)CMinecraftApp::SignInChangeCallback, &gApp);
    gProfile->SetProfileReadErrorCallback(CMinecraftApp::ProfileReadErrorCallback, &gApp);

    static unsigned int s_storageProfileDefaults[5];
    gStorage->InitialiseGenericData();
    gStorage->InitialiseProfileData(0x22, 5, 4, s_storageProfileDefaults, 0x1e60,
                                    &gApp.mGameDefinedDataChangedBitmask);
    gStorage->SetDefaultOptionsCallback(CMinecraftApp::DefaultOptionsCallback, &gApp);
    gStorage->SetOptionsDataCallback(CMinecraftApp::OptionsDataCallback, &gApp);
    gStorage->SetOldProfileVersionCallback(GameSettings::OldProfileVersionCallback, &gApp);
    gStorage->SetGameSaveFolderPrefix("");
    gStorage->SetMaxSaves(99);
    arrayWithLength<unsigned char> thumb = gApp.getArchiveFile(L"DefaultSaveThumbnail64x64.png", false);
    gStorage->SetDefaultImages(0, 0, 0, 0, thumb.data, thumb.length);
    gStorage->SetIncompleteSaveCallback(CConsoleMinecraftApp::Callback_SaveGameIncomplete, &gApp);
    if (thumb.data != 0)
        delete[] thumb.data;
    gStorage->SetSaveTitle(L"Default Save");
    gStorage->SetOpenDirFn(OpenDir);
    gStorage->SetReadDirFn(ReadDir);
    gStorage->SetCloseDirFn(CloseDir);
    gStorage->SetDLCPackageRoot("DLCDrive");

    CreateMainThreadStorage();

    Minecraft::main();
    gApp.InitGameSettings();
    gStorage->ReadFromProfile(0, 0);
    gProfile->SetUpsellCallback(reinterpret_cast<CProfile::UpsellHandler>(CMinecraftApp::UpsellReturnedCallback),
                                &gApp);
    gApp.InitialiseTips();
    g_pLeaderboardManager->OpenSession();
    CGameNetworkManager::sInstance.Initialise();
    CGameNetworkManager::sInstance.SetLocalGame(true);
    if (*noSpaceForSaveQuota)
        g_pInput->RequestErrorViewer(1, kStorageFullErrorCode,
                                     reinterpret_cast<C_4JInput::ErrorViewerCallback>(DeviceFullErrorReturn), 0);
    C_AOC::getInstance()->Init();
}

void ApplyGameKeys(Minecraft* mc, int pad)
{
    if (noHints && GameSettings::GetGameSettings(pad, eGameSetting_Hints) != 0)
        GameSettings::SetGameSettings(pad, eGameSetting_Hints, 0);

    boost::shared_ptr<MultiplayerLocalPlayer> p = mc->mLocalPlayers[pad];
    int slot = PcInputConsumeHotbarKey();
    if (slot >= 0)
        p->mInventory->m_selected = slot;

    if (PcInputConsumeDebugCameraKey() && PcDebugMode())
        gConsoleUIController.NavigateToScene(pad, EUIScene_DebugSetCamera, NULL, EUILayer_Scene, EUIGroup_PlayerDefault);
    if (PcInputConsumeDebugFillAreaKey() && PcDebugMode())
        gConsoleUIController.NavigateToScene(pad, EUIScene_DebugFillArea, NULL, EUILayer_Scene, EUIGroup_PlayerDefault);
    if (PcInputConsumeDebugSchematicKey() && PcDebugMode())
        gConsoleUIController.NavigateToScene(pad, EUIScene_DebugCreateSchematic, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);

    const unsigned int kFreeCrafting = 0x4000;
    if (PcDebugFreeCrafting() && (p->mDebugOptions & kFreeCrafting) == 0 && mc->mGameModes[pad] != 0)
        mc->mGameModes[pad]->handleDebugOptions(p->mDebugOptions | kFreeCrafting, p);

    if (PcInputSprintHeld() && !p->isSprinting() && (p->mOnGround || p->isInWater())
        && !p->isSneaking() && !p->isUsingItem()
        && (p->getFoodData()->getFoodLevel() > FoodConstants::SprintLevel() || p->mAbilities.mMayfly
            || p->isAllowedToFly()))
        p->setSprinting(true);
}

void TakeKeyScreenshot()
{
    if (!PcInputConsumeScreenshotKey())
        return;
    PcOsMakeDirectory("screenshots");
    PcOsCalendar calendar;
    PcOsCalendarFromUnixSeconds((long long)(PcOsRealTimeMilliseconds() / 1000ULL), true, &calendar);
    char path[64];
    snprintf(path, sizeof(path), "screenshots/%04d-%02d-%02d_%02d.%02d.%02d.png", calendar.year, calendar.month,
             calendar.day, calendar.hour, calendar.minute, calendar.second);
    PcRequestScreenshot(path);
}

void ApplyMouseLook(Minecraft* minecraft)
{
    float x = 0.0f;
    float y = 0.0f;
    PcInputConsumeMouseDelta(&x, &y);
    int pad = gProfile->GetPrimaryPad();
    bool active = gApp.GetGameStarted() != 0 && minecraft->mLevel != 0 && minecraft->mLocalPlayers[pad] != 0
                  && minecraft->mGameModes[pad] != 0 && !gConsoleUIController.GetMenuDisplayed(pad)
                  && gApp.IsAppPaused() == 0;
    PcChatTick(minecraft, pad, active);
    active = active && !PcChatActive();
    PcInputSetDirectLook(active);
    PcInputSetInGame(active);
    if (active)
        ApplyGameKeys(minecraft, pad);
    if (!active || (x == 0.0f && y == 0.0f))
        return;
    float sensitivity = static_cast<float>(GameSettings::GetGameSettings(pad, eGameSetting_GameSensitivity)) / 100.0f;
    float yaw = x * sensitivity;
    float pitch = -y * sensitivity;
    if (GameSettings::GetGameSettings(pad, eGameSetting_InvertLook) != 0)
        pitch = -pitch;
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[pad];
    if (!gameMode->isInputAllowed(EControllerActions_LookLeft) && !gameMode->isInputAllowed(EControllerActions_LookRight))
        yaw = 0.0f;
    if (!gameMode->isInputAllowed(EControllerActions_LookUp) && !gameMode->isInputAllowed(EControllerActions_LookDown))
        pitch = 0.0f;
    PcInputNoteMouseLook(yaw != 0.0f ? x : 0.0f, pitch != 0.0f ? y : 0.0f);
    minecraft->mLocalPlayers[pad]->turn(yaw, pitch);
}

void RunFrame(Minecraft* minecraft)
{
    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Frame);
    gRenderer->StartFrame();
    gApp.UpdateTime();
    PIXBeginNamedEvent(0.0f, "Input manager tick");
    g_pInput->Tick();
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Profile manager tick");
    gProfile->Tick();
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Storage manager tick");
    gStorage->Tick();
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Render manager tick");
    gRenderer->Tick();
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Network manager do work #1");
    CGameNetworkManager::sInstance.DoWork();
    PIXEndNamedEvent();
    g_pLeaderboardManager->Tick();

    ApplyMouseLook(minecraft);
    PcScriptsClientFrame(minecraft);
    PcNativeModsFrame();
    TakeKeyScreenshot();
    float hoverX = 0.0f;
    float hoverY = 0.0f;
    if (PcInputConsumeHover(&hoverX, &hoverY))
    {
        int pad = gProfile->GetPrimaryPad();
        gConsoleUIController.PcHoverPointer(pad < 0 ? 0 : static_cast<unsigned int>(pad), static_cast<int>(hoverX),
                                            static_cast<int>(hoverY));
    }
    if (gApp.GetGameStarted() != 0)
    {
        PIXBeginNamedEvent(0.0f, "Minecraft run_middle");
        minecraft->run_middle();
        PIXEndNamedEvent();
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
    PIXBeginNamedEvent(0.0f, "UI tick");
    gConsoleUIController.tick();
    PIXEndNamedEvent();
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_UITick);
    gRenderer->ResetRenderState();
    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_UIRender);
    PIXBeginNamedEvent(0.0f, "UI render");
    gConsoleUIController.render();
    PIXEndNamedEvent();
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_UIRender);
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Frame);
    PIXBeginNamedEvent(0.0f, "Frame present");
    gRenderer->Present();
    PIXEndNamedEvent();
    gRenderer->Set_matrixDirty();
    gConsoleUIController.CheckMenuDisplayed();

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
    PIXBeginNamedEvent(0.0f, "Network manager do work #2");
    CGameNetworkManager::sInstance.DoWork();
    PIXEndNamedEvent();
    gApp.HandleXuiActions();
    Vec3::resetPool();
}
}

int main(int argc, char** argv)
{
    bool terminal = false;
    bool dedicated = false;
    for (int i = 1; i < argc; ++i)
    {
        terminal = terminal || strcmp(argv[i], "--terminal") == 0;
        dedicated = dedicated || strcmp(argv[i], "--dedicated") == 0;
    }
#ifdef _WIN32
    PcStartupLog("main: started");
#endif
    PcOsPrepareOutput(terminal, dedicated);
#ifdef _WIN32
    PcStartupLog("main: output ready");
#endif
    PcInstallCrashHandler();
    PcOsUsePreciseTimers();
    LaunchOptions options;
    if (!ParseArguments(argc, argv, &options))
    {
        printf("usage: %s [--content DIR] [--save DIR] [--mods DIR | --no-mods] [--no-native-mods] [--name NAME] [--identity DIR] [--world NAME [--seed N] [--host]]"
               " [--join [HOST]] [--leave SECONDS] [--log-input] [--accept-eula] [--seconds N] [--hidden | --headless] [--screenshot FILE]"
               " [--profile [MS]] [--profile-gpu] [--audio-capture FILE] [--mic-file WAV] [--mic-test] [--voice-chat]\n",
               argv[0]);
        return 2;
    }
    if (!PcPrepareSaveRoot())
        return 1;
    PcModsLoad();
    PcContentInitialise();
    if (!options.connectAddress.empty())
        PcOnlineConnectionSetAddress(options.connectAddress);
    else
        PcOnlineConnectionAddress();
    if (options.profile)
        PcProfilerEnable(options.profileSlowFrame, options.profileSyncGpu);
    PcSettingsLoad();
    PcKeyBindingsLoad();
    if (options.serverDistance > 0)
        PcSettingsSetServerDistance(options.serverDistance);
    if (PcIsDedicatedServer())
    {
        PcSettingsSetRenderDistance(kPcRenderDistanceMinimum);
        printf("[pc] server: dedicated mode; the host player is hidden, and the world saves on a timer and on stop\n");
    }
    if (FILE* marker = fopen("log-input.txt", "r"))
    {
        fclose(marker);
        PcInputSetLogging(true);
    }
    if (PcInputLogging())
        printf("[pc] input: logging clicks, keys and slow frames\n");
    if (gProfile == 0)
        gProfile = new (calloc(1, sizeof(CProfile))) CProfile();

    printf("[pc] booting\n");
#ifdef _WIN32
    printf("[pc] heap: %u MB reserved\n", static_cast<unsigned int>((gHeapEnd - gHeapStart) >> 20));
#endif
    fflush(stdout);
    bool noSpaceForSaveQuota = false;
    Boot(&noSpaceForSaveQuota);
    PcSettingsApplyDisplay();
    PcScriptsClientLoad();
    PcNativeModsLoad();
    Minecraft* minecraft = Minecraft::GetInstance();
    printf("[pc] boot complete; entering main loop\n");
#ifdef _WIN32
    PcStartupLog("main: boot complete");
#endif
    fflush(stdout);

    unsigned long long start = PcOsMonotonicNanoseconds();
    unsigned long long frames = 0;
    unsigned long long lastFrame = ~0ULL;
    bool quitRequested = false;
    bool stopping = false;
    for (;;)
    {
        PcOsWindowEvent event;
        PIXBeginNamedEvent(0.0f, "Window events");
        while (PcDisplayPollEvent(&event))
        {
            if (event.type == ePcOsWindowEvent_Close)
                quitRequested = true;
            PcInputHandleEvent(event);
        }
        PIXEndNamedEvent();
        RunScriptedInput(options, static_cast<double>(PcOsMonotonicNanoseconds() - start) / 1e9);
        RunFrame(minecraft);
        PcDebugOverlayFrame();
        PIXBeginNamedEvent(0.0f, "World launcher tick");
        PcWorldLauncherTick(minecraft);
        PcNetworkSession::TickRefusalMessage();
        PcVoiceTick();
        PcOnlineConnectionTick();
    PcMultiplayerMenuTick();
        PIXEndNamedEvent();
        quitRequested = quitRequested || PcWorldLauncherQuitRequested() || PcDedicatedStopRequested();
        ++frames;
        unsigned long long elapsed = PcOsMonotonicNanoseconds() - start;
        bool timeUp = options.seconds >= 0 && elapsed >= static_cast<unsigned long long>(options.seconds) * 1000000000ULL;
        if (!stopping && (timeUp || quitRequested))
        {
            stopping = true;
            if (!options.screenshot.empty())
                PcRequestScreenshot(options.screenshot.c_str());
            lastFrame = frames + 1;
        }
        if (stopping && frames >= lastFrame && PcWorldLauncherFinish())
            break;
        if (!gRenderer->HasWindow() || PcGetDisplayMode() != ePcDisplay_Window)
        {
            PIXBeginNamedEvent(0.0f, "Windowless frame sleep");
            PcOsSleepMilliseconds(16);
            PIXEndNamedEvent();
        }
        PcSettingsLimitFrame();
        if (PcInputLogging())
        {
            static unsigned long long previousFrameEnd = 0;
            const unsigned long long frameEnd = PcOsMonotonicNanoseconds();
            if (previousFrameEnd != 0 && frameEnd - previousFrameEnd > 50000000ULL)
            {
                printf("[pc] input: slow frame %llu ms\n", (frameEnd - previousFrameEnd) / 1000000ULL);
                fflush(stdout);
            }
            previousFrameEnd = frameEnd;
        }
        PcProfilerFrameEnd();
        fflush(stdout);
    }
    printf("[pc] ran %llu frames; exiting\n", frames);
    PcOsExitImmediately(0);
}
