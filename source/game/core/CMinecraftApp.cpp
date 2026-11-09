#include "game/core/CMinecraftApp.hpp"
#include "game/core/VirtualPadKeys.hpp"
#include "game/core/System.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/XLanguage.hpp"
#include "game/core/EntityType.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/PlatformTiming.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "game/core/ArchiveFile.hpp"
#include "game/core/File.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CStorage.hpp"
#include <sstream>
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/L10N.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Compression.hpp"
#include "game/core/GameModeTransitionParams.hpp"
#include "game/core/GhostController.hpp"
#include "game/util/Random.hpp"
#include "game/core/StringUtils.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/item/CMinecraftApp.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/chunk/LevelChunk.hpp"

#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/core/Achievement.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/IUIScene_DLCUpsell.hpp"
#include "game/core/MOJANG_DATA.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Stat.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/item/Container.hpp"

#ifdef MINECRAFT_PC
#include "platform/pc/PcDebug.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcKeyBindings.hpp"
#endif
#include "game/item/Inventory.hpp"
#include "game/core/Merchant.hpp"
#include "game/ui/IUIScene_CraftingMenu.hpp"
#include "game/ui/UIScene_ClassicCraftingMenu.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/core/LeaderboardManager.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/core/User.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "vendor/_ConnectionProgressParams.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "platform/wiiu/PlayerUID.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "vendor/SKIN_BOX.hpp"
#include <algorithm>
#include <boost/pointer_cast.hpp>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>

static boost::unordered_map<PlayerUID, MOJANG_DATA*> g_mojangData;
unsigned int CMinecraftApp::sPreviousSignInUsersMask = 0;
CConsoleMinecraftApp gApp;
static Random* sTipRandom = new Random();
static bool s_pendingDlcInstallTip = true;

struct BANNEDLISTDATA
{
    PlayerUID playerUid;
    char levelId[14];
    char padding[2];
};

namespace
{

struct ContainerMenuInitData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

struct HorseMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    boost::shared_ptr<AbstractHorse> horse;
    int player;
    bool splitscreen;
};

struct InventoryContainerMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

struct ContainerPairMenuInitData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

struct TradingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Merchant> merchant;
    Level* level;
    int player;
    bool splitscreen;
};

struct SignEntryMenuInitData
{
    boost::shared_ptr<SignBlockEntity> sign;
    int player;
};

struct LocalPlayerMenuInitData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool creative;
    int player;
    bool splitscreen;
};

struct EnchantingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int playerIndex;
    bool splitscreen;
    std::wstring name;
};

struct RepairingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int player;
    bool splitscreen;
};

struct WritingBookMenuInitData
{
    boost::shared_ptr<ItemInstance> item;
    int player;
    bool signing;
};

struct InviteJoinData
{
    int userIndex;
    unsigned int userIndexMask;
    const nn::fp::FriendPresence* presence;
};

bool sRetryDisconnectAction = false;

}

static TIPSTRUCT sGeneralTips[] = {
    {0, IDS_TIPS_GAMETIP_1},  {0, IDS_TIPS_GAMETIP_2},  {0, IDS_TIPS_GAMETIP_3},  {0, IDS_TIPS_GAMETIP_4},
    {0, IDS_TIPS_GAMETIP_6},  {0, IDS_TIPS_GAMETIP_7},  {0, IDS_TIPS_GAMETIP_8},  {0, IDS_TIPS_GAMETIP_9},
    {0, IDS_TIPS_GAMETIP_10}, {0, IDS_TIPS_GAMETIP_11}, {0, IDS_TIPS_GAMETIP_12}, {0, IDS_TIPS_GAMETIP_13},
    {0, IDS_TIPS_GAMETIP_14}, {0, IDS_TIPS_GAMETIP_15}, {0, IDS_TIPS_GAMETIP_16}, {0, IDS_TIPS_GAMETIP_17},
    {0, IDS_TIPS_GAMETIP_18}, {0, IDS_TIPS_GAMETIP_19}, {0, IDS_TIPS_GAMETIP_20}, {0, IDS_TIPS_GAMETIP_22},
    {0, IDS_TIPS_GAMETIP_23}, {0, IDS_TIPS_GAMETIP_24}, {0, IDS_TIPS_GAMETIP_25}, {0, IDS_TIPS_GAMETIP_26},
    {0, IDS_TIPS_GAMETIP_27}, {0, IDS_TIPS_GAMETIP_28}, {0, IDS_TIPS_GAMETIP_29}, {0, IDS_TIPS_GAMETIP_30},
    {0, IDS_TIPS_GAMETIP_31}, {0, IDS_TIPS_GAMETIP_32}, {0, IDS_TIPS_GAMETIP_33}, {0, IDS_TIPS_GAMETIP_34},
    {0, IDS_TIPS_GAMETIP_35}, {0, IDS_TIPS_GAMETIP_37}, {0, IDS_TIPS_GAMETIP_38}, {0, IDS_TIPS_GAMETIP_39},
    {0, IDS_TIPS_GAMETIP_40}, {0, IDS_TIPS_GAMETIP_41}, {0, IDS_TIPS_GAMETIP_42}, {0, IDS_TIPS_GAMETIP_43},
    {0, IDS_TIPS_GAMETIP_44}, {0, IDS_TIPS_GAMETIP_45}, {0, IDS_TIPS_GAMETIP_46}, {0, IDS_TIPS_GAMETIP_47},
    {0, IDS_TIPS_GAMETIP_48}, {0, IDS_TIPS_GAMETIP_49}, {0, IDS_TIPS_GAMETIP_50}};
static TIPSTRUCT sAlternateTips[] = {
    {0, IDS_TIPS_TRIVIA_1},  {0, IDS_TIPS_TRIVIA_2},  {0, IDS_TIPS_TRIVIA_3},  {0, IDS_TIPS_TRIVIA_4},
    {0, IDS_TIPS_TRIVIA_6},  {0, IDS_TIPS_TRIVIA_7},  {0, IDS_TIPS_TRIVIA_8},  {0, IDS_TIPS_TRIVIA_9},
    {0, IDS_TIPS_TRIVIA_10}, {0, IDS_TIPS_TRIVIA_11}, {0, IDS_TIPS_TRIVIA_12}, {0, IDS_TIPS_TRIVIA_13},
    {0, IDS_TIPS_TRIVIA_14}, {0, IDS_TIPS_TRIVIA_15}, {0, IDS_TIPS_TRIVIA_16}, {0, IDS_TIPS_TRIVIA_17},
    {0, IDS_TIPS_TRIVIA_18}, {0, IDS_TIPS_TRIVIA_19}, {0, IDS_TIPS_TRIVIA_20}, {0, IDS_TIPS_TRIVIA_21},
    {0, IDS_TIPS_TRIVIA_22}, {0, IDS_TIPS_TRIVIA_23}, {0, IDS_TIPS_TRIVIA_24}, {0, IDS_TIPS_TRIVIA_25}};
static TIPSTRUCT sMiniGameTips1[] = {{0, 0x6F523BBE}, {0, 0xA85A4173}, {0, 0x8257C70A},
                                     {0, 0xE0684AD1}, {0, 0x88938BD8}, {0, 0x3440F824}};
static TIPSTRUCT sMiniGameTips2[] = {{0, IDS_MG02_TIP_01}, {0, IDS_MG02_TIP_02}, {0, IDS_MG02_TIP_03},
                                     {0, IDS_MG02_TIP_04}, {0, IDS_MG02_TIP_05}, {0, IDS_MG02_TIP_06},
                                     {0, IDS_MG02_TIP_07}, {0, IDS_MG02_TIP_08}};
static TIPSTRUCT sMiniGameTipsOther[] = {
    {0, IDS_MG03_TIP_01}, {0, IDS_MG03_TIP_02}, {0, IDS_MG03_TIP_03}, {0, IDS_MG03_TIP_04}, {0, IDS_MG03_TIP_05},
    {0, IDS_MG03_TIP_06}, {0, IDS_MG03_TIP_08}, {0, IDS_MG03_TIP_09}, {0, IDS_MG03_TIP_10}, {0, IDS_MG03_TIP_11},
    {0, IDS_MG03_TIP_12}, {0, IDS_MG03_TIP_13}, {0, IDS_MG03_TIP_14}};

uint32_t DeviceFullErrorReturn(void* context)
{
    (void)context;
    switch (gApp.GetOptionsCallbackStatus(0))
    {
    case C4JStorage::eOptions_Callback_Read:
    case C4JStorage::eOptions_Callback_Read_FileNotFound:
        if (GameSettings::GetGameSettings(0, eGameSetting_EULAAccepted) != 0)
        {
            gConsoleUIController.NavigateToScene(0, EUIScene_SaveMessage, NULL, EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
    default:
        gConsoleUIController.NavigateToScene(0, EUIScene_EULA, NULL, EUILayer_Scene, EUIGroup_PlayerDefault);
        break;
    }
    return 0;
}

CMinecraftApp::PngTagData::PngTagData() : hostOptions(0), texturePack(0), extraData(0)
{
    for (int index = 0; index < 5; ++index)
        exploredChunks[index] = 0;
    seed = 0;
    flags &= ~(eFlag_ExploredChunks | eFlag_ExtraData | eFlag_TexturePack | eFlag_HostOptions | eFlag_BaseSaveName
               | eFlag_Seed);
    memset(baseSaveName, 0, sizeof(baseSaveName));
}
CMinecraftApp::_TipData::_TipData() : m_index(0), m_count(0), m_tips(0)
{
}
CMinecraftApp::_TipData::_TipData(TIPSTRUCT* tips, unsigned int count) : m_index(0), m_count(count), m_tips(tips)
{
    unsigned int first = tips == sGeneralTips ? 1 : 0;
    for (unsigned int index = first; index < count; ++index)
        m_tips[index].sortValue = sTipRandom->nextInt();
    std::qsort(m_tips + 1, count - 1, sizeof(TIPSTRUCT), CMinecraftApp::TipsSortFunction);
}
unsigned int CMinecraftApp::_TipData::getNextTip()
{
    unsigned int tip;
    do
    {
        if (m_index >= m_count)
            m_index = 0;
        tip = m_tips[m_index++].tipId;
    } while (CMinecraftApp::tipFiltered(tip));
    return tip;
}
CMinecraftApp::CMinecraftApp()
{
    for (int player = 0; player < 4; ++player)
    {
        mXuiActions[player].action = eXuiAction_Idle;
        mXuiActions[player].param = NULL;
        mTMSActions[player] = 0;
        if (XUserGetSigninInfo(player, 2, &mSignInInfo[player]) < 0)
        {
            static_cast<PlayerUID&>(mSignInInfo[player]) = PlayerUID(0);
            mSignInInfo[player].dwInfoFlags = 0;
        }
        mBanListRead[player] = false;
        SetBanListCheck(player, false);
        mOpacityTimer[player] = 0;
    }
    mLiveLinkRequired = false;
    mDebugInputEnabled = false;
    mAppPaused = false;
    mGameMode = 0;
    mGlobalXuiAction = eXuiAction_Idle;
    mUnusedGameModeFlags[0] = false;
    mDisconnectReason = 0;
    mUnusedServerActionWord = 0;
    mLoadSavesFromFolderEnabled = false;
    mUnusedStartedFlag = false;
    mChangingSessionType = false;
    mGameStarted = false;
    mReallyChangingSessionType = false;
    mUnusedBannedListFlag = false;
    mPrimaryPlayerUID = PlayerUID(0);
    mUnusedClearedWords[0] = 0;
    mUnusedClearedWords[1] = 0;
    mUnusedClearedWords[2] = 0;
    mUnusedClearedWords[3] = 0;
    mDLCInstallIndex = 0;
    mUnusedDLCInstallFlag = false;
    mDLCInstallPending = false;
    mAutosaveTime = 0;
    mDLCInstallProcessCompleted = false;
    mDLCInstallCount = 0;
    mTrialPausedTime = 0.0f;
    std::memset(mUniqueMapName, 0, sizeof(mUniqueMapName));
    mUnusedNewDLCFlag = false;
    mNewDLCAvailable = false;
    std::memset(mPlayerIds, 0, sizeof(mPlayerIds));
    mDLCDownloadQueueResetFlag = true;
    mDLCOffersRetrievedValue = 0;
    InitializeCriticalSection(&mDLCRetrieveMutex);
    mUnusedBanListFlag = false;
    mUnusedDLCQueueFlags[1] = true;
    mUnusedDLCQueueFlags[0] = true;
    InitializeCriticalSection(&mDLCDownloadQueueMutex);
    InitializeCriticalSection(&mAdditionalModelPartsMutex);
    InitializeCriticalSection(&mAdditionalSkinBoxesMutex);
    InitializeCriticalSection(&mSkinAdjustmentsMutex);
    InitializeCriticalSection(&mUnusedMutex);
    InitializeCriticalSection(&mMemoryTextureMutex);
    InitializeCriticalSection(&mMemoryTPDMutex);
    InitializeCriticalSection(&mActionMutex);
    InitializeCriticalSection(&mPendingActionsMutex);
    InitializeCriticalSection(&mTerrainFeaturePositionsMutex);
    InitializeCriticalSection(&mSaveNotificationMutex);
    mUnusedGameModeFlags[2] = true;
    mDebugUIFocussed = false;
    mResetEnd = false;
    mOptionsCallbackStatus[0] = 0;
    mOptionsCallbackStatus[1] = 0;
    mOptionsCallbackStatus[2] = 0;
    mOptionsCallbackStatus[3] = 0;
    mResetNether = false;
    mRequiredTexturePackID = 0;
    mSaveNotificationCount = 0;
    for (int player = 0; player < 4; ++player)
        mBannedListData[player] = new std::vector<BANNEDLISTDATA*>();
    LocaleAndLanguageInit();
    mTutorialModes[PlayerUID(0)] = false;
    mFirstMiniGameType = eMiniGameId_Lobby;
}
CMinecraftApp::~CMinecraftApp()
{
}
void CMinecraftApp::SetGameHostOption(unsigned int& options, eGameHostOption option, unsigned int value)
{
    switch (option)
    {
    case eGameHostOption_FriendsOfFriends:
        if (value != 0)
            options |= 4U;
        else
            options &= ~4U;
        break;
    case eGameHostOption_Difficulty:
        options &= ~3U;
        options |= 3U & value;
        break;
    case eGameHostOption_Gamertags:
        if (value != 0)
            options |= 8U;
        else
            options &= ~8U;
        break;
    case eGameHostOption_GameType:
        options &= ~0x30U;
        options |= 0x30U & (value << 4);
        break;
    case eGameHostOption_LevelType:
        if (value != 0)
            options |= 0x40U;
        else
            options &= ~0x40U;
        break;
    case eGameHostOption_Structures:
        if (value != 0)
            options |= 0x80U;
        else
            options &= ~0x80U;
        break;
    case eGameHostOption_BonusChest:
        if (value != 0)
            options |= 0x100U;
        else
            options &= ~0x100U;
        break;
    case eGameHostOption_HasBeenInCreative:
        if (value != 0)
            options |= 0x200U;
        else
            options &= ~0x200U;
        break;
    case eGameHostOption_PvP:
        if (value != 0)
            options |= 0x400U;
        else
            options &= ~0x400U;
        break;
    case eGameHostOption_TrustPlayers:
        if (value != 0)
            options |= 0x800U;
        else
            options &= ~0x800U;
        break;
    case eGameHostOption_TNT:
        if (value != 0)
            options |= 0x1000U;
        else
            options &= ~0x1000U;
        break;
    case eGameHostOption_FireSpreads:
        if (value != 0)
            options |= 0x2000U;
        else
            options &= ~0x2000U;
        break;
    case eGameHostOption_CheatsEnabled:
        if (value != 0)
            options |= 0x1c000U;
        else
            options &= ~0x1c000U;
        break;
    case eGameHostOption_HostCanFly:
        if (value != 0)
            options |= 0x4000U;
        else
            options &= ~0x4000U;
        break;
    case eGameHostOption_HostCanChangeHunger:
        if (value != 0)
            options |= 0x8000U;
        else
            options &= ~0x8000U;
        break;
    case eGameHostOption_HostCanBeInvisible:
        if (value != 0)
            options |= 0x10000U;
        else
            options &= ~0x10000U;
        break;
    case eGameHostOption_DisableSaving:
        if (value != 0)
            options |= 0x40000U;
        else
            options &= ~0x40000U;
        break;
    case eGameHostOption_WasntSaveOwner:
        if (value != 0)
            options |= 0x80000U;
        else
            options &= ~0x80000U;
        break;
    case eGameHostOption_MobGriefing:
        if (value == 0)
            options |= 0x800000U;
        else
            options &= ~0x800000U;
        break;
    case eGameHostOption_KeepInventory:
        if (value != 0)
            options |= 0x1000000U;
        else
            options &= ~0x1000000U;
        break;
    case eGameHostOption_DoMobSpawning:
        if (value == 0)
            options |= 0x2000000U;
        else
            options &= ~0x2000000U;
        break;
    case eGameHostOption_DoMobLoot:
        if (value == 0)
            options |= 0x4000000U;
        else
            options &= ~0x4000000U;
        break;
    case eGameHostOption_DoTileDrops:
        if (value == 0)
            options |= 0x8000000U;
        else
            options &= ~0x8000000U;
        break;
    case eGameHostOption_NaturalRegeneration:
        if (value == 0)
            options |= 0x10000000U;
        else
            options &= ~0x10000000U;
        break;
    case eGameHostOption_DoDaylightCycle:
        if (value == 0)
            options |= 0x20000000U;
        else
            options &= ~0x20000000U;
        break;
    case eGameHostOption_DoWeatherCycle:
        if (value == 0)
            options |= 0x80000000U;
        else
            options &= ~0x80000000U;
        break;
    case eGameHostOption_WorldSize:
        options &= ~0x700000U;
        options |= 0x700000U & (value << 20);
        break;
    case eGameHostOption_All:
        options = value;
        break;
    case eGameHostOption_MiniGame:
        if (value != 0)
            options |= 0x40000000U;
        else
            options &= ~0x40000000U;
        break;
    default:
        break;
    }
}
void CMinecraftApp::SetGameHostOption(eGameHostOption option, unsigned int value)
{
    SetGameHostOption(mCurrentImageTextData.hostOptions, option, value);
}
void CMinecraftApp::SetXuiServerAction(int player, eXuiServerAction action, void* param)
{
    mXuiServerActions[player] = action;
    mXuiServerActionParams[player] = param;
}
namespace boost
{
void assertion_failed(const char* expr, const char* function, const char* file, long line)
{
}
}
uint32_t CMinecraftApp::GetGameHostOption(unsigned int options, eGameHostOption option)
{
    switch (option)
    {
    case eGameHostOption_FriendsOfFriends:
        return options & 4U;
    case eGameHostOption_Difficulty:
        return options & 3U;
    case eGameHostOption_Gamertags:
        return options & 8U;
    case eGameHostOption_GameType:
        return (options >> 4) & 3U;
    case eGameHostOption_All:
        return options;
    case eGameHostOption_Tutorial:
        return (options & 8U) | 0x3c81U | ((static_cast<unsigned int>(GameType::SURVIVAL->getId()) & 3U) << 4);
    case eGameHostOption_LevelType:
        return options & 0x40U;
    case eGameHostOption_Structures:
        return options & 0x80U;
    case eGameHostOption_BonusChest:
        return options & 0x100U;
    case eGameHostOption_HasBeenInCreative:
        return options & 0x200U;
    case eGameHostOption_PvP:
        return options & 0x400U;
    case eGameHostOption_TrustPlayers:
        return options & 0x800U;
    case eGameHostOption_TNT:
        return options & 0x1000U;
    case eGameHostOption_FireSpreads:
        return options & 0x2000U;
    case eGameHostOption_CheatsEnabled:
        return options & 0x1c000U;
    case eGameHostOption_HostCanFly:
        return options & 0x4000U;
    case eGameHostOption_HostCanChangeHunger:
        return options & 0x8000U;
    case eGameHostOption_HostCanBeInvisible:
        return options & 0x10000U;
    case eGameHostOption_DisableSaving:
        return options & 0x40000U;
    case eGameHostOption_WasntSaveOwner:
        return options & 0x80000U;
    case eGameHostOption_WorldSize:
        return (options >> 20) & 7U;
    case eGameHostOption_MobGriefing:
        return ((options >> 23) & 1U) ^ 1U;
    case eGameHostOption_KeepInventory:
        return options & 0x1000000U;
    case eGameHostOption_DoMobSpawning:
        return ((options >> 25) & 1U) ^ 1U;
    case eGameHostOption_DoMobLoot:
        return ((options >> 26) & 1U) ^ 1U;
    case eGameHostOption_DoTileDrops:
        return ((options >> 27) & 1U) ^ 1U;
    case eGameHostOption_NaturalRegeneration:
        return ((options >> 28) & 1U) ^ 1U;
    case eGameHostOption_DoDaylightCycle:
        return ((options >> 29) & 1U) ^ 1U;
    case eGameHostOption_MiniGame:
        return options & 0x40000000U;
    case eGameHostOption_DoWeatherCycle:
        return (options >> 31) ^ 1U;
    default:
        return 0;
    }
}
uint32_t CMinecraftApp::GetGameHostOption(eGameHostOption option)
{
    return GetGameHostOption(mCurrentImageTextData.hostOptions, option);
}
uint32_t CMinecraftApp::GetOptionsCallbackStatus(int32_t player)
{
    return mOptionsCallbackStatus[player];
}
void CMinecraftApp::UpdateTime()
{
    _LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    _LARGE_INTEGER elapsed;
    elapsed.QuadPart = counter.QuadPart - mLastPerformanceCounter.QuadPart;
    mAccumulatedPerformanceCounter += elapsed.QuadPart;
    mLastPerformanceCounter = counter;
    float scale = mPerformanceCounterScale;
    mFrameTime = scale * elapsed.QuadPart;
    mAppTime = scale * mAccumulatedPerformanceCounter;
}
int CMinecraftApp::GetLocalPlayerCount()
{
    int count = 0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        if (minecraft->mLocalPlayers[0].get() != 0)
            ++count;
        if (minecraft->mLocalPlayers[1].get() != 0)
            ++count;
        if (minecraft->mLocalPlayers[2].get() != 0)
            ++count;
        if (minecraft->mLocalPlayers[3].get() != 0)
            ++count;
    }
    return count;
}
void CMinecraftApp::loadMediaArchive()
{
    std::wstring path = L"";
    path = L"Common\\Media\\MediaWiiU.arc";
    if (!path.empty())
        mMediaArchive = new ArchiveFile(File(path));
}
arrayWithLength<unsigned char> CMinecraftApp::getArchiveFile(const std::wstring& path, bool ignoreTexturePack)
{
    TexturePack* selected = NULL;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mTexturePackRepository != NULL)
        selected = minecraft->mTexturePackRepository->getSelected();
    if (!ignoreTexturePack && selected != NULL && selected->hasData() && selected->getArchiveFile() != NULL
        && selected->getArchiveFile()->hasFile(path))
        return selected->getArchiveFile()->getFile(path);
    return mMediaArchive->getFile(path);
}
bool CMinecraftApp::hasArchiveFile(const std::wstring& path, bool ignoreTexturePack)
{
    TexturePack* selected = 0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0 && minecraft->mTexturePackRepository != 0)
        selected = minecraft->mTexturePackRepository->getSelected();

    if (ignoreTexturePack == 0 && selected != 0 && selected->hasData() && selected->getArchiveFile() != 0
        && selected->getArchiveFile()->hasFile(path))
        return true;

    return mMediaArchive->hasFile(path);
}
void CMinecraftApp::InitTime()
{
    _LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    mPerformanceCounterScale = 1.0f / frequency.QuadPart;
    QueryPerformanceCounter(&mLastPerformanceCounter);
    mAccumulatedPerformanceCounter = 0;
    mAppTime = 0.0f;
    mFrameTime = 0.0f;
}
uint32_t CMinecraftApp::IsActionPending(int player, eXuiAction action)
{
    EnterCriticalSection(&mPendingActionsMutex);
    if (mXuiActions[player].action == action)
    {
        LeaveCriticalSection(&mPendingActionsMutex);
        return 1;
    }
    for (unsigned int i = 0; i < mPendingActions[player].size(); ++i)
        if (mPendingActions[player][i].action == action)
        {
            LeaveCriticalSection(&mPendingActionsMutex);
            return 1;
        }
    LeaveCriticalSection(&mPendingActionsMutex);
    return 0;
}
bool CMinecraftApp::isValidStateTransition(eXuiAction current, eXuiAction next)
{
    switch (current)
    {
    case eXuiAction_ExitWorld:
        return next == eXuiAction_Idle || next == eXuiAction_ExitWorldNow;
    case eXuiAction_ExitWorldNow:
    case eXuiAction_ExitWorldAfterDisconnect:
    case eXuiAction_ReturnToMenuAfterDisconnect:
    case eXuiAction_ReloadTexturePack:
        return next == eXuiAction_Idle;
    case eXuiAction_ConnectionLost:
        return next == eXuiAction_Idle || next == eXuiAction_ConnectionLost || next == eXuiAction_ConnectionLostWait
               || next == eXuiAction_ChangeSessionType;
    case eXuiAction_ConnectionLostWait:
        return next == eXuiAction_ExitWorldAfterDisconnect || next == eXuiAction_ReturnToMenuAfterDisconnect;
    case eXuiAction_ReloadTexturePackInTransition:
        return next == eXuiAction_GameModeTransitionInProgress;
    case eXuiAction_GameModeTransitionInProgress:
        return next == eXuiAction_Idle || next == eXuiAction_ReloadTexturePackInTransition;
    default:
        return true;
    }
}

void CMinecraftApp::SetActionInternal(int player, eXuiAction action, void* param)
{
    if (static_cast<unsigned int>(player) >= 4U)
        return;

    const eXuiAction currentAction = mXuiActions[player].action;

    if ((IsActionPending(player, action) == 0
         || (action != eXuiAction_ConnectionLost && action != eXuiAction_ExitWorld
             && action != eXuiAction_PrimarySignedOut))
        && (currentAction != eXuiAction_SetDefaultGameOptions || action != eXuiAction_ReloadSkin)
        && (currentAction == eXuiAction_Idle || action != eXuiAction_RespawnProgress)
        && (currentAction != eXuiAction_ConnectionLostWait || action != eXuiAction_ConnectionLost))
    {
        if (action == eXuiAction_Idle)
        {
            std::vector<XuiActionAndParam>& pending = mPendingActions[player];
            if (!pending.empty())
            {
                EnterCriticalSection(&mPendingActionsMutex);
                const XuiActionAndParam next = pending.front();
                pending.erase(pending.begin());
                LeaveCriticalSection(&mPendingActionsMutex);
                mXuiActions[player] = next;
                return;
            }
        }
        else if (currentAction != eXuiAction_Idle && !isValidStateTransition(currentAction, action))
        {
            EnterCriticalSection(&mPendingActionsMutex);
            XuiActionAndParam entry;
            entry.action = action;
            entry.param = param;
            mPendingActions[player].push_back(entry);
            LeaveCriticalSection(&mPendingActionsMutex);
            return;
        }
        mXuiActions[player].action = action;
        mXuiActions[player].param = param;
    }
}
void CMinecraftApp::SetAction(int player, eXuiAction action, void* param)
{
    EnterCriticalSection(&mActionMutex);
    SetActionInternal(player, action, param);
    LeaveCriticalSection(&mActionMutex);
}
void CMinecraftApp::SetBanListCheck(int32_t player, bool check)
{
    mBanListCheck[player] = check;
}
void CMinecraftApp::InvalidateBannedList(int32_t player)
{
    if (!mBanListRead[player])
        return;
    mBanListRead[player] = false;
    SetBanListCheck(player, false);
    mBannedListData[player]->clear();
    if (mBanListBuffers[player].data != NULL)
    {
        delete[] mBanListBuffers[player].data;
        mBanListBuffers[player].data = NULL;
    }
}
void CMinecraftApp::ClearAndResetDLCDownloadQueue()
{
    EnterCriticalSection(&mDLCDownloadQueueMutex);
    for (std::vector<_DLCRequest*>::iterator it = mDLCDownloadQueue.begin(); it != mDLCDownloadQueue.end(); ++it)
        delete *it;
    mDLCDownloadQueue.clear();
    mDLCDownloadQueueResetFlag = true;
    LeaveCriticalSection(&mDLCDownloadQueueMutex);
}
void CMinecraftApp::ClearDLCInstalled()
{
    mDLCInstallProcessCompleted = false;
}
uint8_t CMinecraftApp::GetLiveLinkRequired()
{
    return mLiveLinkRequired;
}
void CMinecraftApp::ClearSignInChangeUsersMask()
{
    sPreviousSignInUsersMask = 0;
}

int CMinecraftApp::SignInChangeCallback(void* context, bool signedIn, unsigned int usersMask)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    const int primaryPad = gProfile->GetPrimaryPad();
    const int lockedProfile = gProfile->GetLockedProfile();

    if (primaryPad != -1 && lockedProfile == -1)
    {
        app->InvalidateBannedList(primaryPad);
        gStorage->ClearDLCOffers();
        app->ClearAndResetDLCDownloadQueue();
        app->ClearDLCInstalled();
    }
    else
    {
        if (primaryPad != -1)
        {
            const unsigned int primaryBit = 1u << primaryPad;
            if ((usersMask & primaryBit) == 0 || signedIn)
            {
                app->SetAction(primaryPad, eXuiAction_PrimarySignedOut, NULL);
                app->InvalidateBannedList(primaryPad);
                gStorage->ClearDLCOffers();
                app->ClearAndResetDLCDownloadQueue();
                app->ClearDLCInstalled();
            }
            else
            {
                const unsigned int changedMask = usersMask ^ sPreviousSignInUsersMask;
                CGameNetworkManager& network = CGameNetworkManager::sInstance;
                if (!network.IsInSession())
                {
                    if (app->GetLiveLinkRequired() && !gProfile->IsSignedInLive(gProfile->GetLockedProfile()))
                        app->SetAction(primaryPad, eXuiAction_ConnectionLost, NULL);
                }
                else
                {
                    bool guestSignedOut = false;
                    for (int pad = 0; pad < 4; ++pad)
                    {
                        unsigned int signInFlags = 0;
                        if (gProfile->IsSignedIn(pad))
                        {
                            _XUSER_SIGNIN_INFO info;
                            XUserGetSigninInfo(pad, 2, &info);
                            signInFlags = info.dwInfoFlags;
                        }
                        const unsigned int cachedFlags = app->mSignInInfo[pad].dwInfoFlags;
                        if (cachedFlags != 0 && signInFlags != 0 && cachedFlags != signInFlags)
                            guestSignedOut = true;
                    }
                    if (guestSignedOut)
                    {
                        int options[1] = {IDS_CONFIRM_OK};
                        gConsoleUIController.RequestErrorMessage(IDS_GUEST_ORDER_CHANGED_TITLE,
                                                                 IDS_GUEST_ORDER_CHANGED_TEXT, options, 1,
                                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
                    }

                    const bool connectionLost = !gProfile->IsSignedInLive(gProfile->GetLockedProfile())
                                                && !network.IsLocalGame();

                    for (int pad = 0; pad < 4; ++pad)
                    {
                        if (pad == primaryPad)
                            continue;
                        if (guestSignedOut && app->mSignInInfo[pad].dwInfoFlags != 0
                            && network.GetLocalPlayerByUserIndex(pad) != NULL)
                        {
                            app->SetAction(pad, eXuiAction_RemoveLocalPlayer, NULL);
                            continue;
                        }
                        _XUSER_SIGNIN_INFO info;
                        XUserGetSigninInfo(pad, 2, &info);
                        const unsigned int padBit = 1u << pad;
                        if ((changedMask & padBit) != padBit)
                            continue;
                        if ((usersMask & padBit) != 0 && gProfile->AreXUIDSEqual(app->mSignInInfo[pad], info))
                            continue;
                        app->InvalidateBannedList(pad);
                        if (network.GetLocalPlayerByUserIndex(pad) != NULL
                            || Minecraft::GetInstance()->mLocalPlayers[pad] != NULL)
                            app->SetAction(pad, eXuiAction_RemoveLocalPlayer, NULL);
                    }

                    if (connectionLost)
                        app->SetAction(primaryPad, eXuiAction_ConnectionLost, NULL);
                    network.HandleSignInChange();
                }

                for (int pad = 0; pad < 4; ++pad)
                {
                    const unsigned int padBit = 1u << pad;
                    if (pad != primaryPad && (changedMask & padBit) != 0 && (usersMask & padBit) != 0)
                        gStorage->ReadFromProfile(pad, 0);
                }
            }
        }

        sPreviousSignInUsersMask = usersMask;
    }

    for (int pad = 0; pad < 4; ++pad)
    {
        if (XUserGetSigninInfo(pad, 2, &app->mSignInInfo[pad]) < 0)
        {
            app->mSignInInfo[pad] = _XUSER_SIGNIN_INFO();
            app->mSignInInfo[pad].dwInfoFlags = 0;
        }
    }
    return 0;
}
void CMinecraftApp::ProfileReadErrorCallback(void* context)
{
    static_cast<CMinecraftApp*>(context)->SetAction(gProfile->GetPrimaryPad(), eXuiAction_ProfileReadError, NULL);
}
int CMinecraftApp::DefaultOptionsCallback(void* context, C4JStorage::PROFILESETTINGS* settings, int profile)
{
    static_cast<CMinecraftApp*>(context)->SetAction(profile, eXuiAction_SetDefaultGameOptions, settings);
    return 0;
}
int CMinecraftApp::OptionsDataCallback(void* context, int profile, unsigned short size,
                                       C4JStorage::eOptionsCallback status)
{
    static_cast<CMinecraftApp*>(context)->mOptionsCallbackStatus[profile] = status;
    return 0;
}
void CMinecraftApp::EnterSaveNotificationSection()
{
    EnterCriticalSection(&mSaveNotificationMutex);
    const int previousCount = mSaveNotificationCount;
    ++mSaveNotificationCount;
    if (previousCount == 0 && CGameNetworkManager::sInstance.IsInSession()
        && !CGameNetworkManager::sInstance.IsLeavingGame() && gMinecraftServer != NULL)
    {
        gMinecraftServer->broadcastStartSavingPacket();
        if (CGameNetworkManager::sInstance.IsLocalGame() && CGameNetworkManager::sInstance.GetPlayerCount() == 1)
        {
            gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer,
                                    reinterpret_cast<void*>(1));
        }
    }
    LeaveCriticalSection(&mSaveNotificationMutex);
}
void CMinecraftApp::LeaveSaveNotificationSection()
{
    EnterCriticalSection(&mSaveNotificationMutex);
    mSaveNotificationCount = mSaveNotificationCount - 1;
    if (mSaveNotificationCount == 0 && CGameNetworkManager::sInstance.IsInSession()
        && !CGameNetworkManager::sInstance.IsLeavingGame() && gMinecraftServer != NULL)
    {
        gMinecraftServer->broadcastStopSavingPacket();
        if (CGameNetworkManager::sInstance.IsLocalGame() && CGameNetworkManager::sInstance.GetPlayerCount() == 1)
        {
            gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer,
                                    reinterpret_cast<void*>(0));
        }
    }
    LeaveCriticalSection(&mSaveNotificationMutex);
}
bool CMinecraftApp::GetGameStarted()
{
    return mGameStarted;
}
void CMinecraftApp::SetDefaultHostOptions()
{
    gApp.SetGameHostOption(eGameHostOption_MobGriefing, 1);
    gApp.SetGameHostOption(eGameHostOption_KeepInventory, 0);
    gApp.SetGameHostOption(eGameHostOption_DoMobSpawning, 1);
    gApp.SetGameHostOption(eGameHostOption_DoMobLoot, 1);
    gApp.SetGameHostOption(eGameHostOption_DoTileDrops, 1);
    gApp.SetGameHostOption(eGameHostOption_NaturalRegeneration, 1);
    gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1);
    gApp.SetGameHostOption(eGameHostOption_DoWeatherCycle, 1);
}
void CMinecraftApp::InitGameSettings()
{
    GameSettings::InitGameSettings();
    SetDefaultHostOptions();
}
int CMinecraftApp::UpsellReturnedCallback(void* context, eUpsellType type, eUpsellResponse response, int userIndex)
{
    ESen_UpsellOutcome outcome;
    if (response == static_cast<eUpsellResponse>(1))
        outcome = eSen_UpsellOutcome_Purchased;
    else if (response == static_cast<eUpsellResponse>(2))
        outcome = eSen_UpsellOutcome_Declined;
    else
        outcome = eSen_UpsellOutcome_Cancelled;

    const int playerId = gProfile->GetPrimaryPad();
    telemetryManagerInstance->RecordUpsellResponded(playerId, eSen_UpsellID_FullGame, 1, outcome);
    return 0;
}
int CMinecraftApp::TipsSortFunction(const void* firstTip, const void* secondTip)
{
    const TIPSTRUCT* first = static_cast<const TIPSTRUCT*>(firstTip);
    const TIPSTRUCT* second = static_cast<const TIPSTRUCT*>(secondTip);
    return first->sortValue - second->sortValue;
}
void CMinecraftApp::InitialiseTips()
{
    if (!gRenderer->IsHiDef())
        sGeneralTips[0].tipId = IDS_TIPS_GAMETIP_0;
    std::memset(mTipData, 0, sizeof(mTipData));
    mTipData[0] = _TipData(sGeneralTips, sizeof(sGeneralTips) / sizeof(sGeneralTips[0]));
    mTipData[1] = _TipData(sAlternateTips, sizeof(sAlternateTips) / sizeof(sAlternateTips[0]));
    mTipData[2] = _TipData(sMiniGameTips1, sizeof(sMiniGameTips1) / sizeof(sMiniGameTips1[0]));
    mTipData[3] = _TipData(sMiniGameTips2, sizeof(sMiniGameTips2) / sizeof(sMiniGameTips2[0]));
    mTipData[4] = _TipData(sMiniGameTipsOther, sizeof(sMiniGameTipsOther) / sizeof(sMiniGameTipsOther[0]));
    mTipRotationIndex = 0;
}
void CMinecraftApp::SetAppPaused(bool paused)
{
    mAppPaused = paused;
}
uint8_t CMinecraftApp::GetReallyChangingSessionType()
{
    return mReallyChangingSessionType;
}
eXuiAction CMinecraftApp::GetGlobalXuiAction()
{
    return mGlobalXuiAction;
}
uint32_t CMinecraftApp::GetTMSAction(int32_t player)
{
    return mTMSActions[player];
}
int CMinecraftApp::EthernetDisconnectReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPlayer == NULL)
    {
        gConsoleUIController.NavigateToHomeMenu(false);
        gApp.GetCommerceInstance()->HideStoreIcon();
        int userIndex = static_cast<int>(reinterpret_cast<intptr_t>(context));
        if (userIndex == 0xff)
            userIndex = gProfile->GetPrimaryPad();
        gApp.SetAction(userIndex, eXuiAction_ReturnToMenuAfterDisconnect, NULL);
    }
    else
    {
        gApp.SetAction(minecraft->mPlayer->mUserIndex, eXuiAction_ExitWorldAfterDisconnect, NULL);
    }

    ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
    if (gameMode != NULL)
        gameMode->OnExitedGame();
    return 0;
}
int CMinecraftApp::BannedLevelDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (result != C4JStorage::EMessageResult_Accept)
    {
        eXuiAction action = eXuiAction_RemoveLocalPlayer;
        if (pad == gProfile->GetPrimaryPad())
            action = eXuiAction_ExitWorld;
        app->SetAction(pad, action, NULL);
    }
    return 0;
}
void CMinecraftApp::SetGlobalXuiAction(eXuiAction action)
{
    mGlobalXuiAction = action;
}
uint8_t CMinecraftApp::GetChangingSessionType()
{
    return mChangingSessionType;
}
int CMinecraftApp::UnlockFullSaveReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (result == C4JStorage::EMessageResult_Accept)
    {
        if (gProfile->IsSignedInLive(minecraft->mPlayer.get()->mUserIndex))
            gProfile->DisplayFullVersionPurchase(false, minecraft->mPlayer.get()->mUserIndex, 1);
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(pad, eSen_UpsellID_FullGame, 1, eSen_UpsellOutcome_Cancelled);
    }
    return 0;
}
void CMinecraftApp::SetAutosaveTimerTime()
{
    mAutosaveTime = GetTickCount()
                    + GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_Autosave) * 1000 * 60 * 15;
}
void CMinecraftApp::SetGameStarted(bool started)
{
    mGameStarted = started;
    mAppPaused = !started;
}
uint32_t CMinecraftApp::GetXuiServerAction(int32_t player)
{
    return mXuiServerActions[player];
}
int CMinecraftApp::UnlockFullExitReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);

    if (result == C4JStorage::EMessageResult_Accept)
    {
        if (gProfile->IsSignedInLive(minecraft->mPlayer.get()->mUserIndex))
        {
            gProfile->DisplayFullVersionPurchase(false, minecraft->mPlayer.get()->mUserIndex, 1);
            app->SetAction(minecraft->mPlayer.get()->mUserIndex, eXuiAction_ExitWorldAfterUpsell, NULL);
        }
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(pad, eSen_UpsellID_FullGame, 1, eSen_UpsellOutcome_Cancelled);
        app->SetAction(minecraft->mPlayer.get()->mUserIndex, eXuiAction_ExitWorldAfterUpsell, NULL);
    }
    return 0;
}
void CMinecraftApp::SetChangingSessionType(bool changing)
{
    mChangingSessionType = changing;
}
uint32_t CMinecraftApp::GetDisconnectReason()
{
    return mDisconnectReason;
}
void CMinecraftApp::SetReallyChangingSessionType(bool changing)
{
    mReallyChangingSessionType = changing;
}
int CMinecraftApp::SignoutExitWorldThreadProc(void* context)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();

    Minecraft* minecraft = Minecraft::GetInstance();
    int exitReasonStringId = -1;

    if (!minecraft->isClientSide() && !CGameNetworkManager::sInstance.IsInSession())
    {
        if (context != NULL)
        {
            switch (gApp.GetDisconnectReason())
            {
            case 8:
                exitReasonStringId = IDS_DISCONNECTED_KICKED;
                break;
            case 0xe:
                exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
                break;
            case 0xf:
                exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
                break;
            case 0x14:
                exitReasonStringId = STRING_JOIN_LOCAL_PLAYER_CONTENT_RESTRICTED;
                break;
            case 0x15:
                exitReasonStringId = STRING_JOIN_CONTENT_PRIVILEGE_TOO_RESTRICTIVE;
                break;
            default:
                exitReasonStringId = IDS_DISCONNECTED;
                break;
            }
            minecraft->mProgressRenderer->progressStartNoAbort(exitReasonStringId);
        }
    }
    else
    {
        if (context == NULL)
        {
            exitReasonStringId = IDS_EXITING_GAME;
            minecraft->mProgressRenderer->progressStartNoAbort(exitReasonStringId);
            if (minecraft->mLevels[0] != NULL)
                minecraft->mLevels[0]->disconnect(true);
            if (minecraft->mLevels[1] != NULL)
                minecraft->mLevels[1]->disconnect(true);
        }
        else
        {
            switch (gApp.GetDisconnectReason())
            {
            case 7:
                exitReasonStringId = IDS_DISCONNECTED_FLYING;
                break;
            case 8:
                exitReasonStringId = IDS_DISCONNECTED_KICKED;
                break;
            case 0xe:
                exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
                break;
            case 0xf:
                exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
                break;
            case 0x14:
                exitReasonStringId = STRING_JOIN_LOCAL_PLAYER_CONTENT_RESTRICTED;
                break;
            case 0x15:
                exitReasonStringId = STRING_JOIN_CONTENT_PRIVILEGE_TOO_RESTRICTIVE;
                break;
            default:
                exitReasonStringId = IDS_DISCONNECTED;
                break;
            }
            minecraft->mProgressRenderer->progressStartNoAbort(exitReasonStringId);
            if (minecraft->mLevels[0] != NULL)
                minecraft->mLevels[0]->disconnect(false);
            if (minecraft->mLevels[1] != NULL)
                minecraft->mLevels[1]->disconnect(false);
        }

        MinecraftServer::HaltServer(true);
        CGameNetworkManager::sInstance.LeaveGame(false);
    }

    while (CGameNetworkManager::sInstance.IsInSession())
        Sleep(1);

    while (gConsoleUIController.IsReloadingSkin())
        Sleep(1);

    boost::shared_ptr<Player> emptyPlayer;
    minecraft->setLevel(NULL, exitReasonStringId, emptyPlayer, false, true);
    gApp.mGameRuleManager.unloadCurrentGameRules();
    MinecraftServer::s_haltRequested = false;
    MinecraftServer::s_saveOnExitAnswered = false;
    return 0;
}
int CMinecraftApp::TrialOverReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);

    if (result == C4JStorage::EMessageResult_Accept)
    {
        if (!gProfile->IsSignedInLive(minecraft->mPlayer.get()->mUserIndex))
            app->SetAction(minecraft->mPlayer.get()->mUserIndex, eXuiAction_ExitGame, NULL);
        else
            gProfile->DisplayFullVersionPurchase(false, minecraft->mPlayer.get()->mUserIndex, 1);
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(pad, eSen_UpsellID_FullGame, 1, eSen_UpsellOutcome_Cancelled);
        app->SetAction(minecraft->mPlayer.get()->mUserIndex, eXuiAction_ExitGame, NULL);
    }
    return 0;
}
int CMinecraftApp::UnlockFullInviteReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (result == C4JStorage::EMessageResult_Accept)
    {
        if (gProfile->IsSignedInLive(pad))
            gProfile->DisplayFullVersionPurchase(false, pad, 1);
    }
    else
    {
        telemetryManagerInstance->RecordUpsellResponded(pad, eSen_UpsellID_FullGame, 1, eSen_UpsellOutcome_Cancelled);
    }
    return 0;
}
int CMinecraftApp::ExitAndJoinFromInvite(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
        static_cast<CMinecraftApp*>(context)->SetAction(pad, eXuiAction_ExitAndJoinFromInvite, NULL);
    return 0;
}
int CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
    {
        if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()
            != TexturePackRepository::defaultTexturePack)
        {
            DLCPack* pack = Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getDLCPack();
            if (!pack->hasPurchasedPack())
            {
                int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
                gConsoleUIController.RequestAlertMessage(static_cast<int>(0x3bb3096e), static_cast<int>(0xd52c2e3),
                                                         parameters, 2, static_cast<unsigned int>(pad),
                                                         UIScene_PauseMenu::WarningTrialTexturePackReturned, NULL, NULL,
                                                         true);
                return 0;
            }
        }

        MinecraftServer::s_saveOnExitAnswered = true;
        gMinecraftServer->saveOnExit = true;
        gApp.SetAction(pad, eXuiAction_ExitAndJoinFromInvite, NULL);
    }
    return 0;
}
int CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
    {
        gMinecraftServer->saveOnExit = false;
        MinecraftServer::s_saveOnExitAnswered = true;
        gApp.SetAction(pad, eXuiAction_ExitAndJoinFromInvite, NULL);
    }
    return 0;
}
int CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
    {
        if (Minecraft::GetInstance()->mTexturePackRepository->getSelected()
            != TexturePackRepository::defaultTexturePack)
        {
            DLCPack* pack = Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getDLCPack();
            if (!pack->hasPurchasedPack())
            {
                int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
                gConsoleUIController.RequestAlertMessage(static_cast<int>(0x3bb3096e), static_cast<int>(0xd52c2e3),
                                                         parameters, 2, static_cast<unsigned int>(pad),
                                                         UIScene_PauseMenu::WarningTrialTexturePackReturned, context,
                                                         NULL, true);
                return 0;
            }
        }

        bool saveExists = false;
        gStorage->DoesSaveExist(&saveExists);
        if (!saveExists)
        {
            MinecraftServer::s_saveOnExitAnswered = true;
            gMinecraftServer->saveOnExit = true;
            gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitAndJoinFromInvite, NULL);
        }
        else
        {
            int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
            gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_TITLE_SAVE_GAME),
                                                     static_cast<int>(IDS_CONFIRM_SAVE_GAME), parameters, 2,
                                                     static_cast<unsigned int>(gProfile->GetPrimaryPad()),
                                                     ExitAndJoinFromInviteAndSaveReturned, context, NULL, true);
        }
    }
    else if (result == C4JStorage::EMessageResult_ThirdOption)
    {
        int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_TITLE_DECLINE_SAVE_GAME),
                                                 static_cast<int>(IDS_CONFIRM_DECLINE_SAVE_GAME), parameters, 2,
                                                 static_cast<unsigned int>(gProfile->GetPrimaryPad()),
                                                 ExitAndJoinFromInviteDeclineSaveReturned, context, NULL, true);
    }
    return 0;
}
void CMinecraftApp::SetExtraGameData(unsigned int& options, eGameExtraData type, unsigned int value) const
{
    switch (type)
    {
    case eGameExtraData_Unused:
        options = value == 0 ? options & ~1U : options | 1U;
        break;
    case eGameExtraData_TutorialMode:
        options = value == 0 ? options & ~2U : options | 2U;
        break;
    case eGameExtraData_TrialTexturePack:
        options = value == 0 ? options & ~4U : options | 4U;
        break;
    case eGameExtraData_ModernEnd:
        options = value == 0 ? options & ~8U : options | 8U;
        break;
    case eGameExtraData_SaveBuild:
        options = (options & 0xffffU) | ((value << 16) & 0xffff0000U);
        break;
    case eGameExtraData_SaveVersion:
        options = (options & 0xfffff80fU) | ((value << 4) & 0x7f0U);
        break;
    case eGameExtraData_All:
        options = value;
        break;
    default:
        break;
    }
}
void CMinecraftApp::SetExtraGameData(eGameExtraData type, unsigned int value)
{
    SetExtraGameData(mCurrentImageTextData.extraData, type, value);
}
void CMinecraftApp::SetTutorialMode(bool tutorial, const boost::shared_ptr<Player>& player)
{
    if (!player)
    {
        SetExtraGameData(eGameExtraData_TutorialMode, tutorial ? 1U : 0U);
        mTutorialModes[PlayerUID(0)] = tutorial;
        return;
    }
    Player* tutorialPlayer = player.get();
    PlayerUID onlineXuid = tutorialPlayer->mOnlineXuid;
    PlayerUID uid = onlineXuid != PlayerUID(0) ? onlineXuid : tutorialPlayer->mXuid;
    mTutorialModes[uid] = tutorial;
}
eXuiAction CMinecraftApp::GetXuiAction(int player)
{
    return mXuiActions[player].action;
}
int CMinecraftApp::RemoteSaveThreadProc(void* context)
{
    (void)context;

    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    FireworksRecipe::UseDefaultThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_RemoteSave);
    Block::CreateNewThreadStorage();

    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mProgressRenderer->progressStartNoAbort(static_cast<int>(IDS_PROGRESS_HOST_SAVING));
    minecraft->mProgressRenderer->progressStage(-1);
    minecraft->mProgressRenderer->progressStagePercentage(0);

    while (gApp.GetGameStarted() == 0
           && gApp.GetXuiAction(gProfile->GetPrimaryPad()) == eXuiAction_RemoteSaveInProgress)
    {
        minecraft->tickAllConnections();
        Sleep(100);
    }

    if (gApp.GetXuiAction(gProfile->GetPrimaryPad()) != eXuiAction_RemoteSaveInProgress)
        return ERROR_CANCELLED;

    gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_Idle, NULL);
    gConsoleUIController.UpdatePlayerBasePositions();
    Block::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    return 0;
}
int CMinecraftApp::ExitGameFromRemoteSaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
        gApp.SetAction(pad, eXuiAction_ExitWorld, NULL);
    else
    {
        UIScene* scene = gConsoleUIController.FindScene(EUIScene_FullscreenProgress);
        if (scene != NULL)
            static_cast<UIScene_FullscreenProgress*>(scene)->SetWasCancelled(false);
    }
    return 0;
}
void CMinecraftApp::ExitGameFromRemoteSave(void* context)
{
    unsigned int pad = gProfile->GetPrimaryPad();
    int messageParameters[3];
    messageParameters[0] = IDS_CONFIRM_CANCEL;
    messageParameters[1] = IDS_CONFIRM_OK;
    gConsoleUIController.RequestAlertMessage(static_cast<int>(IDS_EXIT_GAME), static_cast<int>(IDS_CONFIRM_EXIT_GAME),
                                             messageParameters, 2, pad, ExitGameFromRemoteSaveDialogReturned, NULL,
                                             NULL, true);
}
int CMinecraftApp::TexturePackDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    return 0;
}
void CMinecraftApp::SetupMiniGameMusic()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    unsigned int packId = minecraft->mTexturePackRepository->getSelectedId();
    ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
    const unsigned int themeId = masterGameMode->GetCurrentThemeId();
    const bool waitingForMusicStart = masterGameMode->IsWaitingForMusicStartEvent();
    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    SoundEngine* soundEngine = minecraft->mSoundEngine;

    const int miniGameId = miniGame->GetId();
    unsigned int playMiniGameId = static_cast<unsigned int>(miniGameId);

    if (miniGameId == static_cast<int>(eMiniGameId_Battle))
    {
        if (packId < 0x806U)
        {
            if (packId == 0U)
            {
                if (themeId == 0x11U)
                {
                    soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56,
                                                    0x20, 0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
                }
                else
                {
                    soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f,
                                                    0x20, 0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
                }
            }
            else if (packId == 0x801U)
            {
                soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x51, 0x53, 0x20,
                                                0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
            }
            else if (packId == 0x803U)
            {
                soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x45, 0x47, 0x20,
                                                0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
            }
            else
            {
                playMiniGameId = miniGame->GetId();
                goto play_streaming;
            }
        }
        else if (packId == 0x806U || packId == 0x1000806U)
        {
            soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x48, 0x4c, 0x20,
                                            0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
        }
        else if (packId == 0x808U || packId == 0x1000808U)
        {
            soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x4d, 0x50, 0x20,
                                            0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
        }
        else
        {
            playMiniGameId = miniGame->GetId();
            goto play_streaming;
        }

        playMiniGameId = miniGame->GetId();
    }
    else if (miniGameId == static_cast<int>(eMiniGameId_Glide))
    {
        if (packId == 0U)
        {
            if (themeId < 10U)
            {
                if (1U < themeId)
                {
                    if (3U < themeId)
                    {
                        if (themeId != 4U && themeId != 6U)
                        {
                            playMiniGameId = miniGame->GetId();
                            goto play_streaming;
                        }
                        soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54,
                                                        0x56, 0x20, 0x29, 0x2d, 0x30, 0x3e, 0x44, 0x57);
                    }
                    else
                    {
                        soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54,
                                                        0x56, 0x20, 0x29, 0x2a, 0x2c, 0x3e, 0x44, 0x57);
                    }
                }
            }
            else if (themeId < 0xbU)
            {
                soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                                                0x29, 0x2d, 0x30, 0x3e, 0x44, 0x57);
            }
            else if (themeId == 0xbU)
            {
                soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                                                0x29, 0x31, 0x33, 0x3e, 0x44, 0x57);
            }
            else if (themeId == 0xcU)
            {
                soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                                                0x29, 0x34, 0x36, 0x3e, 0x44, 0x57);
            }
            else if (themeId == 0xfU)
            {
                soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                                                0x29, 0x3a, 0x3b, 0x3e, 0x44, 0x57);
            }
            else
            {
                playMiniGameId = miniGame->GetId();
                goto play_streaming;
            }
            playMiniGameId = miniGame->GetId();
        }
        else if (packId == 0x801U)
        {
            soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                                            0x29, 0x37, 0x39, 0x3e, 0x44, 0x57);
            playMiniGameId = miniGame->GetId();
        }
        else if (packId == 0x803U)
        {
            soundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x54, 0x56, 0x20,
                                            0x29, 0x3c, 0x3d, 0x3e, 0x44, 0x57);
            playMiniGameId = miniGame->GetId();
        }
        else
        {
            playMiniGameId = miniGame->GetId();
        }
    }

play_streaming:
    if (playMiniGameId != 0U)
    {
        if (playMiniGameId < 4U)
        {
            soundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, waitingForMusicStart,
                                       ConsoleSoundEngine::eAudioDelayType_None);
            return;
        }
        if (playMiniGameId != static_cast<unsigned int>(eMiniGameId_Build))
        {
            soundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                                       ConsoleSoundEngine::eAudioDelayType_Random);
            return;
        }
    }
    soundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                               ConsoleSoundEngine::eAudioDelayType_None);
}
int CMinecraftApp::GameModeTransitionThreadProc(void* context)
{
    GameModeTransitionParams* params = static_cast<GameModeTransitionParams*>(context);
    const unsigned int texturePackId = params->mTexturePackId;
    const bool waitForRoundStart = params->mWaitForRoundStart;
    delete params;

    Minecraft* minecraft = Minecraft::GetInstance();
    gApp.SetGameStarted(false);
    minecraft->mGameRenderer->DisableUpdateThread(3);

    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    phys::Vec2::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    Block::CreateNewThreadStorage();
    FireworksRecipe::UseDefaultThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_GameModeTransition);

    {
        std::wstring transitionOverride =
            Minecraft::GetInstance()->GetClientMasterGameMode()->GetTransitionTextOverride();

        std::vector<std::wstring> progressStrings;
        if (!transitionOverride.empty())
            progressStrings.push_back(transitionOverride);
        progressStrings.push_back(L10N::GetString(IDS_PROGRESS_DOWNLOADING_TERRAIN));
        progressStrings.push_back(L"");

        minecraft->mProgressRenderer->progressStartNoAbort(progressStrings[0]);
        minecraft->mProgressRenderer->progressStage(progressStrings[1]);
    }

    minecraft->mProgressRenderer->progressStagePercentage(0);
    Sleep(100);
    minecraft->mProgressRenderer->progressStagePercentage(10);

    TexturePack* previousPack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    TexturePack* previousPackWithAudio = NULL;
    if (previousPack->hasAudio())
        previousPackWithAudio = previousPack;

    minecraft->mProgressRenderer->progressStagePercentage(20);

    if (minecraft->mTexturePackRepository->selectTexturePackById(texturePackId, false))
    {
        SetupMiniGameMusic();
        if (previousPack->hasAudio())
        {
            previousPackWithAudio->unloadAudio();
            gStorage->UnmountInstalledDLC("TPACK");
        }
        else if (previousPack != Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack)
        {
            gStorage->UnmountInstalledDLC("TPACK");
        }
    }

    minecraft->mProgressRenderer->progressStagePercentage(30);
    ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
    int progress = 30;
    int progressAccumulator = 0;
    bool exitWaitLoop = false;
    bool setGameStartedWhenReady = true;

    for (;;)
    {
        if ((!waitForRoundStart || exitWaitLoop || !CGameNetworkManager::sInstance.IsInSession()
             || CGameNetworkManager::sInstance.IsLeavingGame())
            && !gConsoleUIController.IsExpectingOrReloadingSkin())
            break;

        masterGameMode->IsRoundStarting();
        gConsoleUIController.IsExpectingOrReloadingSkin();
        CGameNetworkManager::sInstance.IsInSession();
        minecraft->tickAllConnections();
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && minecraft->GetGhostController() != NULL)
            minecraft->GetGhostController()->tick();
        Sleep(100);
        progressAccumulator = (progressAccumulator + 100) % 1000;
        if (progressAccumulator == 0 && progress < 90)
        {
            progress += 10;
            minecraft->mProgressRenderer->progressStagePercentage(progress);
        }

        if (!exitWaitLoop)
        {
            if (!masterGameMode->IsRoundStarting() && !masterGameMode->ServerIsWaitingOnUsLoading())
            {
                if (gApp.IsActionPending(0, eXuiAction_ExitWorld)
                    || (gApp.IsActionPending(0, eXuiAction_ConnectionLost)
                        && !CGameNetworkManager::sInstance.IsLocalGame()))
                {
                    setGameStartedWhenReady = false;
                    exitWaitLoop = true;
                }
            }
            else
            {
                exitWaitLoop = true;
            }
        }
    }

    minecraft->mProgressRenderer->progressStagePercentage(90);

    if (!CGameNetworkManager::sInstance.IsHost())
    {
        const int primaryPad = gProfile->GetPrimaryPad();
        if (!gProfile->IsSignedInLive(primaryPad))
            setGameStartedWhenReady = false;
    }

    if (CGameNetworkManager::sInstance.IsInSession() && !CGameNetworkManager::sInstance.IsLeavingGame()
        && setGameStartedWhenReady)
    {
        gApp.SetGameStarted(true);
    }

    minecraft->mGameRenderer->EnableUpdateThread(3);
    gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_Idle, NULL);

    if (gApp.GetGameStarted() == 0 || !waitForRoundStart)
        goto cleanup;

    for (;;)
    {
        if (!CGameNetworkManager::sInstance.IsInSession() || CGameNetworkManager::sInstance.IsLeavingGame())
        {
            goto cleanup;
        }
        Sleep(100);
        if (masterGameMode->IsRoundStarting() || gApp.IsActionPending(0, eXuiAction_ExitWorld))
        {
            goto cleanup;
        }
        if (gApp.IsActionPending(0, eXuiAction_ConnectionLost) && !CGameNetworkManager::sInstance.IsLocalGame())
        {
            Block::ReleaseThreadStorage();
            ElementStorage::ReleaseThreadStorage();
            return 0;
        }
    }

cleanup:
    Block::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    return 0;
}
int CMinecraftApp::GameModeTransitionComplete(void* context)
{
    gConsoleUIController.setCleanupOnReload(false);
    gConsoleUIController.CloseUIScenes(0xff, false);

    const int primaryPad = gProfile->GetPrimaryPad();
    if (gApp.GetXuiAction(primaryPad) != eXuiAction_ExitWorld && !CGameNetworkManager::sInstance.IsLeavingGame()
        && context == reinterpret_cast<void*>(1))
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
            if (player)
            {
                player.get()->SetWaitingForScoreboard(true);
                gApp.SetAction(playerIndex, eXuiAction_RespawnProgress, NULL);
            }
        }
    }
    return 0;
}
void CMinecraftApp::SetTMSAction(int32_t player, eTMSAction action)
{
    mTMSActions[player] = action;
}
uint8_t CMinecraftApp::IsAppPaused()
{
    return mAppPaused;
}
void CMinecraftApp::UpdateTrialPausedTimer()
{
    mTrialPausedTime += mFrameTime;
}
void CMinecraftApp::setLevelGenerationOptions(LevelGenerationOptions* options)
{
    mGameRuleManager.setLevelGenerationOptions(options);
}
uint8_t CMinecraftApp::DLCInstallProcessCompleted()
{
    return mDLCInstallProcessCompleted;
}
std::wstring CMinecraftApp::getDLCFolderName(EDLC_ID dlcId)
{
    switch (dlcId)
    {
    case EDLC_ID_TutorialWorld:
        return L"Tutorial World";
    case EDLC_ID_SkinPack1:
        return L"Skin Pack 1";
    case EDLC_ID_SkinPack2:
        return L"Skin Pack 2";
    case EDLC_ID_SkinPack3:
        return L"Skin Pack 3";
    case EDLC_ID_SkinPack4:
        return L"Skin Pack 4";
    case EDLC_ID_SummerOfArcadeSkinPack:
        return L"Summer of Arcade Skin Pack";
    case EDLC_ID_HalloweenCharitySkinPack:
        return L"Halloween Charity Skin Pack";
    case EDLC_ID_FestiveSkinPack:
        return L"Festive Skin Pack";
    case EDLC_ID_BirthdaySkinPack:
        return L"Birthday Skin Pack";
    case EDLC_ID_BattleAndBeasts:
        return L"Battle & Beasts";
    case EDLC_ID_SkinPack5:
        return L"Skin Pack 5";
    case EDLC_ID_BattleAndBeasts2:
        return L"Battle & Beasts 2";
    case EDLC_ID_StarWarsClassicSkinPack:
        return L"Star Wars Classic Skin Pack";
    case EDLC_ID_StarWarsPrequelSkinPack:
        return L"Star Wars Prequel Skin Pack";
    case EDLC_ID_SkinPack6:
        return L"Skin Pack 6";
    case EDLC_ID_MarvelAvengers:
        return L"Marvel Avengers";
    case EDLC_ID_MarvelSpiderMan:
        return L"Marvel Spider-Man";
    case EDLC_ID_DoctorWhoSkinsVolume1:
        return L"Doctor Who Skins Volume I";
    case EDLC_ID_TheSimpsons:
        return L"The Simpsons";
    case EDLC_ID_SecondBirthdaySkinPack:
    case EDLC_ID_SecondBirthdaySkinPackAlt:
    case EDLC_ID_SecondBirthdaySkinPackAlt2:
        return L"2nd Birthday Skin Pack";
    case EDLC_ID_MarvelGuardiansOfTheGalaxy:
        return L"Marvel Guardians of the Galaxy";
    case EDLC_ID_StarWarsRebelsSkinPack:
        return L"Star Wars Rebels Skin Pack";
    case EDLC_ID_DoctorWhoSkinsVolume2:
        return L"Doctor Who Skins Volume II";
    case EDLC_ID_BiomeSettlersPack1:
        return L"Biome Settlers Pack 1";
    case EDLC_ID_BiomeSettlersPack2:
        return L"Biome Settlers Pack 2";
    case EDLC_ID_StoryModeSkinPack:
        return L"Story Mode Skin Pack";
    case EDLC_ID_RedstoneSpecialistsSkinPack:
        return L"Redstone Specialists Skin Pack";
    case EDLC_ID_CampfireTalesSkinPack:
        return L"Campfire Tales Skin Pack";
    case EDLC_ID_ThirdBirthdaySkinPack:
        return L"3rd Birthday Skin Pack";
    case EDLC_ID_Minecon2015SkinPack:
        return L"Minecon 2015 Skin Pack";
    case EDLC_ID_FourthBirthdaySkinPack:
    case EDLC_ID_FourthBirthdaySkinPackAlt:
        return L"4th Birthday Skin Pack";
    case EDLC_ID_Minecon2016SkinPack:
        return L"Minecon 2016 Skin Pack";
    case EDLC_ID_FirstBirthdaySkinPack:
        return L"1st Birthday Skin Pack";
    case EDLC_ID_VillainsSkinPack:
        return L"Villains Skin Pack";
    case EDLC_ID_MiniGamesHeroesSkinPack:
        return L"Mini Games Heroes Skin Pack";
    case EDLC_ID_MagicTheGatheringSkinPack:
        return L"Magic: The Gathering Skin Pack";
    case EDLC_ID_StrangersBiomeSettlers3SkinPack:
        return L"Strangers Biome Settlers 3 Skin Pack";
    case EDLC_ID_StrangerThingsSkinPack:
        return L"Stranger Things Skin Pack";
    case EDLC_ID_MineconEarth2017:
        return L"Minecon Earth 2017";
    case EDLC_ID_MoanaCharacterPack:
        return L"Moana Character Pack";
    case EDLC_ID_StarWarsSequelSkinPack:
        return L"Star Wars Sequel Skin Pack";
    case EDLC_ID_FinalFantasyXVSkinPack:
        return L"FINAL FANTASY XV Skin Pack";
    case EDLC_ID_TheIncrediblesSkinPack:
        return L"The Incredibles Skin Pack";
    case EDLC_ID_SoloAStarWarsStoryPack:
        return L"Solo: A Star Wars Story Pack";
    case EDLC_ID_FromTheShadowsSkinPack:
        return L"From The Shadows Skin Pack";
    case EDLC_ID_MassEffect:
        return L"Mass Effect";
    case EDLC_ID_Skyrim:
        return L"Skyrim";
    case EDLC_ID_Halo:
        return L"Halo";
    case EDLC_ID_Festive:
        return L"Festive";
    case EDLC_ID_LittleBigPlanet:
        return L"LittleBigPlanet";
    case EDLC_ID_GreekMythology:
        return L"Greek Mythology";
    case EDLC_ID_NorseMythology:
        return L"Norse Mythology";
    case EDLC_ID_Halloween2015:
        return L"Halloween 2015";
    case EDLC_ID_Mario:
        return L"Mario";
    case EDLC_ID_AdventureTime:
        return L"Adventure Time";
    case EDLC_ID_PiratesOfTheCaribbean:
        return L"Pirates Of The Caribbean";
    case EDLC_ID_EgyptianMythology:
        return L"Egyptian Mythology";
    case EDLC_ID_ChineseMythology:
        return L"Chinese Mythology";
    case EDLC_ID_Fallout:
        return L"Fallout";
    case EDLC_ID_TheNightmareBeforeChristmas:
        return L"The Nightmare Before Christmas";
    case EDLC_ID_Plastic:
        return L"Plastic";
    case EDLC_ID_Candy:
        return L"Candy";
    case EDLC_ID_Fantasy:
        return L"Fantasy";
    case EDLC_ID_HalloweenTexturePack:
        return L"Halloween Texture Pack";
    case EDLC_ID_Natural:
        return L"Natural";
    case EDLC_ID_City:
    case EDLC_ID_CityAlt:
        return L"City";
    case EDLC_ID_Cartoon:
        return L"Cartoon";
    case EDLC_ID_Steampunk:
    case EDLC_ID_SteampunkAlt:
        return L"Steampunk";
    case EDLC_ID_Pattern:
        return L"Pattern";
    case EDLC_ID_SuperCuteTexturePack:
        return L"Super Cute Texture Pack";
    case EDLC_ID_BattleMapPack0:
        return L"Battle Map Pack 0";
    case EDLC_ID_BattleMapPack1:
        return L"Battle Map Pack 1";
    case EDLC_ID_BattleMapPack2:
        return L"Battle Map Pack 2";
    case EDLC_ID_BattleMapPack3:
        return L"Battle Map Pack 3";
    case EDLC_ID_BattleMapPack4:
        return L"Battle Map Pack 4";
    case EDLC_ID_MG02MapPack0:
        return L"MG02 Map Pack 0";
    case EDLC_ID_MG02MapPack1:
        return L"MG02 Map Pack 1";
    case EDLC_ID_MG04TestWorldPack:
        return L"MG04_TestWorldPack";
    case EDLC_ID_HalloweenBattleMap:
        return L"Halloween Battle Map";
    case EDLC_ID_FestiveBattleMap:
        return L"Festive Battle Map";
    case EDLC_ID_FalloutBattleMapPack:
        return L"Fallout Battle Map Pack";
    case EDLC_ID_GlideMapPack0:
        return L"Glide Map Pack 0";
    case EDLC_ID_GlideBeastsTrackPack:
        return L"Glide Beasts Track Pack";
    case EDLC_ID_GlideGiantsTrackPack:
        return L"Glide Giants Track Pack";
    case EDLC_ID_GlideMythsTrackPack:
        return L"Glide Myths Track Pack";
    case EDLC_ID_MG02MapPack2:
        return L"MG02 Map Pack 2";
    case EDLC_ID_MG02MapPack3:
        return L"MG02 Map Pack 3";
    default:
        return std::wstring();
    }
}
std::wstring CMinecraftApp::getRootPath(unsigned int contentType, bool create, bool mounted, std::wstring suffix)
{
    std::wstring path = suffix;
    if (create)
    {
        bool useDLCFolder;
        if (contentType < 0x400)
        {
            if (contentType < 0x222)
                useDLCFolder = (contentType == 4) || (contentType == 0x207) || (contentType == 0x216);
            else
                useDLCFolder = (contentType < 0x227) || (contentType == 0x22c) || (contentType == 0x230)
                               || (contentType == 0x233);
        }
        else if (contentType < 0x4f4)
        {
            if (contentType <= 0x402)
                useDLCFolder = true;
            else if (contentType <= 0x404)
                useDLCFolder = false;
            else if (contentType <= 0x40a)
                useDLCFolder = true;
            else if (contentType == 0x40c)
                useDLCFolder = true;
            else if (contentType <= 0x40d)
                useDLCFolder = false;
            else if (contentType <= 0x40f)
                useDLCFolder = true;
            else
                useDLCFolder = false;
        }
        else
        {
            useDLCFolder = (contentType <= 0x4f5) || (contentType == 0x4f9) || (contentType == 0x804)
                           || (contentType == 0x812);
        }

        if (useDLCFolder)
        {
            const std::wstring name = getDLCFolderName(static_cast<EDLC_ID>(contentType));
            if (contentType == 0x207 || contentType == 0x216 || contentType == 0x804
                || (contentType >= 0x222 && contentType < 0x227) || contentType == 0x22c || contentType == 0x230
                || contentType == 0x233)
                path = std::wstring(L"WiiU\\CU\\DLC\\") + name;
            else
                path = std::wstring(L"WiiU\\DLC\\") + name;
        }

#ifdef MINECRAFT_PC
        if (path == suffix || !File(path).exists())
        {
            DLCPack* pack = mDLCManager.getPackFromPackID(static_cast<int>(contentType));
            if (pack != NULL && File(std::wstring(L"WiiU\\DLC\\") + pack->getName()).exists())
                path = std::wstring(L"WiiU\\DLC\\") + pack->getName();
        }
#endif
        if (!File(path).exists())
            path = suffix;
    }
    return path + (mounted ? std::wstring(L"\\Data\\") : std::wstring(L"\\"));
}
std::wstring CMinecraftApp::getFilePath(unsigned int contentType, std::wstring name, bool mounted, std::wstring suffix)
{
    const std::wstring primary = getRootPath(contentType, true, mounted, suffix) + name;
    if (File(primary).exists())
        return primary;
    return getRootPath(contentType, false, true, suffix) + name;
}
void CMinecraftApp::HandleDLC(DLCPack* pack)
{
    unsigned int numItemsAdded = 0;
    std::vector<std::string> files;
    gStorage->GetMountedDLCFileList("DLCDrive", files);
    if (!files.empty())
    {
        const std::string& filePath = files[0];
        std::wstring widePath(stringtowchararray(filePath));
        std::wstring fileName = File(widePath).getName();
        const unsigned int packId = mDLCManager.retrievePackIDFromDLCDataFile(filePath);
        DLC_INFO* info = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(packId);
        if (info == NULL || info->PackType != 5)
        {
            const unsigned int license = info != NULL ? info->PackID : 0;
            mDLCManager.readDLCDataFile(numItemsAdded, filePath, pack, license, false);
        }
        else
        {
            std::wstring dataPath = getFilePath(packId, fileName, false, L"TPACK:");
            if (!File(dataPath).exists())
                mDLCManager.readDLCDataFile(numItemsAdded, filePath, pack, info->PackID, false);
            else
                mDLCManager.readDLCDataFile(numItemsAdded, dataPath, pack, info->PackID, false);
        }
    }

    if (numItemsAdded == 0)
        mDLCManager.removePack(pack);
}
int CMinecraftApp::DLCMountedCallback(void* context, int result, unsigned int contentType, unsigned int index)
{
    if (contentType != 0)
    {
        ++gApp.mDLCManager.m_numCorruptPacks;
        ++gApp.mDLCInstallIndex;
        gApp.MountNextDLC(result);
        return 0;
    }

    CONTENT_DATA content = *reinterpret_cast<CONTENT_DATA*>(gStorage->GetDLC(gApp.mDLCInstallIndex));
    const std::wstring folderName = convStringToWstring(std::string(content.name));
    DLCPack* existingPack = gApp.mDLCManager.getPack(folderName);
    cNintendoDLCInfo* dlcInfo = static_cast<CConsoleMinecraftApp*>(&gApp)->GetDLCInfoInstance();

    if (existingPack != NULL && !existingPack->IsCorrupt())
    {
        unsigned int licenseMask = dlcInfo->GetDLCPurchased(content.name);
#ifdef MINECRAFT_PC
        if (CStorage::IsLauncherDLC(content))
            licenseMask = 1U;
#endif
        existingPack->SetDLCMountIndex(gApp.mDLCInstallIndex);
        existingPack->SetDLCDeviceID(content.state);
        existingPack->updateLicenseMask(licenseMask);
        gStorage->UnmountInstalledDLC(NULL);
    }
    else
    {
        if (existingPack != NULL)
            gApp.mDLCManager.removePack(existingPack);

        unsigned int licenseMask = dlcInfo->GetDLCPurchased(content.name);
        if (std::wstring(L"Battle & Beasts") == folderName)
            licenseMask = 1U;
        if (std::wstring(L"Battle & Beasts 2") == folderName)
            licenseMask = 1U;
        if (std::wstring(L"Festive") == folderName)
            licenseMask = 1U;
        if (std::wstring(L"Fantasy Texture Pack") == folderName)
            licenseMask = 1U;
        if (std::wstring(L"City Texture Pack") == folderName)
            licenseMask = 1U;
        if (std::wstring(L"Natural Texture Pack") == folderName)
            licenseMask = 1U;
        if (std::wstring(L"Mario") == folderName)
            licenseMask = 1U;
#ifdef MINECRAFT_PC
        if (CStorage::IsLauncherDLC(content))
            licenseMask = 1U;
#endif

        DLCPack* pack = new DLCPack(folderName, licenseMask);
        pack->SetDLCMountIndex(gApp.mDLCInstallIndex);
        pack->SetDLCDeviceID(content.state);
        gApp.mDLCManager.addPack(pack);
#ifdef MINECRAFT_PC
        if (CStorage::IsLauncherDLC(content))
            gApp.mDLCManager.PcGiveLauncherPackId(pack);
#endif
        gApp.HandleDLC(pack);
        gStorage->UnmountInstalledDLC(NULL);
    }

    ++gApp.mDLCInstallIndex;
    gApp.MountNextDLC(result);
    return 0;
}
void CMinecraftApp::MountNextDLC(int32_t pad)
{
    if (mDLCInstallIndex < mDLCInstallCount)
    {
        if (gStorage->MountInstalledDLC(pad, mDLCInstallIndex, DLCMountedCallback, this, NULL) != 0x3e5)
        {
            ++mDLCInstallIndex;
            gApp.MountNextDLC(pad);
        }
    }
    else
    {
        mDLCInstallProcessCompleted = true;
        mDLCInstallPending = false;
        gConsoleUIController.HandleDLCMountingComplete();
    }
}
int CMinecraftApp::DLCInstalledCallback(void* context, int result, int index)
{
    if (result != -1)
    {
        gApp.mDLCInstallCount = result;
        gApp.MountNextDLC(index);
    }
    return 0;
}
uint32_t CMinecraftApp::StartInstallDLCProcess(int32_t pad)
{
    if (gApp.DLCInstallProcessCompleted() != 0 || mDLCInstallPending)
        return 0;
    mDLCInstallCount = 0;
    mDLCInstallIndex = 0;
    gApp.mDLCManager.m_numCorruptPacks = 0;
    mDLCInstallPending = true;
    gStorage->GetInstalledDLC(pad, DLCInstalledCallback, this);
    return 1;
}
std::vector<LevelGenerationOptions*>* CMinecraftApp::getLevelGenerators()
{
    return mGameRuleManager.getLevelGenerators();
}
void CMinecraftApp::ClearTerrainFeaturePosition()
{
    EnterCriticalSection(&mTerrainFeaturePositionsMutex);
    while (!mTerrainFeaturePositions.empty())
    {
        delete mTerrainFeaturePositions.back();
        mTerrainFeaturePositions.pop_back();
    }
    LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
}
void CMinecraftApp::LocaleAndLanguageInit()
{
    mLocaleToString[eMCLang_zhCHT] = L"zh-CHT";
    mLocaleToString[eMCLang_csCS] = L"cs-CS";
    mLocaleToString[eMCLang_laLAS] = L"la-LAS";
    mLocaleToString[eMCLang_enEN] = L"en-EN";
    mLocaleToString[eMCLang_enUS] = L"en-US";
    mLocaleToString[eMCLang_enGB] = L"en-GB";
    mLocaleToString[eMCLang_enIE] = L"en-IE";
    mLocaleToString[eMCLang_enAU] = L"en-AU";
    mLocaleToString[eMCLang_enNZ] = L"en-NZ";
    mLocaleToString[eMCLang_enCA] = L"en-CA";
    mLocaleToString[eMCLang_jaJP] = L"ja-JP";
    mLocaleToString[eMCLang_deDE] = L"de-DE";
    mLocaleToString[eMCLang_deAT] = L"de-AT";
    mLocaleToString[eMCLang_frFR] = L"fr-FR";
    mLocaleToString[eMCLang_frCA] = L"fr-CA";
    mLocaleToString[eMCLang_esES] = L"es-ES";
    mLocaleToString[eMCLang_esMX] = L"es-MX";
    mLocaleToString[eMCLang_itIT] = L"it-IT";
    mLocaleToString[eMCLang_koKR] = L"ko-KR";
    mLocaleToString[eMCLang_ptPT] = L"pt-PT";
    mLocaleToString[eMCLang_ptBR] = L"pt-BR";
    mLocaleToString[eMCLang_ruRU] = L"ru-RU";
    mLocaleToString[eMCLang_nlNL] = L"nl-NL";
    mLocaleToString[eMCLang_fiFI] = L"fi-FI";
    mLocaleToString[eMCLang_svSV] = L"sv-SV";
    mLocaleToString[eMCLang_daDA] = L"da-DA";
    mLocaleToString[eMCLang_noNO] = L"no-NO";
    mLocaleToString[eMCLang_plPL] = L"pl-PL";
    mLocaleToString[eMCLang_trTR] = L"tr-TR";
    mLocaleToString[eMCLang_elEL] = L"el-EL";
    mLocaleToString[eMCLang_zhSG] = L"zh-SG";
    mLocaleToString[eMCLang_zhCN] = L"zh-CN";
    mLocaleToString[eMCLang_zhHK] = L"zh-HK";
    mLocaleToString[eMCLang_zhTW] = L"zh-TW";
    mLocaleToString[eMCLang_nlBE] = L"nl-BE";
    mLocaleToString[eMCLang_daDK] = L"da-DK";
    mLocaleToString[eMCLang_frBE] = L"fr-BE";
    mLocaleToString[eMCLang_frCH] = L"fr-CH";
    mLocaleToString[eMCLang_deCH] = L"de-CH";
    mLocaleToString[eMCLang_nbNO] = L"nb-NO";
    mLocaleToString[eMCLang_enGR] = L"en-GR";
    mLocaleToString[eMCLang_enHK] = L"en-HK";
    mLocaleToString[eMCLang_enSA] = L"en-SA";
    mLocaleToString[eMCLang_enHU] = L"en-HU";
    mLocaleToString[eMCLang_enIN] = L"en-IN";
    mLocaleToString[eMCLang_enIL] = L"en-IL";
    mLocaleToString[eMCLang_enSG] = L"en-SG";
    mLocaleToString[eMCLang_enSK] = L"en-SK";
    mLocaleToString[eMCLang_enZA] = L"en-ZA";
    mLocaleToString[eMCLang_enCZ] = L"en-CZ";
    mLocaleToString[eMCLang_enAE] = L"en-AE";
    mLocaleToString[eMCLang_esAR] = L"es-AR";
    mLocaleToString[eMCLang_esCL] = L"es-CL";
    mLocaleToString[eMCLang_esCO] = L"es-CO";
    mLocaleToString[eMCLang_esUS] = L"es-US";
    mLocaleToString[eMCLang_svSE] = L"sv-SE";
    mLocaleToString[eMCLang_csCZ] = L"cs-CZ";
    mLocaleToString[eMCLang_elGR] = L"el-GR";
    mLocaleToString[eMCLang_nnNO] = L"nn-NO";
    mLocaleToString[eMCLang_skSK] = L"sk-SK";
    mLocaleToString[eMCLang_zhHANS] = L"zh-HANS";
    mLocaleToString[eMCLang_zhHANT] = L"zh-HANT";

    mStringToLocale[L"zh-CHT"] = eMCLang_zhCHT;
    mStringToLocale[L"cs-CS"] = eMCLang_csCS;
    mStringToLocale[L"la-LAS"] = eMCLang_laLAS;
    mStringToLocale[L"en-EN"] = eMCLang_enEN;
    mStringToLocale[L"en-US"] = eMCLang_enUS;
    mStringToLocale[L"en-GB"] = eMCLang_enGB;
    mStringToLocale[L"en-IE"] = eMCLang_enIE;
    mStringToLocale[L"en-AU"] = eMCLang_enAU;
    mStringToLocale[L"en-NZ"] = eMCLang_enNZ;
    mStringToLocale[L"en-CA"] = eMCLang_enCA;
    mStringToLocale[L"ja-JP"] = eMCLang_jaJP;
    mStringToLocale[L"de-DE"] = eMCLang_deDE;
    mStringToLocale[L"de-AT"] = eMCLang_deAT;
    mStringToLocale[L"fr-FR"] = eMCLang_frFR;
    mStringToLocale[L"fr-CA"] = eMCLang_frCA;
    mStringToLocale[L"es-ES"] = eMCLang_esES;
    mStringToLocale[L"es-MX"] = eMCLang_esMX;
    mStringToLocale[L"it-IT"] = eMCLang_itIT;
    mStringToLocale[L"ko-KR"] = eMCLang_koKR;
    mStringToLocale[L"pt-PT"] = eMCLang_ptPT;
    mStringToLocale[L"pt-BR"] = eMCLang_ptBR;
    mStringToLocale[L"ru-RU"] = eMCLang_ruRU;
    mStringToLocale[L"nl-NL"] = eMCLang_nlNL;
    mStringToLocale[L"fi-FI"] = eMCLang_fiFI;
    mStringToLocale[L"sv-SV"] = eMCLang_svSV;
    mStringToLocale[L"da-DA"] = eMCLang_daDA;
    mStringToLocale[L"no-NO"] = eMCLang_noNO;
    mStringToLocale[L"pl-PL"] = eMCLang_plPL;
    mStringToLocale[L"tr-TR"] = eMCLang_trTR;
    mStringToLocale[L"el-EL"] = eMCLang_elEL;
    mStringToLocale[L"zh-SG"] = eMCLang_zhSG;
    mStringToLocale[L"zh-CN"] = eMCLang_zhCN;
    mStringToLocale[L"zh-HK"] = eMCLang_zhHK;
    mStringToLocale[L"zh-TW"] = eMCLang_zhTW;
    mStringToLocale[L"nl-BE"] = eMCLang_nlBE;
    mStringToLocale[L"da-DK"] = eMCLang_daDK;
    mStringToLocale[L"fr-BE"] = eMCLang_frBE;
    mStringToLocale[L"fr-CH"] = eMCLang_frCH;
    mStringToLocale[L"de-CH"] = eMCLang_deCH;
    mStringToLocale[L"nb-NO"] = eMCLang_nbNO;
    mStringToLocale[L"en-GR"] = eMCLang_enGR;
    mStringToLocale[L"en-HK"] = eMCLang_enHK;
    mStringToLocale[L"en-SA"] = eMCLang_enSA;
    mStringToLocale[L"en-HU"] = eMCLang_enHU;
    mStringToLocale[L"en-IN"] = eMCLang_enIN;
    mStringToLocale[L"en-IL"] = eMCLang_enIL;
    mStringToLocale[L"en-SG"] = eMCLang_enSG;
    mStringToLocale[L"en-SK"] = eMCLang_enSK;
    mStringToLocale[L"en-ZA"] = eMCLang_enZA;
    mStringToLocale[L"en-CZ"] = eMCLang_enCZ;
    mStringToLocale[L"en-AE"] = eMCLang_enAE;
    mStringToLocale[L"es-AR"] = eMCLang_esAR;
    mStringToLocale[L"es-CL"] = eMCLang_esCL;
    mStringToLocale[L"es-CO"] = eMCLang_esCO;
    mStringToLocale[L"es-US"] = eMCLang_esUS;
    mStringToLocale[L"sv-SE"] = eMCLang_svSE;
    mStringToLocale[L"cs-CZ"] = eMCLang_csCZ;
    mStringToLocale[L"el-GR"] = eMCLang_elGR;
    mStringToLocale[L"nn-NO"] = eMCLang_nnNO;
    mStringToLocale[L"sk-SK"] = eMCLang_skSK;
    mStringToLocale[L"zh-HANS"] = eMCLang_zhHANS;
    mStringToLocale[L"zh-HANT"] = eMCLang_zhHANT;

    mStringToXcLocale[L"zh-CHT"] = XC_LOCALE_CHINA;
    mStringToXcLocale[L"cs-CS"] = XC_LOCALE_CHINA;
    mStringToXcLocale[L"en-EN"] = XC_LOCALE_UNITED_STATES;
    mStringToXcLocale[L"en-US"] = XC_LOCALE_UNITED_STATES;
    mStringToXcLocale[L"en-GB"] = XC_LOCALE_GREAT_BRITAIN;
    mStringToXcLocale[L"en-IE"] = XC_LOCALE_IRELAND;
    mStringToXcLocale[L"en-AU"] = XC_LOCALE_AUSTRALIA;
    mStringToXcLocale[L"en-NZ"] = XC_LOCALE_NEW_ZEALAND;
    mStringToXcLocale[L"en-CA"] = XC_LOCALE_CANADA;
    mStringToXcLocale[L"ja-JP"] = XC_LOCALE_JAPAN;
    mStringToXcLocale[L"de-DE"] = XC_LOCALE_GERMANY;
    mStringToXcLocale[L"de-AT"] = XC_LOCALE_AUSTRIA;
    mStringToXcLocale[L"fr-FR"] = XC_LOCALE_FRANCE;
    mStringToXcLocale[L"fr-CA"] = XC_LOCALE_CANADA;
    mStringToXcLocale[L"es-ES"] = XC_LOCALE_SPAIN;
    mStringToXcLocale[L"es-MX"] = XC_LOCALE_MEXICO;
    mStringToXcLocale[L"it-IT"] = XC_LOCALE_ITALY;
    mStringToXcLocale[L"ko-KR"] = XC_LOCALE_KOREA;
    mStringToXcLocale[L"pt-PT"] = XC_LOCALE_PORTUGAL;
    mStringToXcLocale[L"pt-BR"] = XC_LOCALE_BRAZIL;
    mStringToXcLocale[L"ru-RU"] = XC_LOCALE_RUSSIAN_FEDERATION;
    mStringToXcLocale[L"nl-NL"] = XC_LOCALE_NETHERLANDS;
    mStringToXcLocale[L"fi-FI"] = XC_LOCALE_FINLAND;
    mStringToXcLocale[L"sv-SV"] = XC_LOCALE_SWEDEN;
    mStringToXcLocale[L"da-DA"] = XC_LOCALE_DENMARK;
    mStringToXcLocale[L"no-NO"] = XC_LOCALE_NORWAY;
    mStringToXcLocale[L"pl-PL"] = XC_LOCALE_POLAND;
    mStringToXcLocale[L"tr-TR"] = XC_LOCALE_TURKEY;
    mStringToXcLocale[L"el-EL"] = XC_LOCALE_GREECE;
    mStringToXcLocale[L"la-LAS"] = XC_LOCALE_LATIN_AMERICA;
    mStringToXcLocale[L"zh-SG"] = XC_LOCALE_SINGAPORE;
    mStringToXcLocale[L"zh-CN"] = XC_LOCALE_CHINA;
    mStringToXcLocale[L"zh-HK"] = XC_LOCALE_HONG_KONG;
    mStringToXcLocale[L"zh-TW"] = XC_LOCALE_TAIWAN;
    mStringToXcLocale[L"nl-BE"] = XC_LOCALE_BELGIUM;
    mStringToXcLocale[L"da-DK"] = XC_LOCALE_DENMARK;
    mStringToXcLocale[L"fr-BE"] = XC_LOCALE_BELGIUM;
    mStringToXcLocale[L"fr-CH"] = XC_LOCALE_SWITZERLAND;
    mStringToXcLocale[L"de-CH"] = XC_LOCALE_SWITZERLAND;
    mStringToXcLocale[L"nb-NO"] = XC_LOCALE_NORWAY;
    mStringToXcLocale[L"en-GR"] = XC_LOCALE_GREECE;
    mStringToXcLocale[L"en-HK"] = XC_LOCALE_HONG_KONG;
    mStringToXcLocale[L"en-SA"] = XC_LOCALE_SAUDI_ARABIA;
    mStringToXcLocale[L"en-HU"] = XC_LOCALE_HUNGARY;
    mStringToXcLocale[L"en-IN"] = XC_LOCALE_INDIA;
    mStringToXcLocale[L"en-IL"] = XC_LOCALE_ISRAEL;
    mStringToXcLocale[L"en-SG"] = XC_LOCALE_SINGAPORE;
    mStringToXcLocale[L"en-SK"] = XC_LOCALE_SLOVAK_REPUBLIC;
    mStringToXcLocale[L"en-ZA"] = XC_LOCALE_SOUTH_AFRICA;
    mStringToXcLocale[L"en-CZ"] = XC_LOCALE_CZECH_REPUBLIC;
    mStringToXcLocale[L"en-AE"] = XC_LOCALE_UNITED_ARAB_EMIRATES;
    mStringToXcLocale[L"ja-IP"] = XC_LOCALE_JAPAN;
    mStringToXcLocale[L"es-AR"] = XC_LOCALE_ARGENTINA;
    mStringToXcLocale[L"es-CL"] = XC_LOCALE_CHILE;
    mStringToXcLocale[L"es-CO"] = XC_LOCALE_COLOMBIA;
    mStringToXcLocale[L"es-US"] = XC_LOCALE_UNITED_STATES;
    mStringToXcLocale[L"sv-SE"] = XC_LOCALE_SWEDEN;
    mStringToXcLocale[L"cs-CZ"] = XC_LOCALE_CZECH_REPUBLIC;
    mStringToXcLocale[L"el-GR"] = XC_LOCALE_GREECE;
    mStringToXcLocale[L"sk-SK"] = XC_LOCALE_SLOVAK_REPUBLIC;
    mStringToXcLocale[L"zh-HANS"] = XC_LOCALE_CHINA;
    mStringToXcLocale[L"zh-HANT"] = XC_LOCALE_CHINA;
}
void CMinecraftApp::HandleButtonPresses()
{
}
bool CMinecraftApp::LoadInventoryMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative)
{
    LocalPlayerMenuInitData* data = new LocalPlayerMenuInitData();
    data->localPlayer = localPlayer;
    data->creative = creative;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_InventoryMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadCreativeMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, bool creative)
{
    LocalPlayerMenuInitData* data = new LocalPlayerMenuInitData();
    data->localPlayer = localPlayer;
    data->creative = creative;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_CreativeMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadEnchantingMenu(int player, boost::shared_ptr<Inventory> inventory, int x, int y, int z,
                                       Level* level, const std::wstring& name)
{
    EnchantingMenuInitData* data = new EnchantingMenuInitData();
    data->inventory = inventory;
    data->level = level;
    data->x = x;
    data->y = y;
    data->z = z;
    data->playerIndex = player;
    data->name = name;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_EnchantingMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadFurnaceMenu(int player, boost::shared_ptr<Inventory> inventory,
                                    boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_FurnaceMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadBrewingStandMenu(int player, boost::shared_ptr<Inventory> inventory,
                                         boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_BrewingStandMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadContainerMenu(int player, boost::shared_ptr<Container> inventory,
                                      boost::shared_ptr<Container> container)
{
    ContainerMenuInitData* data = new ContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    EUIScene scene = EUIScene_ContainerMenu;
    if (data->splitscreen && container->getContainerSize() >= 0x1c)
        scene = EUIScene_ContainerMenuLarge;

    return gConsoleUIController.NavigateToScene(player, scene, data, EUILayer_Scene, EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadTrapMenu(int player, boost::shared_ptr<Container> inventory,
                                 boost::shared_ptr<Container> container)
{
    ContainerPairMenuInitData* data = new ContainerPairMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_DispenserMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadCrafting2x2Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer)
{
    if (GameSettings::GetGameSettings(player, eGameSetting_ClassicCrafting) != 0)
        return static_cast<uint32_t>(LoadInventoryMenu(player, localPlayer, false));

    _CraftingPanelScreenInput* data = new _CraftingPanelScreenInput();
    data->player = localPlayer;
    data->x = 0;
    data->y = 0;
    data->craftingMode = 0;
    data->pad = player;
    data->z = 0;
    data->compactInventory = GetLocalPlayerCount() > 1;
    return gConsoleUIController.NavigateToScene(player, EUIScene_CraftingMenu2x2, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadCrafting3x3Menu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z)
{
    if (GameSettings::GetGameSettings(player, eGameSetting_ClassicCrafting) == 0)
    {
        _CraftingPanelScreenInput* data = new _CraftingPanelScreenInput();
        data->player = localPlayer;
        data->y = y;
        data->craftingMode = 1;
        data->x = x;
        data->pad = player;
        data->z = z;
        data->compactInventory = GetLocalPlayerCount() > 1;
        return gConsoleUIController.NavigateToScene(player, EUIScene_CraftingMenu, data, EUILayer_Scene,
                                                    EUIGroup_PlayerDefault);
    }
    else
    {
        ClassicCraftingMenuInitData* data = new ClassicCraftingMenuInitData();
        data->localPlayer = localPlayer;
        data->y = y;
        data->player = player;
        data->z = z;
        data->fireworks = false;
        data->x = x;
        data->splitscreen = GetLocalPlayerCount() > 1;
        return gConsoleUIController.NavigateToScene(player, EUIScene_ClassicCraftingMenu, data, EUILayer_Scene,
                                                    EUIGroup_PlayerDefault);
    }
}
bool CMinecraftApp::LoadFireworksMenu(int player, boost::shared_ptr<LocalPlayer> localPlayer, int x, int y, int z)
{
    ClassicCraftingMenuInitData* data = new ClassicCraftingMenuInitData();
    data->localPlayer = localPlayer;
    data->fireworks = true;
    data->player = player;
    data->x = x;
    data->y = y;
    data->z = z;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_ClassicCraftingMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadSignEntryMenu(int player, boost::shared_ptr<SignBlockEntity> sign)
{
    SignEntryMenuInitData* data = new SignEntryMenuInitData();
    data->sign = sign;
    data->player = player;

    const bool result = gConsoleUIController.NavigateToScene(player, EUIScene_SignEntryMenu, data, EUILayer_Scene,
                                                             EUIGroup_PlayerDefault);
    delete data;
    return result;
}
bool CMinecraftApp::LoadRepairingMenu(int player, boost::shared_ptr<Inventory> inventory, Level* level, int x, int y,
                                      int z)
{
    RepairingMenuInitData* data = new RepairingMenuInitData();
    data->inventory = inventory;
    data->level = level;
    data->x = x;
    data->y = y;
    data->z = z;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_AnvilMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadTradingMenu(int player, boost::shared_ptr<Inventory> inventory,
                                    boost::shared_ptr<Merchant> merchant, Level* level, const std::wstring& name)
{
    TradingMenuInitData* data = new TradingMenuInitData();
    data->inventory = inventory;
    data->merchant = merchant;
    data->level = level;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_TradingMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadShulkerBoxMenu(int player, boost::shared_ptr<Inventory> inventory,
                                       boost::shared_ptr<Container> container)
{
    return LoadContainerMenu(player, inventory, container);
}
bool CMinecraftApp::LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory,
                                   boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_HopperMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadHopperMenu(int player, boost::shared_ptr<Inventory> inventory,
                                   boost::shared_ptr<MinecartHopper> hopper)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = boost::dynamic_pointer_cast<Container>(hopper);
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_HopperMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadHorseMenu(int player, boost::shared_ptr<Inventory> inventory,
                                  boost::shared_ptr<Container> container, boost::shared_ptr<AbstractHorse> horse)
{
    HorseMenuInitData* data = new HorseMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->horse = horse;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_HorseInventoryMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadBeaconMenu(int player, boost::shared_ptr<Inventory> inventory,
                                   boost::shared_ptr<Container> container)
{
    InventoryContainerMenuInitData* data = new InventoryContainerMenuInitData();
    data->inventory = inventory;
    data->container = container;
    data->player = player;
    data->splitscreen = GetLocalPlayerCount() > 1;

    return gConsoleUIController.NavigateToScene(player, EUIScene_BeaconMenu, data, EUILayer_Scene,
                                                EUIGroup_PlayerDefault);
}
bool CMinecraftApp::LoadWritingBookMenu(int player, not_null_ptr<ItemInstance> item, bool signing)
{
    WritingBookMenuInitData* data = new WritingBookMenuInitData();
    data->item = item;
    data->player = player;
    data->signing = signing;

    const bool result = gConsoleUIController.NavigateToScene(player, EUIScene_WritableBookMenu, data, EUILayer_Scene,
                                                             EUIGroup_PlayerDefault);
    delete data;
    return result;
}
bool CMinecraftApp::IsConsoleChatCensored()
{
    bool chatRestricted = false;
    gProfile->GetChatAndContentRestrictions(0, false, &chatRestricted, NULL, NULL);
#ifdef MINECRAFT_PC
    return false;
#else
    return GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_GameChat) == 0;
#endif
}
uint32_t CMinecraftApp::IsUUIDLocal(const std::wstring& uuid)
{
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        if (localPlayer && localPlayer->getUUID() == uuid)
            return 1;
    }
    return 0;
}
bool CMinecraftApp::GetTutorialMode(PlayerUID uid)
{
    bool tutorial;
    if (mTutorialModes.find(uid) != mTutorialModes.end())
        tutorial = mTutorialModes[uid];
    else
        tutorial = mTutorialModes[PlayerUID(0)];
    return tutorial;
}
bool CMinecraftApp::GetTutorialMode(const boost::shared_ptr<Player>& player)
{
    bool tutorial;
    if (player)
    {
        Player* tutorialPlayer = player.get();
        PlayerUID onlineXuid = tutorialPlayer->mOnlineXuid;
        PlayerUID uid = onlineXuid != PlayerUID(0) ? onlineXuid : tutorialPlayer->mXuid;
        if (mTutorialModes.find(uid) != mTutorialModes.end())
        {
            tutorial = mTutorialModes[uid];
            return tutorial;
        }
    }
    tutorial = mTutorialModes[PlayerUID(0)];
    return tutorial;
}
void CMinecraftApp::SetGameMode(eGameMode mode)
{
    mGameMode = mode;
}
void CMinecraftApp::StartReloadingTexturePack()
{
    const int user = gProfile->GetPrimaryPad();
    eXuiAction action = eXuiAction_ReloadTexturePack;
    if (GetXuiAction(user) == eXuiAction_GameModeTransitionInProgress)
        action = eXuiAction_ReloadTexturePackInTransition;
    SetAction(user, action, NULL);
}
void CMinecraftApp::StaticCtor()
{
}
void* CMinecraftApp::GetXuiServerActionParam(int player)
{
    return mXuiServerActionParams[player];
}
void CMinecraftApp::SetDisconnectReason(DisconnectPacket::eDisconnectReason reason)
{
    mDisconnectReason = reason;
}
uint8_t CMinecraftApp::GetLoadSavesFromFolderEnabled()
{
    return mLoadSavesFromFolderEnabled;
}
uint32_t CMinecraftApp::GetOpacityTimer(int32_t player)
{
    return mOpacityTimer[player];
}
void CMinecraftApp::SetOpacityTimer(int32_t player)
{
    mOpacityTimer[player] = 120;
}
void CMinecraftApp::TickOpacityTimer(int32_t player)
{
    if (mOpacityTimer[player] != 0)
        --mOpacityTimer[player];
}
bool CMinecraftApp::IsLocalMultiplayerAvailable()
{
    unsigned int availablePlayers = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        if (g_pInput->IsPadConnected(pad) || gProfile->IsSignedIn(pad))
            ++availablePlayers;
    }
    return gRenderer->IsHiDef() && availablePlayers > 1;
}
int CMinecraftApp::NotificationsCallback(void* context, unsigned int notification, unsigned int value)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    NOTIFICATION* entry = new NOTIFICATION;
    entry->type = notification;
    entry->value = value;

    if (notification == 1)
    {
        if (gApp.GetGameStarted() != 0 && CGameNetworkManager::sInstance.IsInSession())
        {
            for (int player = 0; player < 4; ++player)
            {
                if (!g_pInput->IsPadConnected(player) && Minecraft::GetInstance()->mLocalPlayers[player] != NULL
                    && !gConsoleUIController.IsPauseMenuDisplayed(player)
                    && !gConsoleUIController.IsSceneInStack(player, EUIScene_EndPoem))
                {
                    gConsoleUIController.CloseUIScenes(player, false);
                    gConsoleUIController.NavigateToScene(player, EUIScene_PauseMenu, NULL, EUILayer_Scene,
                                                         EUIGroup_PlayerDefault);
                    if (player == 3)
                    {
                        app->mNotifications.push_back(entry);
                        return 0;
                    }
                }
            }
        }
    }
    else if (notification == 2)
    {
        gApp.ClearDLCInstalled();
        gConsoleUIController.HandleDLCInstalled(gProfile->GetPrimaryPad());
    }
    else
    {
        app->mNotifications.push_back(entry);
        return 0;
    }

    app->mNotifications.push_back(entry);
    return 0;
}
void CMinecraftApp::SetLiveLinkRequired(bool required)
{
    mLiveLinkRequired = required;
}
std::wstring CMinecraftApp::GetDebugRootPath()
{
    return L"";
}
static bool s_freezePlayers = false;
bool CMinecraftApp::GetFreezePlayers()
{
    return s_freezePlayers;
}
void CMinecraftApp::SetFreezePlayers(bool value)
{
    s_freezePlayers = value;
}
uint32_t CMinecraftApp::DebugSettingsOn()
{
#ifdef MINECRAFT_PC
    return PcDebugMode() ? 1 : 0;
#endif
    return 0;
}
uint32_t CMinecraftApp::DebugArtToolsOn()
{
#ifdef MINECRAFT_PC
    return PcDebugMode() ? 1 : 0;
#endif
    return 0;
}
uint32_t CMinecraftApp::DebugDevToolsOn()
{
    return 0;
}
uint8_t CMinecraftApp::DebugUIFocussed()
{
    return mDebugUIFocussed;
}
void CMinecraftApp::SetDebugUIFocussed(bool focussed)
{
    mDebugUIFocussed = focussed;
    g_pInput->SetMenuDisplayed(gProfile->GetPrimaryPad(), focussed);
}
uint32_t CMinecraftApp::DebugInputCallback(void* context)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    app->mDebugInputEnabled = !app->mDebugInputEnabled;
    return 0;
}
void CMinecraftApp::SetDebugSequence(const char* sequence)
{
    g_pInput->SetDebugSequence(sequence, reinterpret_cast<CInput::InputCallback>(DebugInputCallback), this);
}
uint8_t CMinecraftApp::DLCInstallPending()
{
    return mDLCInstallPending;
}
void CMinecraftApp::SetNewDLCAvailable(bool available)
{
    mNewDLCAvailable = available;
}
void CMinecraftApp::UpsellDLC(int userIndex, DLCPack* pack, int (*callback)(void*, int, int), IUIScene_DLCUpsell* scene)
{
    if (gProfile->IsSignedIn(userIndex) && !gProfile->IsSignedInLive(userIndex))
    {
        scene->SetIgnoreInput(false);
        scene->ShowNotOnlineDialog(userIndex);
    }
}
void CMinecraftApp::StoreLaunchData()
{
}
void CMinecraftApp::ExitGame()
{
}
bool CMinecraftApp::isXuidNotch(PlayerUID uid)
{
    if (mPrimaryPlayerUID != PlayerUID(0) && uid != PlayerUID(0))
        return gProfile->AreXUIDSEqual(uid, mPrimaryPlayerUID) == 1;
    return false;
}
bool CMinecraftApp::isXuidDeadmau5(PlayerUID uid)
{
    if (g_mojangData.find(uid) != g_mojangData.end())
    {
        MOJANG_DATA* data = g_mojangData[uid];
        if (data != NULL && data->field0 == 5)
            return true;
    }
    return false;
}
void CMinecraftApp::AddMemoryTextureFile(const std::wstring& path, unsigned char* data, unsigned int size)
{
    EnterCriticalSection(&mMemoryTextureMutex);
    boost::unordered_map<std::wstring, MEMDATA*>::iterator found = mMemoryTextureFiles.find(path);
    if (found == mMemoryTextureFiles.end())
    {
        MEMDATA* entry = new MEMDATA();
        entry->data = data;
        entry->size = size;
        entry->refCount = 1;
        mMemoryTextureFiles[path] = entry;
    }
    else
    {
        MEMDATA* entry = found->second;
        if (entry->size == 0 && size != 0)
        {
            if (entry->data != NULL)
                delete[] entry->data;
            entry->data = data;
            entry->size = size;
        }
        ++entry->refCount;
    }
    LeaveCriticalSection(&mMemoryTextureMutex);
}
void CMinecraftApp::GetMemFileDetails(const std::wstring& path, unsigned char** data, unsigned int* size)
{
    EnterCriticalSection(&mMemoryTextureMutex);
    boost::unordered_map<std::wstring, MEMDATA*>::iterator found = mMemoryTextureFiles.find(path);
    if (found != mMemoryTextureFiles.end())
    {
        *data = found->second->data;
        *size = found->second->size;
    }
    LeaveCriticalSection(&mMemoryTextureMutex);
}
bool CMinecraftApp::IsFileInMemoryTextures(const std::wstring& path)
{
    EnterCriticalSection(&mMemoryTextureMutex);
    const bool found = mMemoryTextureFiles.find(path) != mMemoryTextureFiles.end();
    LeaveCriticalSection(&mMemoryTextureMutex);
    return found;
}
void CMinecraftApp::GetTPD(int type, unsigned char** data, unsigned int* size)
{
    EnterCriticalSection(&mMemoryTPDMutex);
    boost::unordered_map<int, MEMDATA*>::iterator found = mMemoryTPDFiles.find(type);
    if (found != mMemoryTPDFiles.end())
    {
        *data = found->second->data;
        *size = found->second->size;
    }
    LeaveCriticalSection(&mMemoryTPDMutex);
}
bool CMinecraftApp::DefaultCapeExists()
{
    static const wchar_t kSpecialCapePath[] = L"Special_Cape.png";
    const std::wstring path(kSpecialCapePath, wcslen(kSpecialCapePath));
    EnterCriticalSection(&mMemoryTextureMutex);
    const bool exists = mMemoryTextureFiles.find(path) != mMemoryTextureFiles.end();
    LeaveCriticalSection(&mMemoryTextureMutex);
    return exists;
}
void CMinecraftApp::RemoveLevelFromBannedLevelList(int player, PlayerUID uid, char* levelId)
{
    std::vector<BANNEDLISTDATA*>& bannedList = *mBannedListData[player];
    for (std::vector<BANNEDLISTDATA*>::iterator it = bannedList.begin(); it != bannedList.end();)
    {
        BANNEDLISTDATA* entry = *it;
        if (entry == NULL || !IsEqualXUID(entry->playerUid, uid) || std::strcmp(entry->levelId, levelId) != 0)
        {
            ++it;
            continue;
        }
        telemetryManagerInstance->RecordUnBanLevel(player);
        it = bannedList.erase(it);
    }

    const unsigned int count = static_cast<unsigned int>(bannedList.size());
    if (count * sizeof(BANNEDLISTDATA) != 0)
    {
        BANNEDLISTDATA* data = static_cast<BANNEDLISTDATA*>(operator new[](count * sizeof(BANNEDLISTDATA)));
        for (unsigned int i = 0; i < count; ++i)
            std::memcpy(&data[i], bannedList.at(i), sizeof(BANNEDLISTDATA));
        delete[] data;
    }
}
void CMinecraftApp::SetUniqueMapName(char* name)
{
    std::memcpy(mUniqueMapName, name, sizeof(mUniqueMapName));
}
uint64_t CMinecraftApp::getArchiveFileSize(const std::wstring& path)
{
    TexturePack* selected = NULL;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mTexturePackRepository != NULL)
        selected = minecraft->mTexturePackRepository->getSelected();
    if (selected != NULL && selected->hasData() && selected->getArchiveFile() != NULL
        && selected->getArchiveFile()->hasFile(path))
        return selected->getArchiveFile()->getFileSize(path);
    return mMediaArchive->getFileSize(path);
}
void CMinecraftApp::SetTrialTimerStart()
{
    mTrialTimerStart = mAppTime;
    mTrialPausedTime = 0.0f;
}
double CMinecraftApp::getTrialTimer()
{
    return (mAppTime - mTrialTimerStart) - mTrialPausedTime;
}
uint32_t CMinecraftApp::WarningTrialTexturePackReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept && gProfile->IsSignedIn(pad))
    {
        if (!gProfile->IsSignedInLive(pad))
        {
            int parameter = static_cast<int>(IDS_CONFIRM_OK);
            gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_PRO_NOTONLINE_TITLE),
                                                     static_cast<int>(0xc34c0b77), &parameter, 1,
                                                     static_cast<unsigned int>(pad), NULL, NULL, NULL, true);
        }
        else
        {
            DLCTexturePack* texturePack = static_cast<DLCTexturePack*>(
                Minecraft::GetInstance()->mTexturePackRepository->getSelected());
            DLCPack* pack = texturePack->getDLCInfoParentPack();
            if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(pack->GetPackId()))
            {
                int parameter = static_cast<int>(IDS_CONFIRM_OK);
                gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_DLC_ALREADY_INSTALLED_TITLE),
                                                         static_cast<int>(IDS_DLC_ALREADY_INSTALLED_TEXT), &parameter,
                                                         1, static_cast<unsigned int>(pad), NULL, NULL, NULL, true);
            }
        }
    }
    return 0;
}
uint32_t CMinecraftApp::tipFiltered(int32_t tipId)
{
    return 0;
}
uint32_t CMinecraftApp::GetNextTip(bool firstTipRequest, EMiniGameId miniGame)
{
    if (s_pendingDlcInstallTip && gProfile->IsFullVersion())
    {
        s_pendingDlcInstallTip = false;
        if (gApp.DLCInstallProcessCompleted() == 0 || gApp.mDLCManager.getPackCount(DLCManager::eDLCType_Skin) == 0)
        {
            return IDS_TIPS_GAMETIP_SKINPACKS;
        }
    }

    uint32_t tip;
    if (!firstTipRequest)
    {
        mTipRotationIndex = (mTipRotationIndex + 1) % 3;
        tip = mTipData[mTipRotationIndex == 2 ? 1 : 0].getNextTip();
    }
    else if (miniGame == eMiniGameId_Battle)
        tip = mTipData[2].getNextTip();
    else if (miniGame == eMiniGameId_Tumble)
        tip = mTipData[3].getNextTip();
    else
        tip = mTipData[4].getNextTip();
    return tip;
}
unsigned int CMinecraftApp::GetHTMLColour(eMinecraftColour colour)
{
    return Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getColourTable()->getColour(colour);
}
unsigned int CMinecraftApp::GetHTMLColor(eMinecraftColour colour)
{
    return GetHTMLColour(colour);
}
std::wstring CMinecraftApp::GetSingleActionReplacement(int32_t pad, uint32_t buttons)
{
    std::wstring button = L"";
    std::wstring controller = L"";
    std::wstring size = L"";
    int iconSize = 30;
    bool found = true;
    if ((buttons & 0x1) != 0)
        button = L"ActionBottom";
    else if ((buttons & 0x2) != 0)
        button = L"ActionRight";
    else if ((buttons & 0x4) != 0)
        button = L"ActionLeft";
    else if ((buttons & 0x8) != 0)
        button = L"ActionTop";
    else if ((buttons & 0x30c000) != 0)
        button = L"LeftStick";
    else if ((buttons & 0xf0000) != 0)
        button = L"RightStick";
    else if ((buttons & 0x1000) != 0)
        button = L"DPadLeft";
    else if ((buttons & 0x2000) != 0)
        button = L"DPadRight";
    else if ((buttons & 0x400) != 0)
        button = L"DPadUp";
    else if ((buttons & 0x800) != 0)
        button = L"DPadDown";
    else if ((buttons & 0x800000) != 0)
        button = L"TriggerL";
    else if ((buttons & 0x400000) != 0)
        button = L"TriggerR";
    else if ((buttons & 0x40) != 0)
        button = L"ShoulderR";
    else if ((buttons & 0x80) != 0)
        button = L"ShoulderL";
    else if ((buttons & 0x20) != 0)
        button = L"SystemSelect";
    else if ((buttons & 0x10) != 0)
        button = L"SystemStart";
    else if ((buttons & 0x100) != 0)
        button = L"RightStickButton";
    else if ((buttons & 0x200) != 0)
        button = L"LeftStickButton";
    else
        found = false;

    if (gApp.GetLocalPlayerCount() > 1)
    {
        size = L"_Small";
        iconSize = 20;
    }

    if (!g_pInput->IsPadDRCController(pad) && pad != 4)
    {
        if (g_pInput->IsPadClassicController(pad))
            controller = L"_CTRL_2";
    }
    else
    {
        controller = L"_CTRL_1";
    }

    if (!found)
        return std::wstring(L" ").append(L10N::GetString(IDS_CONTROLS_NONE));

    wchar_t image[128];
    swprintf(image, 128, L"<img src=\"%ls%ls%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>", button.c_str(),
             size.c_str(), controller.c_str(), iconSize, iconSize);
    return image;
}
std::wstring CMinecraftApp::GetActionReplacement(int32_t controllerIndex, uint8_t actionMask)
{
#ifdef MINECRAFT_PC
    if (PcInputGlyphSet() == ePcGlyphSet_Keyboard && controllerIndex == gProfile->GetPrimaryPad())
    {
        std::wstring markup;
        if (PcKeyActionMarkup(actionMask, gApp.GetLocalPlayerCount() > 1 ? 20 : 30, &markup))
            return markup;
    }
#endif
    wchar_t replacement[256];
    uint32_t primaryButton = g_pInput->GetGameJoypadMaps(controllerIndex, actionMask);
    uint32_t secondaryButton = 0;
    if (actionMask >= 0x20 && actionMask < 0x39)
    {
        primaryButton = static_cast<uint32_t>(
            GameSettings::GetInputButton(controllerIndex, false, static_cast<EControllerActions>(actionMask)));
        secondaryButton = static_cast<uint32_t>(
            GameSettings::GetInputButton(controllerIndex, true, static_cast<EControllerActions>(actionMask)));
    }

    if (secondaryButton != 0)
    {
        swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]), L"%ls+%ls",
                 GetSingleActionReplacement(controllerIndex, secondaryButton).c_str(),
                 GetSingleActionReplacement(controllerIndex, primaryButton).c_str());
    }
    else
    {
        swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]), L"%ls",
                 GetSingleActionReplacement(controllerIndex, primaryButton).c_str());
    }
    return std::wstring(replacement);
}
std::wstring CMinecraftApp::GetVKReplacement(int32_t player, uint32_t key, bool fullSize)
{
    std::wstring action = L"";
    std::wstring controller = L"";
    std::wstring size = L"";
    bool found = true;
    if (key >= VK_PAD_LTHUMB_UP && key <= VK_PAD_LTHUMB_DOWNLEFT)
        action = L"LeftStickNavigate";
    else if (key >= VK_PAD_RTHUMB_UP && key <= VK_PAD_RTHUMB_DOWNLEFT)
        action = L"RightStickButton";
    else
    {
        switch (key)
        {
        case VK_PAD_A:
            if (g_pInput->IsCircleCrossSwapped())
                action = L"ActionRight";
            else
                action = L"ActionBottom";
            break;
        case VK_PAD_B:
            if (g_pInput->IsCircleCrossSwapped())
                action = L"ActionBottom";
            else
                action = L"ActionRight";
            break;
        case VK_PAD_X:
            action = L"ActionLeft";
            break;
        case VK_PAD_Y:
            action = L"ActionTop";
            break;
        case VK_PAD_RSHOULDER:
            action = L"ShoulderR";
            break;
        case VK_PAD_LSHOULDER:
            action = L"ShoulderL";
            break;
        case VK_PAD_LTRIGGER:
            action = L"TriggerL";
            break;
        case VK_PAD_RTRIGGER:
            action = L"TriggerR";
            break;
        default:
            found = false;
            break;
        }
    }

    int iconSize = 30;
    if (!fullSize && gApp.GetLocalPlayerCount() > 1)
    {
        size = L"_Small";
        iconSize = 20;
    }

    if (g_pInput->IsPadDRCController(player) || player == 4)
        controller = L"_CTRL_1";
    else if (g_pInput->IsPadClassicController(player))
        controller = L"_CTRL_2";

    if (!found)
        return std::wstring(L" ").append(L10N::GetString(IDS_CONTROLS_NONE));

    wchar_t image[128];
    swprintf(image, 128, L"<img src=\"%ls%ls%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>", action.c_str(),
             size.c_str(), controller.c_str(), iconSize, iconSize);
    return image;
}
std::wstring CMinecraftApp::GetIconReplacement(uint32_t icon)
{
    const int size = gConsoleUIController.getScreenWidth() < 1920.0f ? 22 : 33;
    const wchar_t* format = icon == 4 ? L"<img src=\"Icon_Armor\" align=\"middle\" height=\"%d\" width=\"%d\"/>"
                                      : L"<img src=\"Icon_Shank\" align=\"middle\" height=\"%d\" width=\"%d\"/>";
    wchar_t replacement[128];
    swprintf(replacement, sizeof(replacement) / sizeof(replacement[0]), format, size, size);
    std::wstring result = L"";
    switch (icon)
    {
    case 1:
    case 4:
        result = replacement;
        break;
    case 3:
        result.append(replacement).append(replacement).append(replacement);
        break;
    }
    return result;
}
std::wstring CMinecraftApp::FormatHTMLString(int player, const std::wstring& value, int shadowColour)
{
    std::wstring result = value;
    result = replaceAll(result, L"{*B*}", L"<br />");
    wchar_t colour[64];
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_T1));
    result = replaceAll(result, L"{*T1*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_T2));
    result = replaceAll(result, L"{*T2*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_T3));
    result = replaceAll(result, L"{*T3*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"</font><font color=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_Black));
    result = replaceAll(result, L"{*ETB*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"</font><font color=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_White));
    result = replaceAll(result, L"{*ETW*}", colour);
    result = replaceAll(result, L"{*EF*}", L"</font>");
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_0), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C0*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_1), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C1*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_2), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C2*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_3), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C3*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_4), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C4*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_5), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C5*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_6), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C6*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_7), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C7*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_8), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C8*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_9), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*C9*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_a), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*CA*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_b), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*CB*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_c), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*CC*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_d), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*CD*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_e), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*CE*}", colour);
    swprintf(colour, sizeof(colour) / sizeof(colour[0]), L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eMinecraftColour_HTMLColor_f), static_cast<unsigned int>(shadowColour));
    result = replaceAll(result, L"{*CF*}", colour);
    if (GameSettings::GetGameSettings(player, eGameSetting_SouthPaw) == 0)
    {
        result = replaceAll(result, L"{*CONTROLLER_ACTION_MOVE*}", GetActionReplacement(player, 0x1b));
        result = replaceAll(result, L"{*CONTROLLER_ACTION_LOOK*}", GetActionReplacement(player, 0x1d));
    }
    else
    {
        result = replaceAll(result, L"{*CONTROLLER_ACTION_MOVE*}", GetActionReplacement(player, 0x1d));
        result = replaceAll(result, L"{*CONTROLLER_ACTION_LOOK*}", GetActionReplacement(player, 0x1b));
    }
    result = replaceAll(result, L"{*CONTROLLER_MENU_NAVIGATE*}", GetVKReplacement(player, VK_PAD_LTHUMB_LEFT, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_A*}", GetVKReplacement(player, VK_PAD_A, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_B*}", GetVKReplacement(player, VK_PAD_B, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_X*}", GetVKReplacement(player, VK_PAD_X, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_Y*}", GetVKReplacement(player, VK_PAD_Y, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_LB*}", GetVKReplacement(player, VK_PAD_LSHOULDER, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_RB*}", GetVKReplacement(player, VK_PAD_RSHOULDER, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_LS*}", GetVKReplacement(player, VK_PAD_LTHUMB_UP, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_RS*}", GetVKReplacement(player, VK_PAD_RTHUMB_UP, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_LT*}", GetVKReplacement(player, VK_PAD_LTRIGGER, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_RT*}", GetVKReplacement(player, VK_PAD_RTRIGGER, false));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_JUMP*}", GetActionReplacement(player, 0x20));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_SNEAK*}", GetActionReplacement(player, 0x27));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_USE*}", GetActionReplacement(player, 0x21));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_ACTION*}", GetActionReplacement(player, 0x22));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_LEFT_SCROLL*}", GetActionReplacement(player, 0x23));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_RIGHT_SCROLL*}", GetActionReplacement(player, 0x24));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_INVENTORY*}", GetActionReplacement(player, 0x25));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_CRAFTING*}", GetActionReplacement(player, 0x28));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_DROP*}", GetActionReplacement(player, 0x26));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_CAMERA*}", GetActionReplacement(player, 0x29));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_MENU_PAGEDOWN*}", GetActionReplacement(player, 9));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_DISMOUNT*}", GetActionReplacement(player, 0x27));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_DPAD_UP*}", GetActionReplacement(player, 0x2c));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_DPAD_DOWN*}", GetActionReplacement(player, 0x2d));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_DPAD_RIGHT*}", GetActionReplacement(player, 0x2b));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_DPAD_LEFT*}", GetActionReplacement(player, 0x2a));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_PICKBLOCK*}", GetActionReplacement(player, 0x2f));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_SPECTATE_PREV*}", GetActionReplacement(player, 0x30));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_SPECTATE_NEXT*}", GetActionReplacement(player, 0x31));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_SPECTATE_NOISE*}", GetActionReplacement(player, 0x32));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_SPECTATE_CANCEL*}", GetActionReplacement(player, 0x33));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_READY*}", GetActionReplacement(player, 0x34));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_VOTESELECT*}", GetActionReplacement(player, 0x35));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_RESTART_SECTION*}", GetActionReplacement(player, 0x36));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_RESTART_RACE*}", GetActionReplacement(player, 0x37));
    result = replaceAll(result, L"{*CONTROLLER_ACTION_LOOK_BEHIND*}", GetActionReplacement(player, 0x38));
    result = replaceAll(result, L"{*ICON_SHANK_01*}", GetIconReplacement(1));
    result = replaceAll(result, L"{*ICON_SHANK_03*}", GetIconReplacement(3));
    result = replaceAll(result, L"{*ICON_ARMOR_01*}", GetIconReplacement(4));
    result = replaceAll(result, L"{*CONTROLLER_VK_START*}", GetVKReplacement(player, VK_PAD_START, false));
    result = replaceAll(result, L"{*CONTROLLER_VK_BACK*}", L10N::GetString(IDS_BACK_BUTTON));
    result = replaceAll(result, L"{*PLATFORM_NAME*}", L10N::GetString(IDS_PLATFORM_NAME));
    const unsigned int language = XGetLanguage();
    if (language == XC_LANGUAGE_JAPANESE || language == XC_LANGUAGE_KOREAN || language == XC_LANGUAGE_TCHINESE)
        result = replaceAll(result, L"&nbsp;", L"");
    return result;
}
float CMinecraftApp::getAppTime()
{
    return mAppTime;
}
void CMinecraftApp::AddTerrainFeaturePosition(_eTerrainFeatureType type, int x, int z)
{
    EnterCriticalSection(&mTerrainFeaturePositionsMutex);
    for (std::vector<FEATURE_DATA*>::const_iterator it = mTerrainFeaturePositions.begin();
         it != mTerrainFeaturePositions.end(); ++it)
    {
        if ((*it)->type == static_cast<int>(type) && (*it)->x == x && (*it)->z == z)
        {
            LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
            return;
        }
    }
    FEATURE_DATA* data = new FEATURE_DATA;
    data->type = static_cast<int>(type);
    data->x = x;
    data->z = z;
    mTerrainFeaturePositions.push_back(data);
    LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
}
uint32_t CMinecraftApp::GetTerrainFeaturePosition(_eTerrainFeatureType type, int* x, int* z)
{
    EnterCriticalSection(&mTerrainFeaturePositionsMutex);
    for (std::vector<FEATURE_DATA*>::const_iterator it = mTerrainFeaturePositions.begin();
         it != mTerrainFeaturePositions.end(); ++it)
    {
        if ((*it)->type == static_cast<int>(type))
        {
            *x = (*it)->x;
            *z = (*it)->z;
            LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
            return 1;
        }
    }
    LeaveCriticalSection(&mTerrainFeaturePositionsMutex);
    return 0;
}
MOJANG_DATA* CMinecraftApp::GetMojangDataForXuid(PlayerUID uid)
{
    return g_mojangData[uid];
}
uint8_t CMinecraftApp::GetBanListCheck(int32_t player)
{
    return mBanListCheck[player];
}
bool CMinecraftApp::AutosaveDue()
{
#ifdef MINECRAFT_PC
    return static_cast<int>(GetTickCount() - mAutosaveTime) > 0;
#else
    return mAutosaveTime < GetTickCount();
#endif
}
uint32_t CMinecraftApp::SecondsToAutosave()
{
    return (mAutosaveTime - GetTickCount()) / 1000;
}
uint32_t CMinecraftApp::GetExtraGameData(unsigned int& options, eGameExtraData type) const
{
    switch (type)
    {
    case eGameExtraData_Unused:
        return options & 1U;
    case eGameExtraData_TutorialMode:
        return (options >> 1) & 1U;
    case eGameExtraData_TrialTexturePack:
        return (options >> 2) & 1U;
    case eGameExtraData_ModernEnd:
        return (options >> 3) & 1U;
    case eGameExtraData_SaveBuild:
        return (options >> 16) & 0xffffU;
    case eGameExtraData_SaveVersion:
        return (options & 0x7f0U) >> 4;
    case eGameExtraData_All:
        return options;
    default:
        return 0;
    }
}
uint32_t CMinecraftApp::GetExtraGameData(eGameExtraData type)
{
    return GetExtraGameData(mCurrentImageTextData.extraData, type);
}
void CMinecraftApp::SetResetNether(bool reset)
{
    mResetNether = reset;
}
uint8_t CMinecraftApp::GetResetNether()
{
    return mResetNether;
}
void CMinecraftApp::SetResetEnd(bool reset)
{
    mResetEnd = reset;
}
uint8_t CMinecraftApp::GetResetEnd()
{
    return mResetEnd;
}
uint32_t CMinecraftApp::CanRecordStatsAndAchievements(Stat* stat, arrayWithLength<unsigned char> players)
{
    (void)players;

    bool isTutorial = false;
    if (Minecraft::GetInstance() != NULL && Minecraft::GetInstance()->isTutorial())
        isTutorial = true;

    const bool isCustomGame = Minecraft::GetInstance()->GetMiniGame()->IsCustomGame();
    bool allowMiniGameAward = true;
    if (stat->isAchievement())
    {
        const unsigned int achievementId = static_cast<Achievement*>(stat)->getAchievementID();
        if (achievementId >= eAward_TheStudent
            && (achievementId <= eAward_Mine
                || (achievementId >= eAward_BackFromTheDead && achievementId <= eAward_WillLikeNoOther)))
        {
            allowMiniGameAward = false;
            if (!isCustomGame)
            {
                if (CGameNetworkManager::sInstance.IsInPublicJoinableGame()
                    || Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
                    allowMiniGameAward = true;
            }
        }
    }

    bool canRecord = true;
    if ((gApp.GetGameHostOption(eGameHostOption_HasBeenInCreative) != 0
         || gApp.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0
         || gApp.GetGameHostOption(eGameHostOption_HostCanChangeHunger) != 0
         || gApp.GetGameHostOption(eGameHostOption_HostCanFly) != 0
         || gApp.GetGameHostOption(eGameHostOption_WasntSaveOwner) != 0
         || gApp.GetGameHostOption(eGameHostOption_MobGriefing) == 0
         || gApp.GetGameHostOption(eGameHostOption_KeepInventory) != 0
         || gApp.GetGameHostOption(eGameHostOption_DoMobSpawning) == 0
         || gApp.GetGameHostOption(eGameHostOption_DoWeatherCycle) == 0
         || (gApp.GetGameHostOption(eGameHostOption_DoDaylightCycle) == 0 && !isTutorial))
        && (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0 || !allowMiniGameAward))
    {
        canRecord = false;
    }

    return canRecord;
}
void CMinecraftApp::SetBaseSaveName(const std::wstring& name)
{
    if (name.empty())
    {
        mCurrentImageTextData.flags &= ~PngTagData::eFlag_BaseSaveName;
        std::memset(mCurrentImageTextData.baseSaveName, 0, sizeof(mCurrentImageTextData.baseSaveName));
        return;
    }
    mCurrentImageTextData.flags |= PngTagData::eFlag_BaseSaveName;
    std::memset(mCurrentImageTextData.baseSaveName, 0, sizeof(mCurrentImageTextData.baseSaveName));
    std::strncpy(mCurrentImageTextData.baseSaveName, wstringtochararray(name),
                 sizeof(mCurrentImageTextData.baseSaveName));
}
void CMinecraftApp::SetCurrentImageTextData(const PngTagData* data)
{
    if (data != 0)
        mCurrentImageTextData = *data;
}
void CMinecraftApp::CurrentImageTextData(PngTagData* data)
{
    *data = mCurrentImageTextData;
    if (gMinecraftServer != NULL && gMinecraftServer->mLevels.data != NULL && gMinecraftServer->mLevels.data[0] != NULL)
    {
        data->flags |= PngTagData::eFlag_Seed;
        data->seed = gMinecraftServer->mLevels.data[0]->getLevelData()->getSeed();
    }
    data->flags |= PngTagData::eFlag_HostOptions;
    data->hostOptions = GetGameHostOption(eGameHostOption_All);
    data->flags |= PngTagData::eFlag_ExtraData;
    data->extraData = GetExtraGameData(eGameExtraData_All);
    data->flags |= PngTagData::eFlag_TexturePack;
    data->texturePack = Minecraft::GetInstance()->getCurrentTexturePackId();
}
void CMinecraftApp::ParseImageTextData(const unsigned char* image, unsigned int size, PngTagData* data)
{
    std::memset(data, 0, sizeof(PngTagData));
    const char* cursor = reinterpret_cast<const char*>(image);
    const char* end = cursor + size;

    while (cursor < end)
    {
        char key[80];
        std::memset(key, 0, sizeof(key));
        char* keyOut = key;
        const char* keyLimit = cursor + 999;
        unsigned int keyLength = 0;
        while (*cursor != '\0' && cursor < keyLimit && keyLength < 0x4fU)
        {
            *keyOut++ = *cursor++;
            ++keyLength;
        }
        ++cursor;

        if (std::strcmp(key, "4J_SEED") == 0)
        {
            char seedText[22];
            std::memset(seedText, 0, sizeof(seedText));
            std::strncpy(seedText, cursor, 0x16);
            const char* seedEnd = seedText;
            while (*seedEnd != '\0')
                ++seedEnd;
            const unsigned int seedLength = static_cast<unsigned int>(seedEnd - seedText);
            if (seedLength > 0x16U)
            {
                cursor += 0x17;
            }
            else
            {
                cursor += seedLength + 1;
            }
            data->flags |= PngTagData::eFlag_Seed;
            const long long seed = std::strtoll(seedText, NULL, 10);
            data->seed = seed;
        }
        else if (std::strcmp(key, "4J_HOSTOPTIONS") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= PngTagData::eFlag_HostOptions;
            unsigned int parsedValue = 0;
            std::stringstream valueStream;
            valueStream << valueText;
            valueStream >> std::hex >> parsedValue;
            data->hostOptions = parsedValue;
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_TEXTUREPACK") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= PngTagData::eFlag_TexturePack;
            unsigned int parsedValue = 0;
            std::stringstream valueStream;
            valueStream << valueText;
            valueStream >> std::hex >> parsedValue;
            data->texturePack = parsedValue;
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_EXTRADATA") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= PngTagData::eFlag_ExtraData;
            unsigned int parsedValue = 0;
            std::stringstream valueStream;
            valueStream << valueText;
            valueStream >> std::hex >> parsedValue;
            data->extraData = parsedValue;
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_EXPLOREDCHUNKS") == 0)
        {
            char valueText[41];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 0x28U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= PngTagData::eFlag_ExploredChunks;
            std::sscanf(valueText, "%08x%08x%08x%08x%08x", &data->exploredChunks[0], &data->exploredChunks[1],
                        &data->exploredChunks[2], &data->exploredChunks[3], &data->exploredChunks[4]);
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_BASESAVENAME") == 0)
        {
            char valueText[21];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 0x14U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= PngTagData::eFlag_BaseSaveName;
            std::strncpy(data->baseSaveName, valueText, sizeof(data->baseSaveName));
            if (cursor < end)
                ++cursor;
        }
        else if (std::strcmp(key, "4J_#LOADS") == 0)
        {
            char valueText[12];
            std::memset(valueText, 0, sizeof(valueText));
            unsigned int valueLength = 0;
            while (*cursor != '\0' && cursor < end && valueLength < 8U)
            {
                valueText[valueLength++] = *cursor++;
            }
            data->flags |= PngTagData::eFlag_NumLoads;
            unsigned int parsedValue = 0;
            std::stringstream valueStream;
            valueStream << valueText;
            valueStream >> std::hex >> parsedValue;
            data->numLoads = parsedValue;
            if (cursor < end)
                ++cursor;
        }
    }
}
void CMinecraftApp::GetImageTextData(const unsigned char* image, unsigned int size, PngTagData* data)
{
    static const unsigned char signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    if (std::memcmp(image, signature, sizeof(signature)) != 0)
        return;
    unsigned int offset = sizeof(signature);
    while (offset < size)
    {
        unsigned int chunkSize = *(const unsigned int*)(image + offset);
        unsigned int chunkType = *(const unsigned int*)(image + offset + 4);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
        System::ReverseULONG(&chunkSize);
        System::ReverseULONG(&chunkType);
#endif
        if (chunkType == PNG_CHUNK_TYPE_TEXT)
            ParseImageTextData(image + offset + 8, chunkSize, data);
        offset += chunkSize + 12;
    }
}
unsigned int CMinecraftApp::CreateImageTextData(unsigned char* image, const PngTagData* data)
{
    char* output = static_cast<char*>(static_cast<void*>(image));
    unsigned int length = 0;

    if ((data->flags & PngTagData::eFlag_Seed) != 0)
    {
        std::strcpy(output + length, "4J_SEED");
        length += 8;
        std::sprintf(output + length, "%lld", data->seed);
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    std::strcpy(output + length, "4J_HOSTOPTIONS");
    length += 15;
    std::sprintf(output + length, "%x", data->hostOptions);
    length += static_cast<unsigned int>(std::strlen(output + length)) + 1;

    std::strcpy(output + length, "4J_TEXTUREPACK");
    length += 15;
    std::sprintf(output + length, "%x", data->texturePack);
    length += static_cast<unsigned int>(std::strlen(output + length)) + 1;

    std::strcpy(output + length, "4J_EXTRADATA");
    length += 13;
    std::sprintf(output + length, "%x", data->extraData);
    length += static_cast<unsigned int>(std::strlen(output + length)) + 1;

    if ((data->flags & PngTagData::eFlag_ExploredChunks) != 0)
    {
        std::strcpy(output + length, "4J_EXPLOREDCHUNKS");
        length += 18;
        char exploredChunks[41];
        std::memset(exploredChunks, 0, sizeof(exploredChunks));
        std::sprintf(exploredChunks, "%08x%08x%08x%08x%08x", data->exploredChunks[0], data->exploredChunks[1],
                     data->exploredChunks[2], data->exploredChunks[3], data->exploredChunks[4]);
        std::strcpy(output + length, exploredChunks);
        length += static_cast<unsigned int>(std::strlen(exploredChunks)) + 1;
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    if ((data->flags & PngTagData::eFlag_BaseSaveName) != 0)
    {
        std::strcpy(output + length, "4J_BASESAVENAME");
        length += 16;
        std::strcpy(output + length, data->baseSaveName);
        length += static_cast<unsigned int>(std::strlen(data->baseSaveName)) + 1;
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    if ((data->flags & PngTagData::eFlag_NumLoads) != 0)
    {
        std::strcpy(output + length, "4J_#LOADS");
        length += 10;
        std::sprintf(output + length, "%x", data->numLoads);
        length += static_cast<unsigned int>(std::strlen(output + length)) + 1;
    }

    return length - 1;
}
void CMinecraftApp::processSchematics(LevelChunk* chunk)
{
    mGameRuleManager.processSchematics(chunk);
}
void CMinecraftApp::processSchematicsLighting(LevelChunk* chunk)
{
    mGameRuleManager.processSchematicsLighting(chunk);
}
void CMinecraftApp::loadDefaultGameRules()
{
    mGameRuleManager.loadDefaultGameRules();
}
LevelRules* CMinecraftApp::getLevelRules()
{
    return mGameRuleManager.mLevelRules;
}
LevelRuleset* CMinecraftApp::getGameRuleDefinitions()
{
    return mGameRuleManager.getGameRuleDefinitions();
}
LevelGenerationOptions* CMinecraftApp::getLevelGenerationOptions()
{
    return mGameRuleManager.getLevelGenerationOptions();
}
const wchar_t* CMinecraftApp::GetGameRulesString(const std::wstring& key)
{
    return mGameRuleManager.GetGameRulesString(key);
}
bool CMinecraftApp::ShouldCollideWith(Block* block)
{
    LevelGenerationOptions* options = getLevelGenerationOptions();
    if (options == NULL)
        return true;
    return options->getRequiredGameRules()->ShouldCollideWith(block);
}
void CMinecraftApp::UpdatePlayerInfo(uint8_t networkSmallId, int16_t playerColour, uint32_t privileges,
                                     uint8_t gameType)
{
    for (unsigned int i = 0; i < sizeof(mPlayerIds); ++i)
    {
        if (mPlayerIds[i] == networkSmallId)
        {
            mPlayerIds[i] = 0;
            mPlayerPrivileges[i] = 0;
            mPlayerGameTypes[i] = 0;
        }
    }
    if (playerColour >= 0 && playerColour < static_cast<int>(sizeof(mPlayerIds)))
    {
        const unsigned int slot = static_cast<unsigned int>(playerColour);
        mPlayerIds[slot] = networkSmallId;
        mPlayerPrivileges[slot] = privileges;
        mPlayerGameTypes[slot] = gameType;
    }
}
uint32_t CMinecraftApp::GetPlayerColour(uint8_t networkSmallId)
{
    for (unsigned int i = 0; i < sizeof(mPlayerIds); ++i)
        if (mPlayerIds[i] == networkSmallId)
            return i;
    return 0xffffffffU;
}
uint32_t CMinecraftApp::GetPlayerPrivileges(uint8_t networkSmallId)
{
    for (unsigned int i = 0; i < sizeof(mPlayerIds); ++i)
        if (mPlayerIds[i] == networkSmallId)
            return mPlayerPrivileges[i];
    return 0;
}
uint8_t CMinecraftApp::GetPlayerGameType(uint8_t networkSmallId)
{
    for (unsigned int i = 0; i < sizeof(mPlayerIds); ++i)
        if (mPlayerIds[i] == networkSmallId)
            return mPlayerGameTypes[i];
    return 0;
}
std::wstring CMinecraftApp::getEntityName(uint64_t entityType, int32_t auxValue)
{
    if (entityType == EntityType::DOLPHIN)
        return L10N::GetString(IDS_DOLPHIN);
    if (entityType == EntityType::PHANTOM)
        return L10N::GetString(IDS_PHANTOM);
    if (entityType == EntityType::PUFFERFISH)
        return L10N::GetString(IDS_PUFFERFISH);
    if (entityType == EntityType::ZOMBIE_VILLAGER)
        return L10N::GetString(IDS_ZOMBIE_VILLAGER);
    if (entityType == EntityType::VINDICATION_ILLAGER)
        return L10N::GetString(IDS_VINDICATION_ILLAGER);
    if (entityType == EntityType::VEX)
        return L10N::GetString(IDS_VEX);
    if (entityType == EntityType::EVOCATION_FANGS)
        return L"";
    if (entityType == EntityType::EVOCATION_ILLAGER)
        return L10N::GetString(IDS_EVOCATION_ILLAGER);
    if (entityType == EntityType::PARROT)
        return L10N::GetString(IDS_PARROT);
    if (entityType == EntityType::SHULKER)
        return L10N::GetString(IDS_SHULKER);
    if (entityType == EntityType::POLAR_BEAR)
        return L10N::GetString(IDS_POLARBEAR);
    if (entityType == EntityType::RABBIT)
        return L10N::GetString(IDS_RABBIT);
    if (entityType == EntityType::ELDER_GUARDIAN)
        return L10N::GetString(IDS_ELDER_GUARDIAN);
    if (entityType == EntityType::GUARDIAN)
        return L10N::GetString(IDS_GUARDIAN);
    if (entityType == EntityType::ENDERMITE)
        return L10N::GetString(IDS_ENDERMITE);
    if (entityType == EntityType::BAT)
        return L10N::GetString(IDS_BAT);
    if (entityType == EntityType::WITHER_BOSS)
        return L10N::GetString(IDS_WITHER);
    if (entityType == EntityType::WITCH)
        return L10N::GetString(IDS_WITCH);
    if (entityType == EntityType::DONKEY)
        return L10N::GetString(IDS_DONKEY);
    if (entityType == EntityType::MULE)
        return L10N::GetString(IDS_MULE);
    if (entityType == EntityType::LLAMA_SPIT)
        return L"";
    if (entityType == EntityType::LLAMA)
        return L10N::GetString(IDS_LLAMA);
    if (entityType == EntityType::HORSE)
        return L10N::GetString(IDS_HORSE);
    if (entityType == EntityType::VILLAGER_GOLEM)
        return L10N::GetString(IDS_IRONGOLEM);
    if (entityType == EntityType::MAGMA_CUBE)
        return L10N::GetString(IDS_LAVA_SLIME);
    if (entityType == EntityType::BLAZE)
        return L10N::GetString(IDS_BLAZE);
    if (entityType == EntityType::ENDER_DRAGON)
        return L10N::GetString(IDS_ENDERDRAGON);
    if (entityType == EntityType::TIPPABLE_ARROW || entityType == EntityType::ARROW)
        return L10N::GetString(IDS_ITEM_ARROW);
    if (entityType == EntityType::SLIME)
        return L10N::GetString(IDS_SLIME);
    if (entityType == EntityType::GHAST)
        return L10N::GetString(IDS_GHAST);
    if (entityType == EntityType::SILVERFISH)
        return L10N::GetString(IDS_SILVERFISH);
    if (entityType == EntityType::ENDER_MAN)
        return L10N::GetString(IDS_ENDERMAN);
    if (entityType == EntityType::PIG_ZOMBIE)
        return L10N::GetString(IDS_PIGZOMBIE);
    if (entityType == EntityType::DROWNED)
        return L10N::GetString(IDS_DROWNED);
    if (entityType == EntityType::HUSK)
        return L10N::GetString(IDS_HUSK);
    if (entityType == EntityType::ZOMBIE)
        return L10N::GetString(IDS_ZOMBIE);
    if (entityType == EntityType::CAVE_SPIDER)
        return L10N::GetString(IDS_CAVE_SPIDER);
    if (entityType == EntityType::SPIDER)
        return L10N::GetString(IDS_SPIDER);
    if (entityType == EntityType::STRAY)
        return L10N::GetString(IDS_STRAY_SKELETON);
    if (entityType == EntityType::WITHER_SKELETON)
        return L10N::GetString(IDS_WITHER_SKELETON);
    if (entityType == EntityType::SKELETON)
        return L10N::GetString(IDS_SKELETON);
    if (entityType == EntityType::CREEPER)
        return L10N::GetString(IDS_CREEPER);
    if (entityType == EntityType::WOLF)
        return L10N::GetString(IDS_WOLF);
    return L"";
}
int CMinecraftApp::DLCOffersReturned(void* context, unsigned int offerValue, unsigned int contentId, int status)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    EnterCriticalSection(&app->mDLCDownloadQueueMutex);
    for (std::vector<_DLCRequest*>::iterator it = app->mDLCDownloadQueue.begin(); it != app->mDLCDownloadQueue.end();
         ++it)
    {
        if ((*it)->mContentId == contentId)
        {
            (*it)->mStatus = 2U;
            app->mDLCOffersRetrievedValue = offerValue;
            break;
        }
    }
    LeaveCriticalSection(&app->mDLCDownloadQueueMutex);
    return 0;
}
uint32_t CMinecraftApp::RetrieveNextDLCContent()
{
    const int primaryPad = gProfile->GetPrimaryPad();
    if (primaryPad == -1 || !gProfile->IsSignedInLive(primaryPad))
        return 1;

    EnterCriticalSection(&mDLCRetrieveMutex);

    for (std::vector<_DLCRequest*>::iterator it = mDLCDownloadQueue.begin(); it != mDLCDownloadQueue.end(); ++it)
    {
        if ((*it)->mStatus == 1)
        {
            LeaveCriticalSection(&mDLCRetrieveMutex);
            return 1;
        }
    }

    std::vector<_DLCRequest*>::iterator it = mDLCDownloadQueue.begin();
    while (it != mDLCDownloadQueue.end() && (*it)->mStatus != 0)
        ++it;

    if (it == mDLCDownloadQueue.end())
    {
        LeaveCriticalSection(&mDLCRetrieveMutex);
        return 0;
    }

    _DLCRequest* request = *it;
    const int result =
        gStorage->GetDLCOffers(gProfile->GetPrimaryPad(), DLCOffersReturned, this, request->mContentId);

    request->mStatus = (result == 5) ? 1U : 2U;
    LeaveCriticalSection(&mDLCRetrieveMutex);
    return 1;
}
void CMinecraftApp::SetCorruptSaveDeleted(bool deleted)
{
    mCorruptSaveDeleted = deleted;
}
uint8_t CMinecraftApp::GetCorruptSaveDeleted()
{
    return mCorruptSaveDeleted;
}
void CMinecraftApp::SetAdditionalSkinBoxes(unsigned int skinId, SKIN_BOX* boxes, unsigned int count,
                                           _SkinAdjustments adjustments)
{
    const EPlayerModelType modelType = static_cast<EPlayerModelType>(
        Player::GetModelTypeFromAnimBitmask(adjustments.mAnimFlags));
    PlayerModel* model = static_cast<PlayerModel*>(
        Minecraft::GetInstance()->getEntityRenderDispatcher()->getPlayerRenderer(modelType)->getModel());

    std::vector<ModelPart*>* modelParts = new std::vector<ModelPart*>();
    std::vector<SKIN_BOX*>* skinBoxes = new std::vector<SKIN_BOX*>();

    EnterCriticalSection(&mAdditionalModelPartsMutex);
    EnterCriticalSection(&mAdditionalSkinBoxesMutex);
    for (unsigned int index = 0; index < count; ++index)
    {
        if (model != NULL)
        {
            model->SetSkinAdjustments(adjustments);
            ModelPart* part = model->AddOrRetrievePart(&boxes[index], skinId);
            modelParts->push_back(part);
            skinBoxes->push_back(&boxes[index]);
        }
    }
    mAdditionalModelParts.insert(std::make_pair(skinId, modelParts));
    mAdditionalSkinBoxes.insert(std::make_pair(skinId, skinBoxes));
    LeaveCriticalSection(&mAdditionalSkinBoxesMutex);
    LeaveCriticalSection(&mAdditionalModelPartsMutex);
}
std::vector<ModelPart*>* CMinecraftApp::SetAdditionalSkinBoxes(unsigned int skinId, std::vector<SKIN_BOX*>* boxes,
                                                               _SkinAdjustments adjustments)
{
    const EPlayerModelType modelType = static_cast<EPlayerModelType>(
        Player::GetModelTypeFromAnimBitmask(adjustments.mAnimFlags));
    PlayerRenderer* renderer = Minecraft::GetInstance()->getEntityRenderDispatcher()->getPlayerRenderer(modelType);
    PlayerModel* model = static_cast<PlayerModel*>(renderer->getModel());
    std::vector<ModelPart*>* modelParts = new std::vector<ModelPart*>();

    EnterCriticalSection(&mAdditionalModelPartsMutex);
    EnterCriticalSection(&mAdditionalSkinBoxesMutex);
    for (std::vector<SKIN_BOX*>::iterator it = boxes->begin(); it != boxes->end(); ++it)
    {
        if (model != NULL)
        {
            model->SetSkinAdjustments(adjustments);
            modelParts->push_back(model->AddOrRetrievePart(*it, skinId));
        }
    }
    mAdditionalModelParts.insert(std::make_pair(skinId, modelParts));
    mAdditionalSkinBoxes.insert(std::make_pair(skinId, boxes));
    LeaveCriticalSection(&mAdditionalSkinBoxesMutex);
    LeaveCriticalSection(&mAdditionalModelPartsMutex);
    return modelParts;
}
std::vector<ModelPart*>* CMinecraftApp::GetAdditionalModelParts(unsigned int value)
{
    EnterCriticalSection(&mAdditionalModelPartsMutex);
    std::vector<ModelPart*>* result = NULL;
    if (!mAdditionalModelParts.empty())
    {
        boost::unordered_map<unsigned int, std::vector<ModelPart*>*>::iterator it = mAdditionalModelParts.find(value);
        if (it != mAdditionalModelParts.end())
            result = it->second;
    }
    LeaveCriticalSection(&mAdditionalModelPartsMutex);
    return result;
}
std::vector<SKIN_BOX*>* CMinecraftApp::GetAdditionalSkinBoxes(unsigned int skinId)
{
    EnterCriticalSection(&mAdditionalSkinBoxesMutex);
    std::vector<SKIN_BOX*>* result = NULL;
    if (!mAdditionalSkinBoxes.empty())
    {
        boost::unordered_map<unsigned int, std::vector<SKIN_BOX*>*>::iterator it = mAdditionalSkinBoxes.find(skinId);
        if (it != mAdditionalSkinBoxes.end())
            result = it->second;
    }
    LeaveCriticalSection(&mAdditionalSkinBoxesMutex);
    return result;
}
unsigned int CMinecraftApp::getSkinIdFromPath(const std::wstring& path)
{
    unsigned int id = 0;
    if (path.length() < 14)
        return id;

    const bool dlcSkin = path.substr(0, 3).compare(L"dlc") == 0;
    std::wstring idText = path.substr(7);
    const std::wstring::size_type extension = idText.find(L'.');
    idText = idText.substr(0, extension);

    std::wstringstream stream;
    if (dlcSkin)
        stream << std::dec << idText.c_str();
    else
        stream << std::hex << idText.c_str();
    stream >> id;
    id = (dlcSkin ? (1U << SKIN_ID_DLC_FLAG_SHIFT) : 0U) | (id & SKIN_ID_DLC_MASK);
    return id;
}
std::wstring CMinecraftApp::getSkinPathFromId(unsigned int id)
{
    wchar_t path[256];
    if ((id >> SKIN_ID_DLC_FLAG_SHIFT) != 0)
    {
        std::swprintf(path, 256, L"dlcskin%08d.png", id & SKIN_ID_DLC_MASK);
    }
    else
    {
        const unsigned int ugcIndex = id & SKIN_ID_UGC_MASK;
        const unsigned int defaultIndex = id & SKIN_ID_DEFAULT_MASK;
        if (ugcIndex == 0)
            std::swprintf(path, 256, L"defskin%08X.png", defaultIndex);
        else
            std::swprintf(path, 256, L"ugcskin%08X.png", ugcIndex);
    }
    return std::wstring(path);
}
uint32_t CMinecraftApp::GetTMSGlobalFileListRead()
{
    return 1;
}
uint32_t CMinecraftApp::GetTMSDLCInfoRead()
{
    return 1;
}
uint32_t CMinecraftApp::GetTMSXUIDsFileRead()
{
    return 1;
}
void CMinecraftApp::SetRequiredTexturePackID(uint32_t texturePackId)
{
    mRequiredTexturePackID = texturePackId;
}
void CMinecraftApp::GetFileFromTPD(eTPDFileType type, unsigned char* source, unsigned int sourceSize,
                                   unsigned char** data, unsigned int* size)
{
    *data = 0;
    *size = 0;
}
void CMinecraftApp::getLocale(std::vector<std::wstring>& localeNames)
{
    std::vector<eMCLang> languages;
    switch (XGetLanguage())
    {
    case XC_LANGUAGE_ENGLISH:
        switch (XGetLocale())
        {
        case XC_LOCALE_AUSTRALIA:
        case XC_LOCALE_CANADA:
        case XC_LOCALE_CZECH_REPUBLIC:
        case XC_LOCALE_GREECE:
        case XC_LOCALE_HONG_KONG:
        case XC_LOCALE_HUNGARY:
        case XC_LOCALE_INDIA:
        case XC_LOCALE_IRELAND:
        case XC_LOCALE_NEW_ZEALAND:
        case XC_LOCALE_SINGAPORE:
        case XC_LOCALE_SLOVAK_REPUBLIC:
        case XC_LOCALE_SOUTH_AFRICA:
        case XC_LOCALE_GREAT_BRITAIN:
        case XC_LOCALE_SAUDI_ARABIA:
        case XC_LOCALE_ISRAEL:
        case XC_LOCALE_UNITED_ARAB_EMIRATES:
            languages.push_back(eMCLang_enGB);
            break;
        default:
            break;
        }
        break;
    case XC_LANGUAGE_JAPANESE:
        languages.push_back(eMCLang_jaJP);
        break;
    case XC_LANGUAGE_GERMAN:
        switch (XGetLocale())
        {
        case XC_LOCALE_AUSTRIA:
            languages.push_back(eMCLang_deAT);
            break;
        case XC_LOCALE_SWITZERLAND:
            languages.push_back(eMCLang_deCH);
            break;
        default:
            break;
        }
        languages.push_back(eMCLang_deDE);
        break;
    case XC_LANGUAGE_FRENCH:
        switch (XGetLocale())
        {
        case XC_LOCALE_BELGIUM:
            languages.push_back(eMCLang_frBE);
            break;
        case XC_LOCALE_CANADA:
            languages.push_back(eMCLang_frCA);
            break;
        case XC_LOCALE_SWITZERLAND:
            languages.push_back(eMCLang_frCH);
            break;
        default:
            break;
        }
        languages.push_back(eMCLang_frFR);
        break;
    case XC_LANGUAGE_SPANISH:
        switch (XGetLocale())
        {
        case XC_LOCALE_CHILE:
        case XC_LOCALE_COLOMBIA:
        case XC_LOCALE_MEXICO:
        case XC_LOCALE_UNITED_STATES:
        case XC_LOCALE_ARGENTINA:
        case XC_LOCALE_LATIN_AMERICA:
            languages.push_back(eMCLang_laLAS);
            languages.push_back(eMCLang_esMX);
            break;
        default:
            break;
        }
        languages.push_back(eMCLang_esES);
        break;
    case XC_LANGUAGE_ITALIAN:
        languages.push_back(eMCLang_itIT);
        break;
    case XC_LANGUAGE_KOREAN:
        languages.push_back(eMCLang_koKR);
        break;
    case XC_LANGUAGE_TCHINESE:
        switch (XGetLocale())
        {
        case XC_LOCALE_HONG_KONG:
            languages.push_back(eMCLang_zhHK);
            languages.push_back(eMCLang_zhTW);
            break;
        case XC_LOCALE_TAIWAN:
            languages.push_back(eMCLang_zhTW);
            languages.push_back(eMCLang_zhHK);
            break;
        default:
            break;
        }
        languages.push_back(eMCLang_zhHANT);
        languages.push_back(eMCLang_zhCHT);
        break;
    case XC_LANGUAGE_PORTUGUESE:
        if (XGetLocale() == XC_LOCALE_BRAZIL)
            languages.push_back(eMCLang_ptBR);
        languages.push_back(eMCLang_ptPT);
        break;
    case XC_LANGUAGE_SCHINESE:
        break;
    case XC_LANGUAGE_POLISH:
        languages.push_back(eMCLang_plPL);
        break;
    case XC_LANGUAGE_RUSSIAN:
        languages.push_back(eMCLang_ruRU);
        break;
    case XC_LANGUAGE_SWEDISH:
        languages.push_back(eMCLang_svSV);
        languages.push_back(eMCLang_svSE);
        break;
    case XC_LANGUAGE_TURKISH:
        languages.push_back(eMCLang_trTR);
        break;
    case XC_LANGUAGE_NORWEGIAN:
        languages.push_back(eMCLang_nbNO);
        languages.push_back(eMCLang_noNO);
        languages.push_back(eMCLang_nnNO);
        break;
    case XC_LANGUAGE_DUTCH:
        if (XGetLocale() == XC_LOCALE_BELGIUM)
            languages.push_back(eMCLang_nlBE);
        languages.push_back(eMCLang_nlNL);
        break;
    case XC_LANGUAGE_ZH_HANS:
        languages.push_back(eMCLang_zhHANS);
        break;
    case XC_LANGUAGE_FINNISH:
        languages.push_back(eMCLang_fiFI);
        break;
    case XC_LANGUAGE_GREEK:
        languages.push_back(eMCLang_elEL);
        languages.push_back(eMCLang_elGR);
        languages.push_back(eMCLang_enGR);
        languages.push_back(eMCLang_enGB);
        break;
    case XC_LANGUAGE_DANISH:
        languages.push_back(eMCLang_daDA);
        languages.push_back(eMCLang_daDK);
        break;
    case XC_LANGUAGE_CZECH:
        languages.push_back(eMCLang_csCZ);
        languages.push_back(eMCLang_enCZ);
        break;
    default:
        break;
    }
    languages.push_back(eMCLang_enUS);
    languages.push_back(eMCLang_enEN);

    for (unsigned int i = 0; i < languages.size(); ++i)
        localeNames.push_back(mLocaleToString[languages.at(i)]);
}
void CMinecraftApp::SetFirstMiniGameType(EMiniGameId type)
{
    mFirstMiniGameType = type;
}
EMiniGameId CMinecraftApp::GetFirstMiniGameType()
{
    return mFirstMiniGameType;
}
void CMinecraftApp::ProcessFakePadValueA()
{
    const unsigned int fakePadActionCount = 61;
    for (int pad = 0; pad < 4; ++pad)
    {
        for (unsigned int action = 0; action < fakePadActionCount; ++action)
        {
            if (((mFakePadValues[pad] >> action) & 1) != 0)
            {
                g_pInput->SetFakePadValue(pad, action);
                g_pInput->ButtonPressed(pad, EControllerActions_Jump, 0);
            }
        }
        mFakePadValues[pad] = 0;
    }
}
bool CMinecraftApp::IsVoiceChatEnabled()
{
    return gApp.GetGameStarted() != 0
           && GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_GameChat) != 0;
}
void CMinecraftApp::CaptureScreenshot(int32_t playerNumber)
{
    (void)playerNumber;
}
void CMinecraftApp::RunFrame()
{
}
void CMinecraftApp::HandleXuiActions()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer;

    if (gApp.GetGlobalXuiAction() == eXuiAction_LavaNearSpawn)
    {
        int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
        if (gConsoleUIController.RequestErrorMessage(IDS_CANT_PLACE_NEAR_SPAWN_TITLE, static_cast<int>(0xbf12dc51U),
                                                     parameters, 1, 0xff, NULL, NULL, NULL, true)
            != 1)
            SetGlobalXuiAction(eXuiAction_Idle);
    }

    EnterCriticalSection(&mActionMutex);
    for (int pad = 0; pad < 4; ++pad)
    {
        eXuiAction action = mXuiActions[pad].action;
        void* param = mXuiActions[pad].param;
        if (action == eXuiAction_Idle)
        {
            ClientMasterGameMode* masterGameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
            if (pad == gProfile->GetPrimaryPad() && masterGameMode != NULL && masterGameMode->isWaitingToTransition())
            {
                action = eXuiAction_GameModeTransition;
                param = masterGameMode->popTransition();
            }
        }
        else if (action == eXuiAction_GameModeTransitionInProgress || action == eXuiAction_ConnectionLostWait
                 || action > eXuiAction_Reset)
        {
            action = eXuiAction_Idle;
        }

        switch (action)
        {
        case eXuiAction_SaveGame:
            if (GetChangingSessionType())
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            }
            else if (!gProfile->IsFullVersion())
            {
                int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), IDS_CONFIRM_CANCEL};
                gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_UNLOCK_TITLE),
                                                         static_cast<int>(0xc75616c3U), parameters, 2, pad,
                                                         UnlockFullSaveReturned, this, NULL, true);
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            }
            else
            {
                SetActionInternal(pad, eXuiAction_SaveGameNow, NULL);
            }
            break;
        case eXuiAction_SaveGameNow:
        {
            gApp.SetAutosaveTimerTime();
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            gConsoleUIController.HideAllGameUIElements();
            gConsoleUIController.ShowOtherPlayersBaseScene(gProfile->GetPrimaryPad(), false);
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadParam = NULL;
            params->m_threadProc = &UIScene_PauseMenu::SaveWorldThreadProc;
            params->m_waitForThreadToDelete = true;
            FullscreenProgressState* state = new FullscreenProgressState;
            state->closeMode = 6;
            state->playerId = gProfile->GetPrimaryPad();
            state->backScene = gConsoleUIController.IsSceneInStack(gProfile->GetPrimaryPad(), EUIScene_EndPoem)
                                   ? EUIScene_EndPoem
                                   : EUIScene_PauseMenu;
            params->m_completionData = state;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Fullscreen, EUIGroup_Fullscreen);
            break;
        }
        case eXuiAction_ExitWorld:
        {
            if (CGameNetworkManager::sInstance.IsLeavingGame())
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
                break;
            }
            minecraft->mGui->clearMessages(-1);
            minecraft->mGui->clearSingleMessages();
            gConsoleUIController.HideAllGameUIElements();
            gConsoleUIController.ShowAutosaveCountdownTimer(false);
            gConsoleUIController.HideAllGameUIElements();
            for (int player = 0; player < 4; ++player)
                gConsoleUIController.SetTutorialVisible(player, false);
            gStorage->ForceQueuedProfileWrites(-1);
            if (g_pLeaderboardManager->OpenSession())
            {
                for (int player = 0; player < 4; ++player)
                {
                    if (gProfile->IsSignedIn(player))
                        Minecraft::GetInstance()->forceStatsSave(player);
                }
                g_pLeaderboardManager->CloseSession();
            }

            ClientMasterGameMode* masterGameMode;
            if (!gProfile->IsFullVersion())
            {
                int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), IDS_CONFIRM_CANCEL};
                gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_UNLOCK_TITLE),
                                                         static_cast<int>(0xc75616c3U), parameters, 2, pad,
                                                         UnlockFullExitReturned, this, NULL, true);
                SetActionInternal(pad, eXuiAction_Idle, NULL);
                masterGameMode = minecraft->GetClientMasterGameMode();
            }
            else
            {
                if (pad != gProfile->GetPrimaryPad())
                {
                    minecraft->setLocalPlayerIdx(gProfile->GetPrimaryPad());
                    minecraft->removeLocalPlayerIdx(pad);
                    gProfile->SignOutUser(pad);
                    SetActionInternal(pad, eXuiAction_Idle, NULL);
                    LeaveCriticalSection(&mActionMutex);
                    return;
                }
                SetActionInternal(pad, eXuiAction_ExitWorldNow, param);
                masterGameMode = minecraft->GetClientMasterGameMode();
            }
            if (masterGameMode != NULL)
                masterGameMode->OnExitedGame();
            if (gMinecraftServer != NULL && gMinecraftServer->mMasterGameMode != NULL)
                gMinecraftServer->mMasterGameMode->KillTimers();
            Minecraft::GetInstance()->mSoundEngine->RemoveMusicVolumeLimit();

            if (CGameNetworkManager::sInstance.GetPlayerCount() < 2)
            {
                gApp.SetRichPresenceContext(pad, 0);
                gProfile->SetCurrentGameActivity(pad, CGameNetworkManager::sInstance.IsLocalGame() ? 5 : 4, false);
                telemetryManagerInstance->RecordLevelExit(pad, eSen_LevelExitStatus_Completed);
            }
            else
            {
                for (int player = 0; player < 4; ++player)
                {
                    if (minecraft->mLocalPlayers[player] == NULL)
                        continue;
                    const bool localGame = CGameNetworkManager::sInstance.IsLocalGame();
                    gApp.SetRichPresenceContext(player, 0);
                    gProfile->SetCurrentGameActivity(player, localGame ? 3 : 2, false);
                    telemetryManagerInstance->RecordLevelExit(player, eSen_LevelExitStatus_Completed);
                }
            }
            break;
        }
        case eXuiAction_ExitWorldNow:
        {
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            SetGameStarted(false);
            SetChangingSessionType(true);
            gConsoleUIController.CloseAllPlayersScenes(false);
            gConsoleUIController.HideAllGameUIElements();
            for (int player = 0; player < 4; ++player)
                minecraft->playerLeftTutorial(player);
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadParam = param;
            params->m_threadProc = &UIScene_PauseMenu::ExitWorldThreadProc;
            FullscreenProgressState* completionData = new FullscreenProgressState;
            completionData->showTip = param == NULL;
            completionData->closeMode = 4;
            completionData->playerId = 0;
            params->m_completionData = completionData;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
        case eXuiAction_ExitWorldAfterUpsell:
        {
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            minecraft->mGui->clearMessages(-1);
            minecraft->mGui->clearSingleMessages();
            gConsoleUIController.HideAllGameUIElements();
            for (int player = 0; player < 4; ++player)
                gConsoleUIController.SetTutorialVisible(player, false);
            SetGameStarted(false);
            gConsoleUIController.CloseAllPlayersScenes(false);
            for (int player = 0; player < 4; ++player)
                minecraft->playerLeftTutorial(player);
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadParam = param;
            params->m_threadProc = &UIScene_PauseMenu::ExitWorldThreadProc;
            FullscreenProgressState* completionData = new FullscreenProgressState;
            completionData->closeMode = 4;
            completionData->playerId = 0;
            params->m_completionData = completionData;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
        case eXuiAction_Respawn:
        {
            _ConnectionProgressParams* params = new _ConnectionProgressParams();
            params->m_playerId = pad;
            params->m_showTooltips = false;
            params->m_setFailTimer = false;
            params->m_stringId = IDS_PROGRESS_RESPAWNING;
            gConsoleUIController.DisplayGamertag(pad, false);
            gConsoleUIController.NavigateToScene(pad, EUIScene_ConnectingProgress, params, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            minecraft->mLocalPlayers[pad]->mHasOpenedInventory = false;
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            minecraft->mLocalPlayers[pad]->respawn();
            break;
        }
        case eXuiAction_RespawnProgress:
        {
            localPlayer = minecraft->mLocalPlayers[pad];
            if (localPlayer == NULL || localPlayer->mHasOpenedInventory)
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
                break;
            }
            _ConnectionProgressParams* params = new _ConnectionProgressParams();
            params->m_playerId = pad;
            params->m_showTooltips = false;
            params->m_setFailTimer = false;
            params->m_stringId = IDS_PROGRESS_RESPAWNING;
            gConsoleUIController.DisplayGamertag(pad, false);
            gConsoleUIController.NavigateToScene(pad, EUIScene_ConnectingProgress, params, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        }
        case eXuiAction_PrimarySignedOut:
        case eXuiAction_ProfileReadError:
        {
            if (action == eXuiAction_PrimarySignedOut)
                gConsoleUIController.ShowAutosaveCountdownTimer(false);
            if (!gApp.GetGameStarted())
                MinecraftServer::HaltServer(true);
            if (action == eXuiAction_PrimarySignedOut)
            {
                gStorage->SetSaveDeviceSelected(pad, false);
                Minecraft::GetInstance()->mStats[pad]->clear();
            }
            if (!CGameNetworkManager::sInstance.IsInSession())
            {
                gApp.SetActionInternal(pad, eXuiAction_PrimarySignedOutReturnToMenu, NULL);
                MinecraftServer::s_haltRequested = false;
                MinecraftServer::s_saveOnExitAnswered = false;
            }
            else
            {
                gApp.SetActionInternal(pad, eXuiAction_PrimarySignedOutInGame, NULL);
            }
            break;
        }
        case eXuiAction_PrimarySignedOutReturnToMenu:
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            gProfile->ResetProfileProcessState();
            gStorage->SetSaveDeviceSelected(pad, false);
            gConsoleUIController.UpdatePlayerBasePositions();
            gConsoleUIController.NavigateToHomeMenu(false);
            break;
        case eXuiAction_RemoveLocalPlayer:
        {
            const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
            gStorage->ForceQueuedProfileWrites(pad);
            if (g_pLeaderboardManager->OpenSession())
            {
                for (int player = 0; player < 4; ++player)
                {
                    if (gProfile->IsSignedIn(player))
                        Minecraft::GetInstance()->forceStatsSave(player);
                }
                g_pLeaderboardManager->CloseSession();
            }
            minecraft->mGui->clearMessages(pad);
            minecraft->setLocalPlayerIdx(gProfile->GetPrimaryPad());
            minecraft->removeLocalPlayerIdx(pad);
            gConsoleUIController.SetTooltips(pad, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                             false);
            if (playerCount > 2)
            {
                for (int player = 0; player < 4; ++player)
                {
                    if (player != pad && minecraft->mLocalPlayers[player] != NULL)
                        gProfile->SetCurrentGameActivity(player, CGameNetworkManager::sInstance.IsLocalGame() ? 3 : 2,
                                                         false);
                }
            }
            else
            {
                for (int player = 0; player < 4; ++player)
                {
                    if (player != pad && minecraft->mLocalPlayers[player] != NULL)
                        gProfile->SetCurrentGameActivity(player, CGameNetworkManager::sInstance.IsLocalGame() ? 5 : 4,
                                                         false);
                }
            }
            gProfile->SignOutUser(pad);
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        }
        case eXuiAction_RemoveLocalPlayerNoStats:
        {
            const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
            gStorage->ForceQueuedProfileWrites(pad);
            minecraft->mGui->clearMessages(pad);
            minecraft->setLocalPlayerIdx(gProfile->GetPrimaryPad());
            minecraft->removeLocalPlayerIdx(pad);
            gConsoleUIController.SetTooltips(pad, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                             false);
            const int activity = playerCount < 3 ? (CGameNetworkManager::sInstance.IsLocalGame() ? 5 : 4)
                                                 : (CGameNetworkManager::sInstance.IsLocalGame() ? 3 : 2);
            for (int player = 0; player < 4; ++player)
            {
                if (player != pad && minecraft->mLocalPlayers[player] != NULL)
                    gProfile->SetCurrentGameActivity(player, activity, false);
            }
            gProfile->SignOutUser(pad);
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        }
        case eXuiAction_TrialOver:
        {
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            int parameters[2] = {static_cast<int>(IDS_UNLOCK_TITLE), static_cast<int>(IDS_EXIT_GAME)};
            gConsoleUIController.RequestErrorMessage(IDS_TRIALOVER_TITLE, static_cast<int>(0x91a0515aU), parameters, 2,
                                                     pad, TrialOverReturned, this, NULL, true);
            break;
        }
        case eXuiAction_ExitGame:
            ExitGame();
            break;
        case eXuiAction_CloseScenesOnConnect:
        {
            localPlayer = minecraft->mLocalPlayers[pad];
            if (localPlayer != NULL && localPlayer->mPacketListener != NULL
                && localPlayer->mPacketListener->isStarted())
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
                gConsoleUIController.CloseUIScenes(pad, false);
            }
            else if (!CGameNetworkManager::sInstance.IsInGameplay())
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            }
            break;
        }
        case eXuiAction_ConnectionLost:
        {
            CGameNetworkManager::CancelJoinGame(&CGameNetworkManager::sInstance);
            if (CGameNetworkManager::sInstance.IsLeavingGame())
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            }
            else if (!CGameNetworkManager::sInstance.IsHost())
            {
                if (sRetryDisconnectAction)
                {
                    SetActionInternal(pad, eXuiAction_ConnectionLost, NULL);
                    break;
                }
                SetActionInternal(
                    pad, CGameNetworkManager::sInstance.IsInSession() ? eXuiAction_ConnectionLostWait : eXuiAction_Idle,
                    NULL);
                int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
                if (gConsoleUIController.RequestErrorMessage(
                        CGameNetworkManager::sInstance.CorrectErrorIDS(static_cast<int>(IDS_CONNECTION_LOST)),
                        CGameNetworkManager::sInstance.CorrectErrorIDS(static_cast<int>(IDS_CONNECTION_LOST_LIVE)),
                        parameters, 1, 0xff, EthernetDisconnectReturned, reinterpret_cast<void*>(pad), NULL, true)
                    == 1)
                    SetActionInternal(pad, eXuiAction_ConnectionLost, NULL);
                Minecraft::GetInstance()->mStats[pad]->clear();
            }
            else if (!CGameNetworkManager::sInstance.IsLocalGame() && CGameNetworkManager::sInstance.IsInGameplay())
            {
                SetActionInternal(pad, eXuiAction_ChangeSessionType, NULL);
            }
            else if (CGameNetworkManager::sInstance.IsLocalGame() || CGameNetworkManager::sInstance.IsInGameplay())
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            }
            else
            {
                bool levelLoaded = false;
                for (int level = 0; level < minecraft->mLevels.length; ++level)
                {
                    if (minecraft->mLevels[level] != NULL)
                    {
                        levelLoaded = true;
                        break;
                    }
                }
                if (levelLoaded)
                {
                    SetActionInternal(pad, eXuiAction_ChangeSessionType, NULL);
                    break;
                }
                SetActionInternal(pad, eXuiAction_ConnectionLostWait, NULL);
                int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
                gConsoleUIController.RequestErrorMessage(
                    CGameNetworkManager::sInstance.CorrectErrorIDS(static_cast<int>(IDS_CONNECTION_LOST)),
                    CGameNetworkManager::sInstance.CorrectErrorIDS(static_cast<int>(IDS_CONNECTION_LOST_LIVE)),
                    parameters, 1, 0xff, EthernetDisconnectReturned, reinterpret_cast<void*>(pad), NULL, true);
                MinecraftServer::HaltServer(false);
                CGameNetworkManager::sInstance.LeaveGame(false);
                Minecraft::GetInstance()->mStats[pad]->clear();
            }
            break;
        }
        case eXuiAction_PrimarySignedOutInGame:
        case eXuiAction_ExitWorldAfterDisconnect:
        {
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            ClientMasterGameMode* masterGameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
            if (masterGameMode != NULL && pad == gProfile->GetPrimaryPad())
                masterGameMode->dropAllTransitions();
            minecraft->mGui->clearMessages(-1);
            minecraft->mGui->clearSingleMessages();
            gConsoleUIController.HideAllGameUIElements();
            gProfile->ResetProfileProcessState();
            if (!CGameNetworkManager::sInstance.IsLeavingGame())
            {
                for (int player = 0; player < 4; ++player)
                    gConsoleUIController.SetTutorialVisible(player, false);
                SetGameStarted(false);
                gConsoleUIController.HideAllGameUIElements();
                gConsoleUIController.CloseAllPlayersScenes(false);
                for (int player = 0; player < 4; ++player)
                    minecraft->playerLeftTutorial(player);
                _LoadingInputParams* params = new _LoadingInputParams();
                params->m_threadProc = &SignoutExitWorldThreadProc;
                FullscreenProgressState* completionData = new FullscreenProgressState;
                completionData->closeMode = 4;
                completionData->playerId = 0;
                params->m_completionData = completionData;
                gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                     EUILayer_Scene, EUIGroup_PlayerDefault);
            }
            else
            {
                minecraft->mProgressRenderer->progressStartNoAbort(IDS_EXITING_GAME);
                minecraft->mProgressRenderer->progressStage(-1);
                MinecraftServer::HaltServer(true);
            }
            break;
        }
        case eXuiAction_ReturnToMenuAfterDisconnect:
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            gProfile->ResetProfileProcessState();
            gConsoleUIController.UpdatePlayerBasePositions();
            gConsoleUIController.NavigateToHomeMenu(false);
            break;
        case eXuiAction_JoinFromInvite:
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            if (!gProfile->IsFullVersion())
            {
                int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), IDS_CONFIRM_CANCEL};
                gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_UNLOCK_TITLE),
                                                         static_cast<int>(0xadece1fdU), parameters, 2, pad,
                                                         UnlockFullInviteReturned, this, NULL, true);
                telemetryManagerInstance->RecordUpsellPresented(pad, eSen_UpsellID_FullGame, 1);
            }
            else
            {
                UIScene_PauseMenu::OpenExitGameDialog(gProfile->GetPrimaryPad(), this, ExitAndJoinFromInvite,
                                                      ExitAndJoinFromInviteSaveDialogReturned,
                                                      UIScene_PauseMenu::EOpenExitGameDialog_JoinInvite);
            }
            break;
        case eXuiAction_UpsellForInvite:
        {
            telemetryManagerInstance->RecordUpsellPresented(pad, eSen_UpsellID_FullGame, 1);
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), IDS_CONFIRM_CANCEL};
            gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_UNLOCK_TITLE), static_cast<int>(0xadece1fdU),
                                                     parameters, 2, pad, UnlockFullInviteReturned, this, NULL, true);
            break;
        }
        case eXuiAction_ExitAndJoinFromInvite:
        {
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
            if (masterGameMode != NULL)
                masterGameMode->OnExitedGame();
            minecraft->mGui->clearMessages(-1);
            minecraft->mGui->clearSingleMessages();
            gConsoleUIController.HideAllGameUIElements();
            for (int player = 0; player < 4; ++player)
                gConsoleUIController.SetTutorialVisible(player, false);
            SetGameStarted(false);
            gConsoleUIController.CloseAllPlayersScenes(false);
            for (int player = 0; player < 4; ++player)
                minecraft->playerLeftTutorial(player);
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadParam = &CGameNetworkManager::sInstance.m_pendingInviteUserIndex;
            params->m_threadProc = &CGameNetworkManager::ExitAndJoinFromInviteThreadProc;
            FullscreenProgressState* completionData = new FullscreenProgressState;
            completionData->closeMode = 0;
            completionData->playerId = 0;
            params->m_completionData = completionData;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
        case eXuiAction_JoinInvitedGame:
        {
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            SetTutorialMode(false, boost::shared_ptr<Player>());
            CGameNetworkManager::sInstance.SetLocalGame(false);
            Minecraft::GetInstance()->clearConnectionFailed();
            const InviteJoinData* invite = static_cast<const InviteJoinData*>(param);
            gProfile->SetLockedProfile(invite->userIndex);
            gProfile->SetPrimaryPad(invite->userIndex);
            for (int player = 0; player < 4; ++player)
            {
                if (player == invite->userIndex)
                    continue;
                if (gProfile->IsSignedIn(player)
                    && (CGameNetworkManager::sInstance.m_pendingInviteUserMask
                        & CGameNetworkManager::GetLocalPlayerMask(player))
                           == 0)
                    gProfile->SignOutUser(player);
            }
            Minecraft::GetInstance()->mUser->name = convStringToWstring(
                gProfile->GetGamertag(gProfile->GetPrimaryPad()));
            if (!CGameNetworkManager::sInstance.JoinGameFromInviteInfo(invite->userIndex, invite->userIndexMask,
                                                                       invite->presence))
            {
                int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
                gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, IDS_CONNECTION_LOST_SERVER, parameters,
                                                         1, gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
                gConsoleUIController.NavigateToHomeMenu(false);
                gConsoleUIController.UpdatePlayerBasePositions();
            }
            break;
        }
        case eXuiAction_ChangeSessionType:
        {
            if (!CGameNetworkManager::sInstance.IsInGameplay() || gConsoleUIController.IsExpectingOrReloadingSkin())
            {
                if (CGameNetworkManager::sInstance.IsLeavingGame())
                    SetActionInternal(pad, eXuiAction_Idle, NULL);
                break;
            }
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            if (GetChangingSessionType() || CGameNetworkManager::sInstance.IsLocalGame())
                break;
            SetGameStarted(false);
            SetChangingSessionType(true);
            SetReallyChangingSessionType(true);
            gConsoleUIController.HideAllGameUIElements();
            if (!gConsoleUIController.IsSceneInStack(gProfile->GetPrimaryPad(), EUIScene_EndPoem))
                gConsoleUIController.CloseAllPlayersScenes(false);
            gConsoleUIController.ShowOtherPlayersBaseScene(gProfile->GetPrimaryPad(), true);
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadProc = &CGameNetworkManager::ChangeSessionTypeThreadProc;
            params->m_threadParam = NULL;
            FullscreenProgressState* state = new FullscreenProgressState;
            state->playerId = 0;
            state->showConfirmation = true;
            if (gConsoleUIController.IsSceneInStack(gProfile->GetPrimaryPad(), EUIScene_EndPoem))
            {
                state->closeMode = 6;
                state->backScene = EUIScene_EndPoem;
            }
            else
            {
                state->closeMode = 3;
            }
            params->m_completionData = state;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
        case eXuiAction_SetDefaultGameOptions:
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            GameSettings::SetDefaultGameOptions(static_cast<C4JStorage::PROFILESETTINGS*>(param), pad, true);
            GameSettings::CheckGameSettingsChanged(true, pad);
            gConsoleUIController.SetupFont();
            break;
        case eXuiAction_RemoteSave:
        {
            if (GetGameStarted())
            {
                SetActionInternal(gProfile->GetPrimaryPad(), eXuiAction_Idle, NULL);
                break;
            }
            SetActionInternal(pad, eXuiAction_RemoteSaveInProgress, NULL);
            for (int player = 0; player < 4; ++player)
                gConsoleUIController.CloseUIScenes(player, true);
            gConsoleUIController.HideAllGameUIElements();
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadProc = &RemoteSaveThreadProc;
            params->m_threadParam = NULL;
            FullscreenProgressState* state = new FullscreenProgressState;
            state->playerId = 0;
            if (gConsoleUIController.IsSceneInStack(gProfile->GetPrimaryPad(), EUIScene_ConnectingProgress))
            {
                state->closeMode = 1;
            }
            else if (gConsoleUIController.IsSceneInStack(gProfile->GetPrimaryPad(), EUIScene_EndPoem))
            {
                state->closeMode = 6;
                state->backScene = EUIScene_EndPoem;
            }
            else
            {
                state->closeMode = 3;
            }
            params->m_cancelFunc = &ExitGameFromRemoteSave;
            params->m_cancelText = IDS_TOOLTIPS_EXIT;
            params->m_completionData = state;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
        case eXuiAction_MultiplayerNotAllowed:
        {
            int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
            if (gConsoleUIController.RequestErrorMessage(
                    IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, static_cast<int>(IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT),
                    parameters, 1, gProfile->GetPrimaryPad(), NULL, NULL, NULL, true)
                != 1)
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        }
        case eXuiAction_CheckSaveExists:
        {
            bool saveExists;
            gStorage->DoesSaveExist(&saveExists);
            SetActionInternal(pad, GetChangingSessionType() ? eXuiAction_Idle : eXuiAction_AutoSave, NULL);
            break;
        }
        case eXuiAction_AutoSave:
        {
            gApp.SetAutosaveTimerTime();
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            gConsoleUIController.HideAllGameUIElements();
            gConsoleUIController.ShowOtherPlayersBaseScene(gProfile->GetPrimaryPad(), false);
            if (minecraft->mGameModes[gProfile->GetPrimaryPad()] != NULL)
            {
                Tutorial* tutorial = minecraft->mGameModes[gProfile->GetPrimaryPad()]->getTutorial();
                if (tutorial != NULL)
                    tutorial->showTutorialPopup(false);
            }
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadProc = &UIScene_PauseMenu::SaveWorldThreadProc;
            params->m_threadParam = reinterpret_cast<void*>(1);
            FullscreenProgressState* completionData = new FullscreenProgressState;
            completionData->closeMode = 5;
            completionData->playerId = gProfile->GetPrimaryPad();
            params->m_completionData = completionData;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Fullscreen, EUIGroup_Fullscreen);
            break;
        }
        case eXuiAction_BannedLevel:
            if (!CGameNetworkManager::sInstance.IsInGameplay() || CGameNetworkManager::sInstance.IsLeavingGame())
                break;
            telemetryManagerInstance->RecordBanLevel(pad);
            if (gProfile->GetPrimaryPad() == pad)
                SetActionInternal(pad, eXuiAction_ExitWorld, NULL);
            else
                SetActionInternal(pad, eXuiAction_RemoveLocalPlayer, NULL);
            break;
        case eXuiAction_BannedLevelPrompt:
        {
            int parameters[2] = {static_cast<int>(0xdccbba45U), static_cast<int>(IDS_EXIT_GAME)};
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(pad);
            if (player == NULL)
            {
                SetActionInternal(pad, eXuiAction_Idle, NULL);
                break;
            }
            wchar_t message[40];
            swprintf(message, 40, L"%ls\n\n%%ls", player->GetOnlineName());
            if (gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_BANNED_LEVEL_TITLE),
                                                         static_cast<int>(IDS_PLAYER_BANNED_LEVEL), parameters, 2, pad,
                                                         BannedLevelDialogReturned, this, message, true)
                != 1)
                SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        }
        case eXuiAction_ReloadTexturePack:
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            Minecraft::GetInstance()->fjReloadTexturePack();
            break;
        case eXuiAction_ReloadTexturePackInTransition:
            SetActionInternal(pad, eXuiAction_GameModeTransitionInProgress, NULL);
            Minecraft::GetInstance()->fjReloadTexturePack();
            break;
        case eXuiAction_ReloadSkin:
            gConsoleUIController.SetTooltips(pad, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                             false);
            gConsoleUIController.ReloadSkin();
            gConsoleUIController.StartReloadSkinThread();
            gConsoleUIController.setCleanupOnReload(true);
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        case eXuiAction_TexturePackMissing:
        {
            int parameters[2] = {IDS_TEXTUREPACK_FULLVERSION, IDS_TEXTURE_PACK_TRIALVERSION};
            gConsoleUIController.RequestErrorMessage(
                IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, parameters, 2,
                gProfile->GetPrimaryPad(), TexturePackDialogReturned, this, NULL, true);
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        }
        case eXuiAction_GameModeTransition:
        {
            SetActionInternal(pad, eXuiAction_GameModeTransitionInProgress, NULL);
            gConsoleUIController.CloseAllPlayersScenes(true);
            for (int player = 0; player < 4; ++player)
            {
                if (minecraft->mGameModes[player] != NULL && minecraft->mGameModes[player]->getTutorial() != NULL)
                    minecraft->mGameModes[player]->getTutorial()->setMessage(std::wstring(L""), -1, 0);
            }
            gConsoleUIController.HideAllGameUIElements();
            gConsoleUIController.ShowOtherPlayersBaseScene(gProfile->GetPrimaryPad(), false);
            _LoadingInputParams* params = new _LoadingInputParams();
            params->m_threadParam = param;
            params->m_threadProc = &GameModeTransitionThreadProc;
            params->m_miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();
            FullscreenProgressState* completionData = new FullscreenProgressState;
            completionData->closeMode = 3;
            completionData->playerId = gProfile->GetPrimaryPad();
            params->m_completionData = completionData;
            params->m_completeFunc = &GameModeTransitionComplete;
            params->m_completeFuncParam = reinterpret_cast<void*>(
                static_cast<intptr_t>(static_cast<GameModeTransitionParams*>(param)->mReturningToLobby));
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                                 EUILayer_Scene, EUIGroup_PlayerDefault);
            break;
        }
        case eXuiAction_Reset:
            SetActionInternal(pad, eXuiAction_Idle, NULL);
            break;
        default:
            break;
        }

        switch (gApp.GetTMSAction(pad))
        {
        case 1:
            SetTMSAction(pad, eTMSAction_Default);
            break;
        case 2:
        case 3:
            SetTMSAction(pad, static_cast<eTMSAction>(6));
            break;
        case 4:
        case 5:
            SetTMSAction(pad, static_cast<eTMSAction>(0x13));
            break;
        case 6:
            SetTMSAction(pad, static_cast<eTMSAction>(8));
            break;
        case 8:
            SetTMSAction(pad, static_cast<eTMSAction>(10));
            break;
        case 10:
            SetTMSAction(pad, static_cast<eTMSAction>(0xc));
            break;
        case 0xc:
            SetTMSAction(pad, static_cast<eTMSAction>(0xe));
            break;
        case 0xe:
            SetTMSAction(pad, static_cast<eTMSAction>(0x10));
            break;
        case 0x10:
            SetTMSAction(pad, static_cast<eTMSAction>(1));
            break;
        case 0x12:
            SetTMSAction(pad, eTMSAction_Default);
            break;
        case 0x13:
            SetTMSAction(pad, static_cast<eTMSAction>(0x12));
            break;
        case 0x14:
            SetTMSAction(pad, static_cast<eTMSAction>(0x13));
            break;
        default:
            break;
        }
    }
    LeaveCriticalSection(&mActionMutex);
}
void CMinecraftApp::FatalLoadError(const wchar_t* message, ...)
{
}
void CMinecraftApp::SetSkinAdjustments(uint32_t id, _SkinAdjustments value)
{
    EnterCriticalSection(&mSkinAdjustmentsMutex);
    mSkinAdjustments[id] = value;
    LeaveCriticalSection(&mSkinAdjustmentsMutex);
}
_SkinAdjustments CMinecraftApp::GetSkinAdjustments(uint32_t id)
{
    EnterCriticalSection(&mSkinAdjustmentsMutex);
    _SkinAdjustments result;
    if (!mSkinAdjustments.empty())
    {
        boost::unordered_map<unsigned int, _SkinAdjustments>::iterator found = mSkinAdjustments.find(id);
        if (found != mSkinAdjustments.end())
            result = found->second;
    }
    LeaveCriticalSection(&mSkinAdjustmentsMutex);
    return result;
}

namespace boost
{
void throw_exception(const std::exception& e)
{
}
}
