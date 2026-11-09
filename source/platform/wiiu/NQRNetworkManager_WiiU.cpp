#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "game/core/StringIds.hpp"


#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <coreinit/systeminfo.h>
#include <boost/unordered_set.hpp>
#include <nn/ac/ac_cpp.h>
#include <nn/fp/fp_cpp.h>

#include "game/core/System.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSessionData.hpp"
#include "game/core/SampleNotificationEventHandler.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"
#include "platform/wiiu/Shared_Transport_Setup.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DataStoreClientTemplate.hpp"
#include "nn/nex/DataStoreSearchParam.hpp"
#include "nn/nex/DataStoreSearchResult.hpp"
#include "nn/nex/ErrorCodeConverter.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/common/CachedPrint.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/Trace.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/inet/Api.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexJoinRandomSessionSetting.hpp"
#include "nn/pia/inet/NexJoinSessionSetting.hpp"
#include "nn/pia/inet/NexNetworkFactory.hpp"
#include "nn/pia/inet/NexSessionStartupSetting.hpp"
#include "nn/pia/inet/NexUpdateSessionSetting.hpp"
#include "nn/pia/lan/LanSessionStartupSetting.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/ISessionInfo.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionSupport.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/Shared_Nex_Setup.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

static NQRNetworkManager_WiiU* s_networkManager;
static unsigned int s_connectedLocalPlayerMask;
static int (*s_connectCallback)(void*, bool, int);
static void* s_connectCallbackData;
static int (*s_connectProgressCallback)(void*, bool, int, int);
static void* s_connectProgressCallbackData;
static int s_connectProgressUserIndex;
static int s_connectProgressControllerId;
static bool s_cancelConnect;
static bool s_connectShowProgress;
static bool s_connectCompleted;
static bool s_connectThreadRunning;
static bool s_connectRetryPending;
static bool s_connectNetworkErrorPending;
static bool s_appSuspended;
bool g_bGameBootInvitePending;
bool g_bHandlingGameBootInvite;
static int s_lastLoginErrorCode;
static bool s_connectFailed;
static int s_tickThreadProcessor = 2;
static bool s_tickConnectionsOnNetworkThread = false;
static const int cTickThreadWaitTime = 32;
static const unsigned int s_nexDispatchTimeout = 0xFFFFFFFFu;
static const unsigned int s_loginErrorServiceUnavailable = 0xf9bbf;
static const unsigned int cErrorCode118_0008 = 0x120168;
static const unsigned int cErrorCode118_0541 = 0x12037d;
static const unsigned int cErrorCode118_0543 = 0x12037f;
static const unsigned int cErrorCode118_0544 = 0x120380;
static const int cPiaResultJoinSessionRefused = 0x6434;
static const int cPiaResultJoinSessionFull = 0x6c4c;
static const int cAllLocalPlayersMask = 0xf;
static const unsigned int cReceiveBufferSize = 0x20000;
static const int s_rateControlledEventInterval = 10000;
static const long long cSessionUpdateInterval = 6000;
static const unsigned int cMaxFriends = 100;
static const unsigned int cNetworkNameLength = 20;
static const int cTerminateLeaveRoomTicks = 200;
static const unsigned int cTerminateLeaveRoomSleepTime = 10;
static const unsigned int cNexCallWaitForever = 0xFFFFFFFFu;
static const unsigned int s_nexGameServerId = 0x101D9D00u;
static const unsigned int cPresenceMatchmakeSystemType = 2;
static const wchar_t* const s_nexAccessKey =
#include "platform/wiiu/shaders/NexAccessKey.inc"
    ;
static const unsigned int cPiaCachedPrintBufferSize = 512 * 1024;
static const int cPiaSchedulerThreadPriority = 16;
static const unsigned int cPiaWorkMemorySize = 4 * 1024 * 1024;
static const unsigned long long cPiaTraceFlags = 0x4008123;
static const unsigned int cPiaSdkVersion = 0x5335;
static const unsigned char cNexMajorVersion = 3;
static const unsigned char cNexMinorVersion = 10;
static const unsigned char cNexMicroVersion = 0;
static const unsigned int cNexRuntimeMemorySize = 1024 * 1024;
static const unsigned int cNexReservedMemorySize = 50 * 1024;
static const unsigned int cNexLoginTimeout = 30000;
static const unsigned short cPiaMaxStationNum = 8;
static const unsigned int cPiaSendPacketNum = 32;
static const unsigned int cPiaReceivePacketNum = 64;
static const unsigned short cPiaUnreliablePort = 0;
static const unsigned short cPiaReliablePort = 0;
static const unsigned int cPiaReliableSendBufferNum = 32;
static const unsigned int cPiaReliableReceiveBufferNum = 64;
static const unsigned int cPiaUnreliableReceiveBufferNum = 32;
static const unsigned short cPiaBrowseSessionInfoListSize = 20;

static unsigned char s_piaWorkMemory[cPiaWorkMemorySize];

static const unsigned int cLanSessionCryptoKeySize = 16;
static const unsigned char s_lanSessionCryptoKey[cLanSessionCryptoKeySize] = {
    0x9a, 0x1c, 0xf3, 0x80, 0xf3, 0xfc, 0x32, 0x5c, 0xa7, 0x0f, 0x4b, 0xc7, 0xd5, 0xe6, 0x36, 0x97};

bool NQRNetworkManager_WiiU::s_applicationConnected;

extern "C" int socket_lib_init();
extern "C" int socket_lib_finish();

void checkPiaCoreToUse();

static nn::nex::DataStoreClient* s_dataStoreClient;
static nn::nex::MatchmakeExtensionClient* s_matchmakeClient;
static bool s_matchmakeHandlerRegistered;
static unsigned int s_lastNetworkErrorCode;
static nn::pia::Result s_lastPiaResult = 0;
static bool s_friendPresenceLoginPending;
static bool s_friendPresenceOnline;
static bool s_friendLoginShutdownPending;
static bool s_acConnectPending;
static bool s_applicationConnectedPrevious;
static bool s_connectionRecheckPending;
static int s_unknownStatic;
static OSFastMutex s_connectMutex;
static NQRNetworkManager::PresenceSyncInfo s_presenceInfo;
static nn::fp::FriendPresence s_gameBootInvitePresence;
unsigned int g_uiGameBootInvitePrincipalId;
static const unsigned int cNetworkProtocolVersion = 0x881;
static const unsigned char cRoomSyncChannel = 0xff;
static const unsigned char cJoinChannel = 0xfe;
static const unsigned char cLeaveChannel = 0xfd;
static const unsigned char cJoinFailedChannel = 0xfc;
static const unsigned int cPresenceJoinGameMode = 1;

static const NQRNetworkManager::eNQRNetworkManagerState s_internalToExternalState[] = {
    NQRNetworkManager::eNQRNetworkManagerState_Uninitialised, NQRNetworkManager::eNQRNetworkManagerState_Initialising,
    NQRNetworkManager::eNQRNetworkManagerState_Idle,          NQRNetworkManager::eNQRNetworkManagerState_Idle,
    NQRNetworkManager::eNQRNetworkManagerState_Hosting,       NQRNetworkManager::eNQRNetworkManagerState_Hosting,
    NQRNetworkManager::eNQRNetworkManagerState_Hosting,       NQRNetworkManager::eNQRNetworkManagerState_Hosting,
    NQRNetworkManager::eNQRNetworkManagerState_Hosting,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Ending,        NQRNetworkManager::eNQRNetworkManagerState_Starting,
    NQRNetworkManager::eNQRNetworkManagerState_Playing,       NQRNetworkManager::eNQRNetworkManagerState_Leaving,
    NQRNetworkManager::eNQRNetworkManagerState_Leaving,       NQRNetworkManager::eNQRNetworkManagerState_Ending};

static const long long cConnectProgressMinimumTime = 2000;
static const int cConnectProgressStageSleepTime = 10;
static const int cConnectProgressLoginPercentage = 50;
static const int cConnectingToNintendoNetworkTextId = 0x5f33ba82;
static const int cConnectingToNetworkTextId = 0xcb861729;
static const int cGameFullTitleId = 0x1afa54b5;
static const int cGameFullTextId = 0xf18fccd0;
static const int cMiiInUseTitleId = STRING_MII_IN_USE_TITLE;
static const int cMiiInUseTextId = STRING_MII_ALREADY_IN_USE;
static const unsigned short cMaxSessionParticipants = 8;
static const unsigned int cSearchAttributeValueMax = UINT_MAX;
static const unsigned int cSearchResultOffsetRandom = UINT_MAX;
static const unsigned int cSearchResultCount = 1;

static SampleNotificationEventHandler s_matchmakeNotificationHandler;
static nn::pia::inet::NexNetworkFactory s_networkFactory;
static bool s_piaInitialised;
static bool s_piaFinalizing;
bool NQRNetworkManager_WiiU::s_piaShutdownPending = false;
static bool s_shutdownTickPending;
static bool s_shutdownForLan;
static bool s_piaSessionStarted;
static bool s_extDataRequested = false;
static unsigned int s_extDataSessionId;

nn::pia::Result NQRNetworkManager_WiiU_Pia_StartupSession(nn::pia::transport::Station::IdentificationToken* token,
                                                          char* name)
{
    nn::pia::inet::NexSessionStartupSetting setting;

    setting.m_field00 = 10000;
    setting.m_field04 = 500;
    setting.m_identificationToken = token;
    setting.m_hostMigrationEnabled = false;
    setting.m_bandwidthCheckMeasurementTime = 200;
    setting.m_bandwidthCheckIpPacketSize = 0x400;
    setting.m_recentPlayRecordEnabled = false;

    nn::pia::transport::Station::PlayerInfo playerInfo;
    playerInfo.m_language = 1;
    playerInfo.m_name.Copy(nn::pia::common::String(name));
    setting.m_playerInfo[0] = playerInfo;

    nn::pia::Result result = nn::pia::session::g_piaSession->Startup(setting);
    return result;
}

nn::pia::Result NQRNetworkManager_Lan_WiiU_Pia_StartupSession(nn::pia::transport::Station::IdentificationToken* token,
                                                              char* name)
{
    nn::pia::lan::LanSessionStartupSetting setting;

    setting.m_cryptoSetting.m_mode = nn::pia::common::Crypto::cModeAesGcm;
    setting.m_field00 = 10000;
    setting.m_field04 = 500;
    setting.m_identificationToken = token;
    for (unsigned int index = 0; index < cLanSessionCryptoKeySize; ++index)
        setting.m_cryptoSetting.m_key[index] = s_lanSessionCryptoKey[index];

    nn::pia::transport::Station::PlayerInfo playerInfo;
    playerInfo.m_language = 1;
    playerInfo.m_name.Copy(nn::pia::common::String(name));
    setting.m_playerInfo[0] = playerInfo;

    nn::pia::Result result = nn::pia::session::g_piaSession->Startup(setting);
    return result;
}

static unsigned int Nex_FinalizeDataStore()
{
    if (s_dataStoreClient != NULL)
    {
        s_dataStoreClient->Unbind();
        delete s_dataStoreClient;
        s_dataStoreClient = NULL;
    }
    return 0;
}

unsigned int Nex_FinalizeMatchmake()
{
    if (s_matchmakeClient != NULL)
    {
        if (s_matchmakeHandlerRegistered)
        {
            s_matchmakeClient->Unbind();
            s_matchmakeHandlerRegistered = false;
        }
        if (Nex_GetNgsFacadeInstance() != NULL)
            Nex_GetNgsFacadeInstance()->UnregisterNotificationEventHandler(&s_matchmakeNotificationHandler);
        delete s_matchmakeClient;
        s_matchmakeClient = NULL;
    }
    return 0;
}

void checkPiaCoreToUse()
{
}

void AssertPiaError(nn::pia::Result& result)
{
}

void* Nex_GetMatchmakeClient()
{
    return s_matchmakeClient;
}

unsigned int Nex_InitializeMatchmake()
{
    s_matchmakeClient = new nn::nex::MatchmakeExtensionClient();
    if (!s_matchmakeClient->Bind(Nex_GetNgsFacadeInstance()->GetCredentials()))
        return 0;
    Nex_GetNgsFacadeInstance()->RegisterNotificationEventHandler(&s_matchmakeNotificationHandler);
    s_matchmakeHandlerRegistered = true;
    return 1;
}

unsigned int Nex_SearchObject()
{
    nn::nex::ProtocolCallContext callContext(NULL);
    nn::nex::DataStoreSearchParam searchParam;
    searchParam.Reset();
    nn::nex::DataStoreSearchResult searchResult;
    nn::nex::qResult result;

    searchParam.m_uiSearchTarget = 10;
    s_dataStoreClient->SearchObject(&callContext, searchParam, &searchResult);
    callContext.Wait(0xFFFFFFFF);
    result = callContext.GetResult();
    if (!result)
        return 0;

    return 0;
}

void StartNatSessionCallback(nn::nex::CallContext* context, const void* userContext)
{
    nn::nex::ConnectivityManager* manager = nn::nex::ConnectivityManager::GetInstanceWithoutNetworkCheck();
    if (userContext != NULL && *static_cast<void* const*>(userContext) == manager)
        manager->OnStartNATSession(context);
}

NQRNetworkManager_WiiU::ClientJoinData::ClientJoinData()
{
}

NQRNetworkManager_WiiU::NQRNetworkManager_WiiU(INQRNetworkManagerListener* listener)
{
    m_state = eNQRNetworkManagerState_Uninitialised;
    m_nextSmallId = 0;
    m_sessionJoined = false;
    m_friendSearchState = eNQRNetworkManagerFriendSearchState_Idle;
    m_host = false;
    m_nextIdleReasonIsFailure = false;
    m_internalState = eNQRNetworkManagerInternalState_Uninitialised;
    m_unknown11E8 = false;
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    m_updateRequested = false;
    s_acConnectPending = false;
    m_unknown1354 = false;
    m_nexLoginState = eNEXLoginState_Failed;
    s_unknownStatic = 0;
    m_signedInLive = 0;
    m_localGame = false;
    m_nexCallContext = NULL;
    m_unknownDCD = false;
    m_friendPresenceUpdated = false;
    m_listener = listener;
    m_unknownE78 = 0;
    s_applicationConnected = false;
    s_friendPresenceOnline = false;
    m_unknownDD4 = false;
    m_unknown1378 = 0;
    s_applicationConnectedPrevious = false;
    m_randomJoinRequested = false;
    m_receiving = false;
    m_unknown11E0 = 0;
    m_leaveRequested = false;
    s_friendPresenceLoginPending = false;
    m_unknownDD0 = 0;
    s_connectionRecheckPending = false;
    m_createRequested = false;
    m_browseRequested = false;
    m_lastSessionUpdateTime = 0;
    m_joinMatchmakeRequested = false;
    s_piaShutdownPending = false;
    m_unknown13A0 = false;
    m_joinRequested = false;
    m_unknown1250 = false;
    m_unknownDC8 = -1;
    m_gatheringHolder = NULL;
    SetNexAsyncProcess(eNEXAsyncProcess_None);
    m_presenceGameMode.joinAvailabilityFlag = FALSE;
    m_presenceGameMode.matchmakeSystemType = cPresenceMatchmakeSystemType;
    m_presenceGameMode.joinGameId = s_nexGameServerId;
    m_presenceGameMode.joinGameMode = 0;
    m_presenceGameMode.ownerPrincipalId = 0;
    m_presenceGameMode.joinGroupId = 0;
    std::memset(m_presenceGameMode.applicationArg, 0, sizeof(m_presenceGameMode.applicationArg));
    s_networkManager = this;
    m_matchmaking = false;
    std::memset(m_roomPlayers, 0, sizeof(m_roomPlayers));
    InitializeCriticalSection(&m_playerMutex);
    InitializeCriticalSection(&m_playerConnectionMutex);
    InitializeCriticalSection(&m_stateChangeLock);
    InitializeCriticalSection(&m_localPlayerRequestLock);
    InitializeCriticalSection(&m_sendQueueMutex);
    InitializeCriticalSection(&m_friendSearchMutex);
    InitializeCriticalSection(&s_connectMutex);
    InitializeCriticalSection(&m_connectedPlayerMutex);
    InitializeCriticalSection(&m_connectedStationMutex);
    m_roomAttributeCount = 0;
    m_inviteConnectPromptState = eInviteConnectPrompt_None;
    m_lastRateControlledEvent = 0;
    m_externalDataSize = 0;
    m_matchmakingExtData = NULL;
    m_roomAttributes = NULL;
}

NQRNetworkManager_WiiU::~NQRNetworkManager_WiiU()
{
}

void NQRNetworkManager_WiiU::SetNexAsyncProcess(ENEXAsyncProcess process)
{
    m_nexAsyncProcess = process;
}

int NQRNetworkManager_WiiU::ConnectOnlineNintendo_Returned(void*, bool connected, int iPad)
{
    if (connected)
    {
        int (*callback)(void*, bool, int, int) = s_connectProgressCallback;
        if (callback != NULL)
            callback(s_connectProgressCallbackData, gProfile->IsSignedInLive(0), s_connectProgressUserIndex,
                     s_connectProgressControllerId);
    }
    else
    {
        if (s_lastLoginErrorCode != 0)
            g_pInput->RequestErrorViewer(iPad, s_lastLoginErrorCode, NULL, NULL);
        s_connectNetworkErrorPending = true;
    }
    return 0;
}

int NQRNetworkManager_WiiU::ConnectOnlineNintendoThreadProc(void*)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    s_connectThreadRunning = true;
    minecraft->mProgressRenderer->progressStagePercentage(0);
    minecraft->mProgressRenderer->progressStart(cConnectingToNintendoNetworkTextId);
    minecraft->mProgressRenderer->progressStage(IDS_CONNECTING_TO_NETWORK);
    long long startTime = System::processTimeInMilliSecs();
    while (true)
    {
        if (System::processTimeInMilliSecs() - startTime >= cConnectProgressMinimumTime && !s_connectRetryPending)
        {
            EnterCriticalSection(&s_connectMutex);
            bool connectPending = s_acConnectPending;
            LeaveCriticalSection(&s_connectMutex);
            if (!connectPending)
            {
                EnterCriticalSection(&s_connectMutex);
                bool loginPending = s_friendPresenceLoginPending;
                LeaveCriticalSection(&s_connectMutex);
                if (!loginPending)
                    break;
            }
        }
        if (s_cancelConnect)
            break;
        EnterCriticalSection(&s_connectMutex);
        bool loginPending = s_friendPresenceLoginPending;
        LeaveCriticalSection(&s_connectMutex);
        if (!loginPending)
        {
            minecraft->mProgressRenderer->progressStage(IDS_CONNECTING_TO_NETWORK);
            minecraft->mProgressRenderer->progressStagePercentage(0);
        }
        else
        {
            minecraft->mProgressRenderer->progressStage(cConnectingToNetworkTextId);
            minecraft->mProgressRenderer->progressStagePercentage(cConnectProgressLoginPercentage);
        }
        Sleep(cConnectProgressStageSleepTime);
    }
    s_connectThreadRunning = false;
    return 0;
}

void NQRNetworkManager_WiiU::CancelConnectCallbacks()
{
    s_connectProgressCallback = NULL;
    s_connectShowProgress = false;
    s_connectCallback = NULL;
    s_connectCallbackData = NULL;
}

void NQRNetworkManager_WiiU::CancelConnectOnlineNintendo(void* param)
{
    s_cancelConnect = true;
    static_cast<NQRNetworkManager_WiiU*>(param)->CancelConnectCallbacks();
}

int NQRNetworkManager_WiiU::ShowConnectOnlineNintendoProgress(int userIndex, int (*callback)(void*, bool, int, int),
                                                              void* callbackData)
{
    s_connectProgressCallbackData = callbackData;
    s_connectProgressUserIndex = userIndex;
    s_connectProgressCallback = callback;
    s_connectProgressControllerId = g_pInput->GetControllerID(userIndex);
    CGameNetworkManager::sInstance.TryToConnectOnline(ConnectOnlineNintendo_Returned, NULL, true);
    s_connectNetworkErrorPending = false;
    s_cancelConnect = false;
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = ConnectOnlineNintendoThreadProc;
    params->m_threadParam = NULL;
    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->closeMode = 6;
    progressState->playerId = userIndex;
    params->m_completionData = progressState;
    params->m_cancelFunc = CancelConnectOnlineNintendo;
    params->m_cancelFuncParam = NULL;
    params->m_cancelText = IDS_TOOLTIPS_CANCEL;
    gConsoleUIController.NavigateToScene(userIndex, EUIScene_FullscreenProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
    return 0;
}

int NQRNetworkManager_WiiU::InvitationSignInReturned(void* context, bool, int, int)
{
    static_cast<NQRNetworkManager_WiiU*>(context)->m_inviteConnectPromptState = eInviteConnectPrompt_Answered;
    return 0;
}

int NQRNetworkManager_WiiU::InvitationMustConnectReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept)
    {
        if (pad == 0xff)
            pad = gProfile->GetPrimaryPad();
        ShowConnectOnlineNintendoProgress(pad, InvitationSignInReturned, context);
    }
    else
    {
        static_cast<NQRNetworkManager_WiiU*>(context)->m_inviteConnectPromptState = eInviteConnectPrompt_Answered;
    }
    return 0;
}

void NQRNetworkManager_WiiU::UpdateInvitationChecks()
{
    if (m_inviteConnectPromptState != eInviteConnectPrompt_Showing && g_bGameBootInvitePending
        && g_bSafeToRespondToGameBootInvite && !m_unknown1354)
    {
        bool presenceFound = true;
        if (nn::fp::GetFriendPresence(&s_gameBootInvitePresence, &g_uiGameBootInvitePrincipalId, 1).IsFailure())
            presenceFound = false;
        int messageId = IDS_DISCONNECTED_SERVER_QUIT;
        if (m_signedInLive == 0)
        {
            if (m_inviteConnectPromptState == eInviteConnectPrompt_None)
            {
                int buttons[2] = {IDS_PRO_NOTONLINE_ACCEPT, IDS_PRO_NOTONLINE_DECLINE};
                gConsoleUIController.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, buttons, 2,
                                                         0xff, InvitationMustConnectReturned, this, NULL, true);
                m_inviteConnectPromptState = eInviteConnectPrompt_Showing;
                return;
            }
            messageId = IDS_CONNECTION_LOST_LIVE_NO_EXIT;
        }
        g_bGameBootInvitePending = false;
        const PresenceSyncInfo* presenceInfo = reinterpret_cast<const PresenceSyncInfo*>(
            s_gameBootInvitePresence.gameMode.applicationArg);
        if (presenceFound && s_gameBootInvitePresence.isValid && s_gameBootInvitePresence.isOnline
            && presenceInfo->m_usTitleVersion != 0)
        {
            if (presenceInfo->m_usTitleVersion == cNetworkProtocolVersion)
            {
                if (presenceInfo->m_bJoinable)
                    return;
                if (!g_pInput->IsPadLocked(gProfile->GetPrimaryPad()))
                {
                    if (g_iGameBootInviteUserIndex > 0)
                    {
                        g_pInput->LockPad(g_iGameBootInviteUserIndex, gProfile->GetPrimaryPad());
                        g_iGameBootInviteUserIndex = -1;
                    }
                    if (!g_pInput->IsPadLocked(gProfile->GetPrimaryPad())
                        && !g_pInput->LockPad(4, gProfile->GetPrimaryPad())
                        && !g_pInput->LockPad(5, gProfile->GetPrimaryPad())
                        && !g_pInput->LockPad(6, gProfile->GetPrimaryPad())
                        && !g_pInput->LockPad(7, gProfile->GetPrimaryPad())
                        && !g_pInput->LockPad(8, gProfile->GetPrimaryPad()))
                        return;
                }
                m_listener->HandleInviteReceived(gProfile->GetPrimaryPad(), &s_gameBootInvitePresence);
                return;
            }
            messageId = presenceInfo->m_usTitleVersion < cNetworkProtocolVersion ? IDS_DISCONNECTED_SERVER_OLD
                                                                                 : IDS_DISCONNECTED_CLIENT_OLD;
        }
        int okButton = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, messageId, &okButton, 1, 0xff, NULL, NULL, NULL,
                                                 true);
    }
}

void NQRNetworkManager_WiiU::Tick()
{
    UpdateInvitationChecks();
    m_tickEvent->Set();
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId,
                                                                            int localIndex)
{
    for (unsigned int index = 0; index < m_players.size(); ++index)
    {
        if (m_players[index]->GetStationId() == stationId
            && m_players[index]->GetLocalIndex() == static_cast<unsigned int>(localIndex))
            return m_players[index];
    }
    return NULL;
}

void NQRNetworkManager_WiiU::RemoveConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    for (unsigned int index = 0; index < m_connectedStations.size(); ++index)
    {
        ConnectedStation& station = m_connectedStations[index];
        if (station.stationId == stationId)
        {
            station = m_connectedStations.back();
            m_connectedStations.pop_back();
            LeaveCriticalSection(&m_connectedStationMutex);
            return;
        }
    }
    LeaveCriticalSection(&m_connectedStationMutex);
}

void NQRNetworkManager_WiiU::RemoveConnectedPlayer(NQRNetworkPlayer* player)
{
    EnterCriticalSection(&m_connectedPlayerMutex);
    unsigned long long stationId = player->GetStationId();
    if (player->IsLocal())
        s_connectedLocalPlayerMask &= ~(1 << player->GetLocalPlayerIndex());
    for (unsigned int index = 0; index < m_players.size(); ++index)
    {
        if (m_players[index] == player)
        {
            m_players[index] = m_players.back();
            m_players.pop_back();
            break;
        }
    }
    NintendoVoiceChat_WiiU::RemovePlayer(player);
    for (unsigned int index = 0; index < m_players.size(); ++index)
    {
        if (m_players[index]->GetStationId() == stationId)
        {
            LeaveCriticalSection(&m_connectedPlayerMutex);
            return;
        }
    }
    RemoveConnectedStation(stationId);
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

void NQRNetworkManager_WiiU::NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId)
{
    player->ConnectionComplete();
    player->SmallIdAllocated(smallId);
}

void NQRNetworkManager_WiiU::AddConnectedPlayer(NQRNetworkPlayer* player)
{
    EnterCriticalSection(&m_connectedPlayerMutex);
    m_players.push_back(player);
    if (player->IsLocal())
        s_connectedLocalPlayerMask |= 1 << player->GetLocalPlayerIndex();
    NintendoVoiceChat_WiiU::AddPlayer(player);
    if (player->IsLocal())
    {
        PlayerNameMap::iterator existing = m_miiNames.find(player->GetNetworkNameW());
        if (existing != m_miiNames.end())
            m_miiNames.erase(existing);
        m_miiNames.insert(std::make_pair(player->GetNetworkNameW(), player->GetMiiName()));
    }
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

void NQRNetworkManager_WiiU::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (m_listener != NULL)
        m_listener->HandlePlayerJoined(player);
    if (!m_host && player->IsLocal())
        ++m_localPlayersJoined;
}

void NQRNetworkManager_WiiU::FindOrCreateNonNetworkPlayer(int slotIndex, int type, PlayerSyncData* syncData)
{
    for (std::vector<NQRNetworkPlayer*>::iterator candidate = m_unconnectedPlayers.begin();
         candidate != m_unconnectedPlayers.end(); ++candidate)
    {
        NQRNetworkPlayer* player = *candidate;
        if (player->GetType() == type && player->GetLocalIndex() == syncData->m_uiLocalPlayerIndex
            && (type != NQRNetworkPlayer::eNQRNetworkPlayerType_Remote
                || player->GetStationId() == syncData->m_uiStationId))
        {
            m_unconnectedPlayers.erase(candidate);
            m_roomSlotPlayers[slotIndex] = player;
            return;
        }
    }

    PlayerUID localUID(0);
    PlayerUID* uid;
    if (type == NQRNetworkPlayer::eNQRNetworkPlayerType_Local
        || (m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Host))
    {
        gProfile->GetXUID(syncData->m_uiLocalPlayerIndex, &localUID, true);
        if (localUID == PlayerUID(0))
            gProfile->GetXUID(syncData->m_uiLocalPlayerIndex, &localUID, false);
        uid = &localUID;
    }
    else
    {
        uid = syncData;
    }
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        if (m_roomPlayers[index] == *uid)
            uid->flags |= PlayerUID::FlagMiiPrincipal;

    NQRNetworkPlayer* player = new NQRNetworkPlayer(this, static_cast<NQRNetworkPlayer::eNQRNetworkPlayerType>(type),
                                                    m_host, syncData->m_uiStationId, syncData->m_uiLocalPlayerIndex,
                                                    uid, syncData->m_szNetworkName, syncData->m_szMiiName);
    NonNetworkPlayerComplete(player, syncData->m_uiSmallId);
    AddConnectedPlayer(player);
    m_roomSlotPlayers[slotIndex] = player;
    HandlePlayerJoined(player);
}

void NQRNetworkManager_WiiU::NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId)
{
    EnterCriticalSection(&m_playerConnectionMutex);
    const bool wasReady = player->IsReady();
    player->SmallIdAllocated(smallId);
    const bool isReady = player->IsReady();
    LeaveCriticalSection(&m_playerConnectionMutex);
    if (!wasReady && isReady)
        HandlePlayerJoined(player);
}

void NQRNetworkManager_WiiU::MapRoomSlotPlayers(int previousPlayerCount)
{
    EnterCriticalSection(&m_playerMutex);
    bool removingPlayer = false;
    if (previousPlayerCount == -1)
        previousPlayerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    else
        removingPlayer = true;

    if (m_host)
    {
        for (int index = 0; index < 8; ++index)
        {
            if (m_roomSlotPlayers[index] != NULL
                && m_roomSlotPlayers[index]->GetType() != NQRNetworkPlayer::eNQRNetworkPlayerType_Remote)
            {
                m_unconnectedPlayers.push_back(m_roomSlotPlayers[index]);
                m_roomSlotPlayers[index] = NULL;
            }
        }
        for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        {
            PlayerSyncData& record = m_roomPlayers[index];
            if (index == 0)
            {
                FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Host, &record);
                static_cast<PlayerUID&>(record) = m_roomSlotPlayers[index]->GetUID();
            }
            else if (record.m_uiStationId == m_localStationId)
            {
                FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Local, &record);
                static_cast<PlayerUID&>(record) = m_roomSlotPlayers[index]->GetUID();
            }
            else
            {
                EnterCriticalSection(&m_connectedPlayerMutex);
                m_roomSlotPlayers[index] = GetPlayerFromStationIdAndLocalIdx(record.m_uiStationId,
                                                                             record.m_uiLocalPlayerIndex);
                if (m_roomSlotPlayers[index] != NULL)
                    NetworkPlayerSmallIdAllocated(m_roomSlotPlayers[index], record.m_uiSmallId);
                LeaveCriticalSection(&m_connectedPlayerMutex);
            }
        }
        if (removingPlayer && previousPlayerCount != 0)
            m_roomSlotPlayers[previousPlayerCount - 1] = NULL;
        if (m_listener != NULL)
            m_listener->HandleResyncPlayerRequest(m_roomSlotPlayers);
    }
    else
    {
        for (int index = 0; index < 8; ++index)
        {
            if (m_roomSlotPlayers[index] != NULL)
            {
                m_unconnectedPlayers.push_back(m_roomSlotPlayers[index]);
                m_roomSlotPlayers[index] = NULL;
            }
        }
        for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        {
            PlayerSyncData& record = m_roomPlayers[index];
            if (index == 0)
            {
                FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Host, &record);
                m_roomSlotPlayers[index]->SetUID(static_cast<PlayerUID&>(record));
            }
            else if (record.m_uiStationId == m_localStationId)
            {
                if (gProfile->IsSignedIn(record.m_uiLocalPlayerIndex))
                    FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Local, &record);
            }
            else
            {
                FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Remote, &record);
                m_roomSlotPlayers[index]->SetUID(static_cast<PlayerUID&>(record));
            }
        }
    }

    for (std::vector<NQRNetworkPlayer*>::iterator player = m_unconnectedPlayers.begin();
         player != m_unconnectedPlayers.end(); ++player)
    {
        if (m_listener != NULL)
            m_listener->HandlePlayerLeaving(*player);
        RemoveConnectedPlayer(*player);
        delete *player;
    }
    m_unconnectedPlayers.clear();
    LeaveCriticalSection(&m_playerMutex);
}

void NQRNetworkManager_WiiU::UpdateRoomSyncUIDsFromPlayers()
{
    EnterCriticalSection(&m_playerMutex);
    if (m_host)
        for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
            if (m_roomSlotPlayers[index] != NULL)
                static_cast<PlayerUID&>(m_roomPlayers[index]) = m_roomSlotPlayers[index]->GetUID();
    LeaveCriticalSection(&m_playerMutex);
}

void NQRNetworkManager_WiiU::RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data,
                                            unsigned int size)
{
    EnterCriticalSection(&m_sendQueueMutex);
    SendQueueItem item;
    item.stationId = stationId;
    item.data = static_cast<unsigned char*>(std::malloc(size + 1));
    item.size = size + 1;
    item.data[0] = channel;
    std::memcpy(item.data + 1, data, size);
    m_sendQueue.push_back(item);
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_WiiU::SyncRoomData()
{
    if (!m_localGame)
    {
        UpdateRoomSyncUIDsFromPlayers();
        RemoteDataSend(0, cRoomSyncChannel, m_roomPlayers, sizeof(m_roomPlayers));
    }
}

void NQRNetworkManager_WiiU::RemoveRemotePlayersAndSync(unsigned long long stationId, int playerMask)
{
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;)
    {
        if (m_roomPlayers[index].m_uiStationId == stationId
            && ((1 << m_roomPlayers[index].m_uiLocalPlayerIndex) & playerMask) != 0)
        {
            EnterCriticalSection(&m_connectedPlayerMutex);
            NQRNetworkPlayer* player = GetPlayerFromStationIdAndLocalIdx(stationId,
                                                                         m_roomPlayers[index].m_uiLocalPlayerIndex);
            if (player != NULL)
            {
                if (m_listener != NULL)
                    m_listener->HandlePlayerLeaving(player);
                RemoveConnectedPlayer(player);
                delete player;
            }
            m_roomPlayers[0].m_uiPlayerCountMinusOne--;
            for (int slot = index; slot < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++slot)
            {
                m_roomPlayers[slot] = m_roomPlayers[slot + 1];
                m_roomSlotPlayers[slot] = m_roomSlotPlayers[slot + 1];
            }
            std::memset(&m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1], 0, sizeof(PlayerSyncData));
            m_roomSlotPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1] = NULL;
            LeaveCriticalSection(&m_connectedPlayerMutex);
        }
        else
            ++index;
    }
    LeaveCriticalSection(&m_playerMutex);
    MapRoomSlotPlayers(-1);
    SyncRoomData();
}

void NQRNetworkManager_WiiU::ClearSendQueue()
{
    while (!m_sendQueue.empty())
    {
        const SendQueueItem& item = m_sendQueue.front();
        std::free(item.data);
        m_sendQueue.pop_front();
    }
}

void NQRNetworkManager_WiiU::SetState(eNQRNetworkManagerInternalState state)
{
    if (state == eNQRNetworkManagerInternalState_Idle)
        m_matchmaking = false;
    eNQRNetworkManagerState oldState = s_internalToExternalState[m_internalState];
    eNQRNetworkManagerState newState = s_internalToExternalState[state];
    bool idleReasonIsFailure = false;
    if (state == eNQRNetworkManagerInternalState_Idle && m_nextIdleReasonIsFailure)
    {
        idleReasonIsFailure = true;
        m_nextIdleReasonIsFailure = false;
    }
    m_internalState = state;
    if (newState != oldState || idleReasonIsFailure)
    {
        EnterCriticalSection(&m_stateChangeLock);
        m_stateChanges.push_back(StateChangeInfo(oldState, newState, idleReasonIsFailure));
        LeaveCriticalSection(&m_stateChangeLock);
    }
}

void NQRNetworkManager_WiiU::TickSendQueue()
{
    nn::pia::transport::Transport* transport = nn::pia::transport::g_piaTransport;
    if (transport == NULL)
        return;
    EnterCriticalSection(&m_sendQueueMutex);
    nn::pia::transport::ReliableProtocol* protocol =
        transport->m_protocolManager.GetProtocol<nn::pia::transport::ReliableProtocol>(m_reliableProtocolHandle);
    while (!m_sendQueue.empty())
    {
        SendQueueItem item = m_sendQueue.front();
        if (item.stationId == 0 || protocol->IsInCommunication(item.stationId))
        {
            nn::pia::Result result;
            if (item.stationId == 0)
                result = protocol->SendToAll(item.data, item.size);
            else
                result = protocol->Send(item.stationId, item.data, item.size);
            if (result != 0)
            {
                if (result == nn::pia::transport::cResultWindowOverflow)
                    break;
            }
        }
        else if (m_host)
        {
            RemoveRemotePlayersAndSync(item.stationId, cAllLocalPlayersMask);
        }
        else if (m_internalState == eNQRNetworkManagerInternalState_Playing)
        {
            ClearSendQueue();
            SetState(eNQRNetworkManagerInternalState_Ending);
            LeaveCriticalSection(&m_sendQueueMutex);
            return;
        }
        std::free(item.data);
        m_sendQueue.pop_front();
    }
    LeaveCriticalSection(&m_sendQueueMutex);
}

nn::pia::Result NQRNetworkManager_WiiU::Pia_InitializeCommon(unsigned char* memory, unsigned int memorySize,
                                                             unsigned long long traceFlags, bool enableCachedPrint,
                                                             bool)
{
    checkPiaCoreToUse();
    nn::pia::Result result = 0;
    result = nn::pia::common::Initialize(memory, memorySize);
    AssertPiaError(result);
    result = nn::pia::common::BeginSetup();
    AssertPiaError(result);
    if (enableCachedPrint)
    {
        result = nn::pia::common::CachedPrint::CreateInstance(cPiaCachedPrintBufferSize);
        AssertPiaError(result);
    }
    result = nn::pia::common::Trace::CreateInstance();
    AssertPiaError(result);
    nn::pia::common::Trace::GetInstance()->SetFlag(traceFlags);
    result = nn::pia::common::Scheduler::CreateInstance(cPiaSchedulerThreadPriority);
    AssertPiaError(result);
    result = nn::pia::common::WatermarkManager::CreateInstance();
    AssertPiaError(result);
    nn::pia::common::WatermarkManager::GetInstance()->EnableAllWatermark();
    result = nn::pia::common::EndSetup();
    AssertPiaError(result);
    return result;
}

nn::pia::Result NQRNetworkManager_WiiU::Pia_InitializeInet()
{
    checkPiaCoreToUse();
    nn::pia::Result result = 0;
    result = nn::pia::inet::Initialize();
    AssertPiaError(result);
    result = nn::pia::inet::BeginSetup();
    AssertPiaError(result);
    result = nn::pia::inet::NexFacade::CreateInstance();
    AssertPiaError(result);
    result = nn::pia::inet::EndSetup();
    AssertPiaError(result);
    return result;
}

nn::pia::Result NQRNetworkManager_WiiU::Pia_InitializeSession(const nn::pia::session::Session::Setting& setting)
{
    checkPiaCoreToUse();
    nn::pia::Result result = 0;
    result = nn::pia::session::Initialize();
    AssertPiaError(result);
    result = nn::pia::session::BeginSetup();
    AssertPiaError(result);
    result = nn::pia::session::Session::CreateInstance(setting);
    AssertPiaError(result);
    result = nn::pia::session::EndSetup();
    AssertPiaError(result);
    return result;
}

NQRNetworkManager_WiiU::ConnectedStation* NQRNetworkManager_WiiU::GetConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    for (unsigned int index = 0; index < m_connectedStations.size(); ++index)
    {
        ConnectedStation* station = &m_connectedStations[index];
        if (station->stationId == stationId)
        {
            LeaveCriticalSection(&m_connectedStationMutex);
            return station;
        }
    }
    LeaveCriticalSection(&m_connectedStationMutex);
    return NULL;
}

void NQRNetworkManager_WiiU::AddConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    if (GetConnectedStation(stationId) == NULL)
    {
        ConnectedStation station;
        station.stationId = stationId;
        station.state = 0;
        m_connectedStations.push_back(station);
    }
    LeaveCriticalSection(&m_connectedStationMutex);
}

void NQRNetworkManager_WiiU::Pia_SignalLeaveSession()
{
    m_leaveRequested = true;
}

void NQRNetworkManager_WiiU::Pia_SessionCallback(nn::pia::session::Session::EventType eventType,
                                                 unsigned long long stationId, void*)
{
    switch (eventType)
    {
    case nn::pia::session::Session::cEventTypeJoin:
        s_networkManager->m_localStationId = nn::pia::session::g_piaSession->m_localStationId;
        s_networkManager->m_hostStationId = nn::pia::session::g_piaSession->m_hostStationId;
        s_networkManager->m_sessionJoined = true;
        if (s_networkManager->m_host)
        {
            if (stationId != s_networkManager->m_localStationId)
                s_networkManager->AddConnectedStation(stationId);
        }
        else
            s_networkManager->AddConnectedStation(s_networkManager->m_hostStationId);
        break;
    case nn::pia::session::Session::cEventTypeLeave:
        if (s_networkManager->m_host)
        {
            if (stationId != s_networkManager->m_localStationId)
            {
                s_networkManager->RemoveRemotePlayersAndSync(stationId, cAllLocalPlayersMask);
                s_networkManager->RemoveConnectedStation(stationId);
            }
        }
        else if (stationId == s_networkManager->m_hostStationId)
            s_networkManager->Pia_SignalLeaveSession();
        break;
    }
}

bool NQRNetworkManager_WiiU::NexFacadeLogin(unsigned int timeout)
{
    nn::pia::common::SetProgramInformation(cPiaSdkVersion, cNexMajorVersion, cNexMinorVersion, cNexMicroVersion);
    nn::nex::NgsFacade* ngsFacade = Nex_GetNgsFacadeInstance();
    s_lastNetworkErrorCode = 0;
    s_lastPiaResult = 0;
    m_nexCallContext = new nn::nex::ProtocolCallContext;
    if (!ngsFacade->Login(m_nexCallContext, s_nexGameServerId, s_nexAccessKey, timeout, 0))
        return false;
    SetNexAsyncProcess(eNEXAsyncProcess_Login);
    m_nexLoginState = eNEXLoginState_Pending;
    return true;
}

void NQRNetworkManager_WiiU::InitialiseAfterOnline()
{
    if (!gProfile->IsFullVersion())
    {
        SetState(eNQRNetworkManagerInternalState_InitialiseFailed);
        return;
    }
    s_connectNetworkErrorPending = false;
    nn::pia::Result result = Pia_InitializeCommon(s_piaWorkMemory, cPiaWorkMemorySize, cPiaTraceFlags, true, true);
    s_piaInitialised = true;
    Nex_InitializeNex(cNexRuntimeMemorySize, cNexReservedMemorySize, nn::nex::Core::THREAD_MODE_2);
    result = Pia_InitializeInet();
    nn::pia::transport::Transport::Setting transportSetting(&s_networkFactory, cPiaMaxStationNum, cPiaSendPacketNum,
                                                            cPiaReceivePacketNum, 0, true, false);
    result = Transport_InitializePiaTransport(transportSetting, cPiaUnreliablePort, &m_unreliableProtocolHandle,
                                              cPiaReliablePort, &m_reliableProtocolHandle, cPiaReliableSendBufferNum,
                                              cPiaReliableReceiveBufferNum, NULL, cPiaUnreliableReceiveBufferNum);
    NintendoVoiceChat_WiiU::InitialiseAfterOnline();
    nn::pia::session::Session::Setting sessionSetting;
    sessionSetting.m_networkFactory = &s_networkFactory;
    sessionSetting.m_networkTopologyType = 0;
    sessionSetting.m_browseSessionInfoListSize = cPiaBrowseSessionInfoListSize;
    sessionSetting.m_browseCommunityInfoListSize = 0;
    sessionSetting.m_bitRateCheckMode = 0;
    result = Pia_InitializeSession(sessionSetting);
    result = nn::pia::session::g_piaSession->RegisterSessionEventCallback(Pia_SessionCallback, NULL);
    if (socket_lib_init() != 0)
    {
        SetState(eNQRNetworkManagerInternalState_InitialiseFailed);
        return;
    }
    gApp.GetCommerceInstance()->Init();
    if (!IsInSession())
        SetState(eNQRNetworkManagerInternalState_LoggingIn);
    NexFacadeLogin(cNexLoginTimeout);
}

void NQRNetworkManager_WiiU::OnlineCheck()
{
    if (s_piaShutdownPending)
        return;
    bool signedInLive = gProfile->IsSignedInLive(gProfile->GetPrimaryPad());
    if (!m_signedInLive && signedInLive)
        InitialiseAfterOnline();
    m_signedInLive = signedInLive;
}

void NQRNetworkManager_WiiU::FriendSearchTick()
{
    if (m_friendSearchState == eNQRNetworkManagerFriendSearchState_Searching && !m_friendSearchThread->isRunning())
    {
        m_friendSearchState = eNQRNetworkManagerFriendSearchState_Complete;
        delete m_friendSearchThread;
        m_friendSearchThread = NULL;
        FriendRoomManagerSearch2();
    }
}

void NQRNetworkManager_WiiU::FriendPresenceLoginCallback(nn::Result result, void*)
{
    s_friendPresenceOnline = nn::fp::IsOnline();
    gProfile->SetFriendPresenceOnline(s_friendPresenceOnline);
    if (s_friendPresenceOnline && s_applicationConnected)
    {
        gProfile->SetNetworkStatus(true);
        s_connectFailed = false;
        s_connectCompleted = true;
    }
    else
    {
        gProfile->SetNetworkStatus(false);
        s_connectCompleted = true;
        s_friendLoginShutdownPending = true;
        s_connectFailed = true;
        s_lastLoginErrorCode = nn::fp::ResultToErrorCode(result);
    }
    s_friendPresenceLoginPending = false;
}

void NQRNetworkManager_WiiU::Session_CleanupSession()
{
    if (nn::pia::session::g_piaSession != NULL)
        nn::pia::session::g_piaSession->Cleanup();
}

bool NQRNetworkManager_WiiU::Inet_Logout()
{
    if (nn::pia::inet::NexFacade::GetInstance() != NULL)
        nn::pia::inet::NexFacade::GetInstance()->Unbind();
    nn::nex::NgsFacade* ngsFacade = Nex_GetNgsFacadeInstance();
    if (ngsFacade != NULL)
    {
        nn::nex::ProtocolCallContext callContext;
        ngsFacade->Logout(&callContext);
        callContext.Wait(cNexCallWaitForever);
        m_nexLoginState = eNEXLoginState_Failed;
    }
    return false;
}

void NQRNetworkManager_WiiU::Inet_FinalizeNetwork()
{
    socket_lib_finish();
    nn::Result result = nn::ac::Close();
    if (result.IsFailure())
    {
        nn::ac::ErrorCode errorCode;
        nn::ac::GetLastErrorCode(&errorCode);
    }
}

void NQRNetworkManager_WiiU::Session_FinalizePiaSession()
{
    if (nn::pia::session::g_piaSession != NULL)
    {
        nn::pia::session::g_piaSession->Cleanup();
        nn::pia::session::Session::DestroyInstance();
    }
    nn::pia::session::Finalize();
}

void NQRNetworkManager_WiiU::Inet_FinalizePiaInet()
{
    nn::pia::inet::NexFacade::DestroyInstance();
    nn::pia::inet::Finalize();
}

void NQRNetworkManager_WiiU::Pia_FinalizeCommon()
{
    nn::pia::common::WatermarkManager::DestroyInstance();
    nn::pia::common::Scheduler::DestroyInstance();
    nn::pia::common::Trace::DestroyInstance();
    nn::pia::common::Finalize();
}

void NQRNetworkManager_WiiU::Pia_Finalize()
{
    s_piaFinalizing = true;
    if (nn::fp::IsInitialized())
        nn::fp::Logout();
    if (m_nexCallContext != NULL)
        delete m_nexCallContext;
    m_nexCallContext = NULL;
    Nex_FinalizeMatchmake();
    Inet_FinalizeNetwork();
    Session_FinalizePiaSession();
    NintendoVoiceChat_WiiU::Shutdown();
    Transport_FinalizePiaTransport(m_unreliableProtocolHandle, m_reliableProtocolHandle, NULL);
    Inet_FinalizePiaInet();
    Nex_FinalizeNex();
    Pia_FinalizeCommon();
    s_piaFinalizing = false;
}

void NQRNetworkManager_WiiU::Pia_Shutdown()
{
    if (s_piaInitialised)
    {
        s_piaShutdownPending = true;
        if (m_nexAsyncProcess == eNEXAsyncProcess_None)
        {
            nn::nex::Core* core = nn::nex::Core::GetInstance();
            if (core != NULL)
                core->SetTerminateImmediately(true);
            m_unknown13A0 = false;
            m_listener->HandleDisconnect(false);
            System::processTimeInMilliSecs();
            if (nn::pia::session::g_piaSession != NULL)
            {
                Session_CleanupSession();
                nn::pia::session::g_piaSession->UnregisterSessionEventCallback();
            }
            Inet_Logout();
            Nex_FinalizeDataStore();
            Pia_Finalize();
            s_piaShutdownPending = false;
            s_piaSessionStarted = false;
            s_piaInitialised = false;
        }
    }
}

void NQRNetworkManager_WiiU::SetLastNetworkError(nn::pia::Result result)
{
    s_lastNetworkErrorCode = result.GetErrorCode();
    s_lastPiaResult = result;
}

void NQRNetworkManager_WiiU::UpdateApplicationConnected()
{
    EnterCriticalSection(&s_connectMutex);
    if (s_friendLoginShutdownPending)
    {
        Pia_Shutdown();
        s_friendLoginShutdownPending = false;
    }
    if (s_acConnectPending)
    {
        nn::ac::Status status;
        nn::ac::GetConnectStatus(&status);
        if (status == nn::ac::STATUS_OK)
        {
            s_acConnectPending = false;
            nn::ac::IsApplicationConnected(&s_applicationConnected);
        }
        else if (status == nn::ac::STATUS_FAILED)
        {
            s_acConnectPending = false;
            s_connectCompleted = true;
            s_connectFailed = true;
            nn::ac::ErrorCode errorCode;
            nn::ac::GetLastErrorCode(&errorCode);
            s_lastLoginErrorCode = errorCode;
        }
        if (s_acConnectPending)
        {
            LeaveCriticalSection(&s_connectMutex);
            return;
        }
    }
    if (s_piaSessionStarted && nn::pia::session::g_piaSession != NULL)
    {
        if (!CGameNetworkManager::sInstance.IsLeavingGame())
        {
            nn::pia::Result result = nn::pia::session::g_piaSession->CheckConnectionError();
            if (result != 0)
            {
                SetLastNetworkError(result);
                nn::nex::NgsFacade* ngsFacade = Nex_GetNgsFacadeInstance();
                if (ngsFacade != NULL && !(ngsFacade->IsConnected() == false)
                    && result == nn::pia::session::Session::cResultStatusDisconnectedSession)
                {
                    if (nn::pia::session::g_piaSession->GetSessionDisconnectReason()
                        == nn::pia::session::Session::cStatusUnknown8)
                    {
                        Minecraft* minecraft = Minecraft::GetInstance();
                        int primaryPad = gProfile->GetPrimaryPad();
                        minecraft->mConnectionFailed[primaryPad] = true;
                        minecraft->mDisconnectReasons[primaryPad] = DisconnectPacket::eDisconnectReason_NetworkError;
                        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
                    }
                    if (m_internalState != eNQRNetworkManagerInternalState_LeavingRoom
                        && m_internalState != eNQRNetworkManagerInternalState_Ending)
                    {
                        SetState(eNQRNetworkManagerInternalState_LeavingRoom);
                        Pia_SignalLeaveSession();
                    }
                }
                else
                {
                    Pia_Shutdown();
                }
            }
        }
    }
    else
    {
        if (s_applicationConnected)
            nn::ac::IsApplicationConnected(&s_applicationConnected);
        if (s_applicationConnectedPrevious != s_applicationConnected || s_connectionRecheckPending)
        {
            s_connectionRecheckPending = false;
            if (s_applicationConnected)
            {
                s_friendPresenceOnline = nn::fp::IsOnline();
                gProfile->SetFriendPresenceOnline(s_friendPresenceOnline);
                if (!s_friendPresenceOnline)
                {
                    nn::Result result = nn::fp::LoginAsync(FriendPresenceLoginCallback, this);
                    if (result.IsFailure())
                    {
                        s_connectFailed = true;
                        s_connectCompleted = true;
                        s_lastLoginErrorCode = nn::fp::ResultToErrorCode(result);
                    }
                    s_friendPresenceLoginPending = true;
                }
                else
                {
                    gProfile->SetNetworkStatus(true);
                    s_connectFailed = false;
                    s_connectCompleted = true;
                }
            }
            else
            {
                gProfile->SetNetworkStatus(false);
                Pia_Shutdown();
            }
            s_applicationConnectedPrevious = s_applicationConnected;
        }
        else if (s_applicationConnected && s_friendPresenceOnline)
        {
            s_friendPresenceOnline = nn::fp::IsOnline();
            gProfile->SetFriendPresenceOnline(s_friendPresenceOnline);
            if (!s_friendPresenceOnline)
                s_connectionRecheckPending = true;
        }
    }
    LeaveCriticalSection(&s_connectMutex);
}

bool NQRNetworkManager_WiiU::Pia_CreateSession()
{
    if (m_internalState == eNQRNetworkManagerInternalState_LoggingIn)
    {
        SetState(eNQRNetworkManagerInternalState_HostWaitingForLogin);
        return true;
    }
    nn::pia::inet::NexCreateSessionSetting setting;
    setting.SetGameMode(m_gameMode);
    setting.SetMaxParticipantNum(cPiaMaxStationNum);
    for (int index = 0; index < m_roomAttributeCount; ++index)
    {
        setting.SetAttribute(index, m_roomAttributes[index]);
        m_matchmakeAttributes[index] = m_roomAttributes[index];
    }
    setting.SetOpenSession(true);
    setting.SetSessionType(nn::pia::inet::cSessionType_Normal);
    setting.SetApplicationData(m_externalData, m_externalDataSize);
    m_lastSessionUpdateTime = System::processTimeInMilliSecs();
    nn::pia::Result result = nn::pia::session::g_piaSession->CreateSessionAsync(&setting);
    if (result != 0)
    {
        SetLastNetworkError(result);
        SetState(eNQRNetworkManagerInternalState_HostCreateSessionFailed);
        return false;
    }
    SetNexAsyncProcess(eNEXAsyncProcess_CreateSession);
    SetState(eNQRNetworkManagerInternalState_HostCreatingSession);
    return true;
}

bool NQRNetworkManager_WiiU::canRunRateControlledEvent()
{
    unsigned long long now = System::processTimeInMilliSecs();
    if (m_lastRateControlledEvent == 0
        || static_cast<int>(now - m_lastRateControlledEvent) >= s_rateControlledEventInterval)
    {
        m_lastRateControlledEvent = now;
        return true;
    }
    return false;
}

bool NQRNetworkManager_WiiU::Pia_JoinSession()
{
    if (m_internalState == eNQRNetworkManagerInternalState_LoggingIn)
    {
        SetState(eNQRNetworkManagerInternalState_JoinWaitingForLogin);
        return true;
    }
    nn::pia::inet::NexJoinSessionSetting setting;
    setting.SetSessionId(m_pendingSessionId);
    nn::pia::Result result = nn::pia::session::g_piaSession->JoinSessionAsync(&setting);
    if (result != 0)
    {
        SetLastNetworkError(result);
        SetState(eNQRNetworkManagerInternalState_JoinSessionFailed);
        if (s_lastNetworkErrorCode == cErrorCode118_0543 || s_lastNetworkErrorCode == cErrorCode118_0544)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_CannotJoinGame);
        else if (s_lastNetworkErrorCode == cErrorCode118_0541)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        return false;
    }
    SetState(eNQRNetworkManagerInternalState_JoinJoiningSession);
    SetNexAsyncProcess(eNEXAsyncProcess_JoinSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_JoinMatchmakeSession()
{
    nn::pia::inet::NexJoinSessionSetting setting;
    setting.SetSessionId(m_pendingSessionId);
    nn::pia::Result result = nn::pia::session::g_piaSession->JoinSessionAsync(&setting);
    if (result != 0)
    {
        SetLastNetworkError(result);
        SetState(eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed);
        if (s_lastNetworkErrorCode == cErrorCode118_0543 || s_lastNetworkErrorCode == cErrorCode118_0544)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_CannotJoinGame);
        else if (s_lastNetworkErrorCode == cErrorCode118_0541)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        return false;
    }
    SetState(eNQRNetworkManagerInternalState_JoinJoiningMatchmakeSession);
    SetNexAsyncProcess(eNEXAsyncProcess_JoinSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_LeaveSession()
{
    nn::pia::Result result = nn::pia::session::g_piaSession->LeaveSessionAsync();
    if (result != 0)
    {
        if (result == nn::pia::session::Session::cResultStatusDisconnectedNetwork)
        {
            SetState(eNQRNetworkManagerInternalState_Ending);
            return true;
        }
        SetState(eNQRNetworkManagerInternalState_LeaveRoomFailed);
        return true;
    }
    SetNexAsyncProcess(eNEXAsyncProcess_LeaveSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_BrowseSession()
{
    nn::pia::Result result = nn::pia::session::g_piaSession->BrowseSessionAsync(&m_searchCriteria);
    if (result != 0)
        return false;
    SetNexAsyncProcess(eNEXAsyncProcess_BrowseSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_JoinRandomSession()
{
    nn::pia::inet::NexCreateSessionSetting createSetting;
    createSetting.SetGameMode(m_gameMode);
    createSetting.SetMaxParticipantNum(1);
    createSetting.SetOpenSession(false);
    m_searchCriteria.SetRandomSessionSelectionMethod(nn::pia::inet::cSelectionMethod_0);
    nn::pia::inet::NexJoinRandomSessionSetting joinSetting;
    joinSetting.Reset();
    joinSetting.SetCreateSessionSetting(&createSetting);
    joinSetting.SetSessionSearchCriteria(&m_searchCriteria, 1);
    nn::pia::Result result = nn::pia::session::g_piaSession->JoinRandomSessionAsync(&joinSetting);
    if (result != 0)
        return false;
    SetNexAsyncProcess(eNEXAsyncProcess_JoinRandomSession);
    return true;
}

void NQRNetworkManager_WiiU::UpdateExternalRoomDataFromThread()
{
    if (!m_localGame && !s_piaShutdownPending && m_host)
    {
        long long now = System::processTimeInMilliSecs();
        long long elapsed = now - m_lastSessionUpdateTime;
        if (m_nexAsyncProcess == eNEXAsyncProcess_None && elapsed > cSessionUpdateInterval)
        {
            if (nn::pia::session::g_piaSession != NULL)
            {
                nn::pia::inet::NexUpdateSessionSetting setting;
                setting.SetApplicationData(m_externalData, m_externalDataSize);
                nn::pia::Result result = nn::pia::session::g_piaSession->UpdateSessionSettingAsync(&setting);
                if (result == 0)
                {
                    SetNexAsyncProcess(eNEXAsyncProcess_Update);
                    m_updateRequested = false;
                }
                m_lastSessionUpdateTime = now;
            }
        }
        else
            m_updateRequested = true;
    }
}

void NQRNetworkManager_WiiU::UpdateMatchmakingAttributes()
{
    if (m_nexAsyncProcess != eNEXAsyncProcess_None)
        return;
    for (int index = 0; index < m_roomAttributeCount; ++index)
    {
        if (m_roomAttributes[index] != m_matchmakeAttributes[index])
        {
            nn::pia::inet::NexUpdateSessionSetting setting;
            setting.SetAttribute(index, m_roomAttributes[index]);
            nn::pia::Result result = nn::pia::session::g_piaSession->UpdateSessionSettingAsync(&setting);
            if (result == 0)
            {
                SetNexAsyncProcess(eNEXAsyncProcess_UpdateSession);
                m_pendingAttributeIndex = index;
                m_pendingAttributeValue = m_roomAttributes[index];
            }
            return;
        }
    }
}

bool NQRNetworkManager_WiiU::Pia_StartupSession()
{
    char tokenData[32] = "Friend";
    nn::pia::transport::Station::IdentificationToken token;
    std::memcpy(token.m_token, tokenData, sizeof(token.m_token));
    char* name = gProfile->GetNetworkAccountID();
    nn::pia::Result result = NQRNetworkManager_WiiU_Pia_StartupSession(&token, name);
    if (result != 0)
    {
        SetLastNetworkError(result);
        return false;
    }
    nn::pia::session::g_piaSession->SetSyncClockPulseInterval(1000);
    s_piaSessionStarted = true;
    return true;
}

void NQRNetworkManager_WiiU::Pia_SignalCreateSession()
{
    m_createRequested = true;
}

void NQRNetworkManager_WiiU::Pia_SignalJoinSession()
{
    m_joinRequested = true;
}

void NQRNetworkManager_WiiU::SetLocalPlayersAndSync()
{
    for (int index = 0; index < m_localPlayerCount; ++index)
        m_roomPlayers[index].m_uiStationId = m_localStationId;
    MapRoomSlotPlayers(-1);
    SyncRoomData();
}

void NQRNetworkManager_WiiU::ClearIsHandlingBootInvite()
{
    g_bHandlingGameBootInvite = false;
    gApp.setIgnoreMainMenuInputDuringInvite(false);
}

void NQRNetworkManager_WiiU::SetNQRPresenceInfoFromExtData(PresenceSyncInfo* presenceInfo, void* extData)
{
    GameSessionData* sessionData = static_cast<GameSessionData*>(extData);
    presenceInfo->m_ucGameMode = sessionData->m_subTexturePackId;
    presenceInfo->m_uiWorldId = sessionData->m_texturePackParentId;
    presenceInfo->m_usTitleVersion = sessionData->m_version;
    presenceInfo->m_bJoinable = !sessionData->m_bPublicGame;
    presenceInfo->m_ucPlayerCount = sessionData->m_gameModeId;
    presenceInfo->m_ucMaxPlayers = sessionData->m_miniGameId;
    presenceInfo->m_uiHostAttributes = sessionData->m_customGameModeHostOptions;
}

void NQRNetworkManager_WiiU::UpdateRichPresenceCustomData(void* extData, unsigned int extDataSize)
{
    nn::pia::session::Session* session = nn::pia::session::g_piaSession;
    nn::pia::transport::StationManager* stationManager = nn::pia::transport::StationManager::s_pInstance;
    if (session != NULL && stationManager != NULL)
    {
        m_presenceGameMode.joinAvailabilityFlag = TRUE;
        m_presenceGameMode.matchmakeSystemType = cPresenceMatchmakeSystemType;
        m_presenceGameMode.joinGameId = s_nexGameServerId;
        m_presenceGameMode.joinGroupId = session->m_sessionId[session->m_currentSessionIndex];
        m_presenceGameMode.joinGameMode = cPresenceJoinGameMode;
        unsigned long long principalId;
        stationManager->GetStation(session->m_hostStationId)->GetPrincipalId(&principalId);
        m_presenceGameMode.ownerPrincipalId = static_cast<unsigned int>(principalId);
        std::memcpy(m_presenceGameMode.applicationArg, extData, extDataSize);
        gProfile->SetRichPresenceGameModeData(m_presenceGameMode);
    }
}

void NQRNetworkManager_WiiU::MatchmakingExtDataUpdatedCallback(bool success, void* context)
{
    if (success)
    {
        NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
        manager->SetNQRPresenceInfoFromExtData(&s_presenceInfo, manager->m_matchmakingExtData);
        std::free(manager->m_matchmakingExtData);
        manager->m_matchmakingExtData = NULL;
        manager->UpdateRichPresenceCustomData(&s_presenceInfo, sizeof(s_presenceInfo));
    }
}

void NQRNetworkManager_WiiU::RemoveAllPlayersOnClient()
{
    EnterCriticalSection(&m_connectedPlayerMutex);
    EnterCriticalSection(&m_connectedStationMutex);
    std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin();
    while (it != m_players.end())
    {
        NQRNetworkPlayer* player = *it;
        if (m_listener != NULL)
            m_listener->HandlePlayerLeaving(player);
        for (int index = 0; index < 8; ++index)
            if (m_roomSlotPlayers[index] == player)
                m_roomSlotPlayers[index] = NULL;
        RemoveConnectedPlayer(player);
        delete player;
    }
    while (m_connectedStations.size() != 0)
        RemoveConnectedStation(m_connectedStations[0].stationId);
    LeaveCriticalSection(&m_connectedStationMutex);
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

void NQRNetworkManager_WiiU::ResetToIdle()
{
    if (!m_host)
        RemoveAllPlayersOnClient();
    m_unknown11E8 = false;
    m_host = false;
    m_sessionJoined = false;
    m_nextSmallId = 0;
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomSlotPlayers[index] != NULL)
        {
            RemoveConnectedPlayer(m_roomSlotPlayers[index]);
            delete m_roomSlotPlayers[index];
        }
    }
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    std::memset(m_roomPlayers, 0, sizeof(m_roomPlayers));
    LeaveCriticalSection(&m_playerMutex);
    SetState(eNQRNetworkManagerInternalState_Idle);
    ClearSendQueue();
}

void NQRNetworkManager_WiiU::TickNexAsyncProcess()
{
    if (nn::nex::Scheduler::GetInstance() == NULL)
        return;
    nn::nex::Scheduler::GetInstance()->Dispatch(s_nexDispatchTimeout, 0);
    nn::pia::common::g_piaJobScheduler->Dispatch(0);
    if (m_nexAsyncProcess == eNEXAsyncProcess_None)
    {
        if (m_createRequested)
        {
            Pia_CreateSession();
            m_createRequested = false;
            return;
        }
        if (m_joinRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            Pia_JoinSession();
            m_joinRequested = false;
            return;
        }
        if (m_joinMatchmakeRequested)
        {
            Pia_JoinMatchmakeSession();
            m_joinMatchmakeRequested = false;
            return;
        }
        if (m_leaveRequested)
        {
            Pia_LeaveSession();
            m_leaveRequested = false;
            return;
        }
        if (m_browseRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            Pia_BrowseSession();
            m_browseRequested = false;
            return;
        }
        if (m_randomJoinRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            Pia_JoinRandomSession();
            m_randomJoinRequested = false;
            return;
        }
        if (m_updateRequested)
            UpdateExternalRoomDataFromThread();
        else
            UpdateMatchmakingAttributes();
        return;
    }
    if (m_nexCallContext != NULL && m_nexCallContext->GetState() == nn::nex::CallContext::CallInProgress)
        return;
    switch (m_nexAsyncProcess)
    {
    case eNEXAsyncProcess_Login:
    {
        if (m_nexCallContext->GetState() == nn::nex::CallContext::CallInProgress)
            return;
        SetNexAsyncProcess(eNEXAsyncProcess_None);
        nn::nex::NgsFacade* ngsFacade = Nex_GetNgsFacadeInstance();
        nn::nex::qResult loginResult(m_nexCallContext->GetResult());
        bool loggedIn = false;
        if (!static_cast<bool>(loginResult))
        {
            m_nexLoginState = eNEXLoginState_Failed;
            if (m_internalState == eNQRNetworkManagerInternalState_HostWaitingForLogin)
                SetState(eNQRNetworkManagerInternalState_HostCreateSessionFailed);
            else
                SetState(m_internalState == eNQRNetworkManagerInternalState_JoinWaitingForLogin
                             ? eNQRNetworkManagerInternalState_JoinSessionFailed
                             : eNQRNetworkManagerInternalState_InitialiseFailed);
            nn::nex::qResult failure(m_nexCallContext->GetResult());
            nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(failure);
            s_lastNetworkErrorCode = ngsFacade->GetLastLoginErrorCode();
            if (s_lastNetworkErrorCode == s_loginErrorServiceUnavailable)
            {
                s_applicationConnected = false;
                s_connectFailed = true;
            }
        }
        else
        {
            nn::nex::qResult connected = ngsFacade->IsConnected();
            if (!static_cast<bool>(connected))
            {
                s_lastNetworkErrorCode = ngsFacade->GetLastLoginErrorCode();
            }
            else
            {
                bool nexFacadeBound = true;
                if (nn::pia::inet::NexFacade::GetInstance() != NULL)
                {
                    nn::pia::inet::NexFacade::LoginInfo loginInfo = {ngsFacade, s_nexGameServerId, NULL};
                    nn::pia::common::String accessKey(s_nexAccessKey);
                    loginInfo.accessKey = &accessKey;
                    if (nn::pia::inet::NexFacade::GetInstance()->Bind(&loginInfo) != 0)
                        nexFacadeBound = false;
                }
                if (nexFacadeBound && Nex_InitializeMatchmake() && Pia_StartupSession())
                    loggedIn = true;
            }
        }
        if (!loggedIn)
        {
            m_nexLoginState = eNEXLoginState_Failed;
            Pia_Shutdown();
            if (m_internalState == eNQRNetworkManagerInternalState_HostWaitingForLogin)
            {
                SetState(eNQRNetworkManagerInternalState_HostCreateSessionFailed);
                return;
            }
            if (m_internalState == eNQRNetworkManagerInternalState_JoinWaitingForLogin)
            {
                SetState(eNQRNetworkManagerInternalState_JoinSessionFailed);
                return;
            }
            if (!IsInSession())
                SetState(eNQRNetworkManagerInternalState_InitialiseFailed);
            break;
        }
        m_nexLoginState = eNEXLoginState_LoggedIn;
        if (m_internalState == eNQRNetworkManagerInternalState_LoggingIn)
            SetState(eNQRNetworkManagerInternalState_Idle);
        else if (m_internalState == eNQRNetworkManagerInternalState_HostWaitingForLogin)
            Pia_SignalCreateSession();
        else if (m_internalState == eNQRNetworkManagerInternalState_JoinWaitingForLogin)
            Pia_SignalJoinSession();
        break;
    }
    case eNEXAsyncProcess_Shutdown:
        if (m_nexCallContext->GetState() != nn::nex::CallContext::CallInProgress)
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            m_nexLoginState = eNEXLoginState_Failed;
            nn::nex::qResult shutdownResult(m_nexCallContext->GetResult());
            if (!static_cast<bool>(shutdownResult))
            {
                nn::nex::qResult failure(m_nexCallContext->GetResult());
                nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(failure);
            }
            Pia_Finalize();
            return;
        }
        break;
    case eNEXAsyncProcess_Search:
        if (m_nexCallContext->GetState() != nn::nex::CallContext::CallInProgress)
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            if (m_nexCallContext->GetState() == nn::nex::CallContext::CallSucceeded)
            {
                if (m_gatheringHolder->GetObject() != NULL)
                {
                    const nn::nex::MatchmakeSession* session = static_cast<const nn::nex::MatchmakeSession*>(
                        m_gatheringHolder->GetObject());
                    std::vector<unsigned char, nn::nex::MemAllocator<unsigned char> > applicationBuffer;
                    applicationBuffer.assign(session->m_ApplicationBuffer.begin(), session->m_ApplicationBuffer.end());
                    std::memcpy(m_extDataBuffer, &applicationBuffer[0], applicationBuffer.size());
                    m_extDataCallback(true, m_extDataCallbackData);
                }
                delete m_gatheringHolder;
            }
            else
            {
                m_extDataCallback(false, m_extDataCallbackData);
                delete m_gatheringHolder;
            }
            m_gatheringHolder = NULL;
            return;
        }
        break;
    case eNEXAsyncProcess_Update:
        if (nn::pia::session::g_piaSession->IsUpdateSessionSettingCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::session::g_piaSession->GetUpdateSessionSettingResult();
            return;
        }
        break;
    case eNEXAsyncProcess_CreateSession:
        if (nn::pia::session::g_piaSession->IsCreateSessionCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetCreateSessionResult();
            if (result != 0)
            {
                SetLastNetworkError(result);
                SetState(eNQRNetworkManagerInternalState_HostCreateSessionFailed);
                Pia_Shutdown();
                return;
            }
            m_unknown13A0 = true;
            SetState(eNQRNetworkManagerInternalState_HostSessionCreated);
            SetState(eNQRNetworkManagerInternalState_HostRoomReady);
            m_localStationId = nn::pia::session::g_piaSession->m_localStationId;
            SetLocalPlayersAndSync();
            return;
        }
        break;
    case eNEXAsyncProcess_JoinSession:
    {
        if (!nn::pia::session::g_piaSession->IsJoinSessionCompleted())
            return;
        SetNexAsyncProcess(eNEXAsyncProcess_None);
        nn::pia::Result result = nn::pia::session::g_piaSession->GetJoinSessionResult();
        if (result == 0)
        {
            m_unknown13A0 = true;
            SetState(eNQRNetworkManagerInternalState_JoinRoomReady);
            ClearIsHandlingBootInvite();
            return;
        }
        SetLastNetworkError(result);
        SetState(GetState() == eNQRNetworkManagerInternalState_JoinJoiningMatchmakeSession
                     ? eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed
                     : eNQRNetworkManagerInternalState_JoinSessionFailed);
        if (result == cPiaResultJoinSessionFull)
        {
            Minecraft* minecraft = Minecraft::GetInstance();
            int primaryPad = gProfile->GetPrimaryPad();
            minecraft->mConnectionFailed[primaryPad] = true;
            minecraft->mDisconnectReasons[primaryPad] = DisconnectPacket::eDisconnectReason_GameFull;
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_GameFull);
        }
        else if (result == cPiaResultJoinSessionRefused)
        {
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
            Minecraft* minecraft = Minecraft::GetInstance();
            int primaryPad = gProfile->GetPrimaryPad();
            minecraft->mConnectionFailed[primaryPad] = true;
            minecraft->mDisconnectReasons[primaryPad] = DisconnectPacket::eDisconnectReason_NoUserContentPrivilege;
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
        }
        Session_CleanupSession();
        if (!Pia_StartupSession())
            Pia_Shutdown();
        if (s_lastNetworkErrorCode == cErrorCode118_0543 || s_lastNetworkErrorCode == cErrorCode118_0544)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_CannotJoinGame);
        else if (s_lastNetworkErrorCode == cErrorCode118_0541)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        break;
    }
    case eNEXAsyncProcess_JoinRandomSession:
    {
        if (!nn::pia::session::g_piaSession->IsJoinRandomSessionCompleted())
            return;
        SetNexAsyncProcess(eNEXAsyncProcess_None);
        nn::pia::Result result = nn::pia::session::g_piaSession->GetJoinRandomSessionResult();
        if (result == 0)
        {
            if (nn::pia::session::g_piaSession->IsHost())
            {
                SetState(eNQRNetworkManagerInternalState_LeavingRoom);
                Pia_SignalLeaveSession();
                m_nextIdleReasonIsFailure = true;
                return;
            }
            m_unknown13A0 = true;
            SetState(eNQRNetworkManagerInternalState_JoinRoomReady);
            ClearIsHandlingBootInvite();
            unsigned int sessionId =
                nn::pia::session::g_piaSession->m_sessionId[nn::pia::session::g_piaSession->m_currentSessionIndex];
            if (m_matchmakingExtData == NULL)
                m_matchmakingExtData = std::malloc(sizeof(GameSessionData));
            GetExtDataForRoom(sessionId, m_matchmakingExtData, MatchmakingExtDataUpdatedCallback, this);
            return;
        }
        SetLastNetworkError(result);
        SetState(GetState() == eNQRNetworkManagerInternalState_JoinJoiningMatchmakeSession
                     ? eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed
                     : eNQRNetworkManagerInternalState_JoinSessionFailed);
        if (result == cPiaResultJoinSessionRefused)
        {
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
            Minecraft* minecraft = Minecraft::GetInstance();
            int primaryPad = gProfile->GetPrimaryPad();
            minecraft->mConnectionFailed[primaryPad] = true;
            minecraft->mDisconnectReasons[primaryPad] = DisconnectPacket::eDisconnectReason_NoUserContentPrivilege;
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
        }
        Session_CleanupSession();
        if (!Pia_StartupSession())
            Pia_Shutdown();
        if (s_lastNetworkErrorCode == cErrorCode118_0543 || s_lastNetworkErrorCode == cErrorCode118_0544)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_CannotJoinGame);
        else if (s_lastNetworkErrorCode == cErrorCode118_0541)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        break;
    }
    case eNEXAsyncProcess_LeaveSession:
        if (nn::pia::session::g_piaSession->IsLeaveSessionCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            if (m_internalState == eNQRNetworkManagerInternalState_LeavingRoom)
                SetState(eNQRNetworkManagerInternalState_Ending);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetLeaveSessionResult();
            if (result != 0)
                SetLastNetworkError(result);
            Session_CleanupSession();
            if (!Pia_StartupSession())
                Pia_Shutdown();
            ResetToIdle();
            return;
        }
        break;
    case eNEXAsyncProcess_BrowseSession:
        if (nn::pia::session::g_piaSession->IsBrowseSessionCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetBrowseSessionResult();
            if (result == 0)
            {
                nn::pia::session::ISessionInfoList* sessionList =
                    nn::pia::session::g_piaSession->GetBrowsedSessionInfoList();
                nn::pia::session::ISessionInfo** sessionInfo = sessionList->Begin();
                std::vector<int> sessionIds;
                if (sessionInfo != sessionList->End())
                {
                    do
                    {
                        unsigned int gameMode = (*sessionInfo)->GetGameMode();
                        sessionIds.push_back((*sessionInfo)->GetSessionId());
                        ++sessionInfo;
                    } while (sessionInfo != sessionList->End());
                    m_pendingSessionId = sessionIds[System::processTimeInMilliSecs() % sessionIds.size()];
                    Pia_JoinMatchmakeSession();
                }
                else
                    SetState(eNQRNetworkManagerInternalState_JoinNoMatchmakeSessionFound);
                return;
            }
        }
        break;
    case eNEXAsyncProcess_UpdateSession:
        if (nn::pia::session::g_piaSession->IsUpdateSessionSettingCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetUpdateSessionSettingResult();
            if (result == 0)
                m_matchmakeAttributes[m_pendingAttributeIndex] = m_pendingAttributeValue;
        }
        break;
    default:
        break;
    }
}

void NQRNetworkManager_WiiU::SendJoinData()
{
    if (m_localGame)
        return;
    ClientJoinData joinData;
    joinData.playerMask = m_pendingUserMask & ~s_connectedLocalPlayerMask;
    for (int index = 0; index < 4; ++index)
    {
        if ((joinData.playerMask & (1 << index)) != 0)
        {
            gProfile->GetXUID(index, &joinData.playerUIDs[index], true);
            std::wstring miiName(gProfile->GetMiiName(index));
            std::wcscpy(joinData.displayNames[index], miiName.c_str());
            std::wstring displayName(gProfile->GetDisplayName(index));
            std::wcstombs(joinData.onlineIds[index], displayName.c_str(), sizeof(joinData.onlineIds[index]));
        }
    }
    RemoteDataSend(m_hostStationId, cJoinChannel, &joinData, sizeof(joinData));
}

void NQRNetworkManager_WiiU::NetworkPlayerConnectionComplete(NQRNetworkPlayer* player)
{
    EnterCriticalSection(&m_playerConnectionMutex);
    bool wasReady = player->IsReady();
    player->ConnectionComplete();
    bool isReady = player->IsReady();
    LeaveCriticalSection(&m_playerConnectionMutex);
    if (!wasReady && isReady)
        HandlePlayerJoined(player);
}

void NQRNetworkManager_WiiU::PollUnconnectedStations()
{
    EnterCriticalSection(&m_connectedPlayerMutex);
    EnterCriticalSection(&m_connectedStationMutex);
    if (m_sessionJoined)
    {
        for (unsigned int index = 0; index < m_connectedStations.size(); ++index)
        {
            if ((m_connectedStations[index].state & 1) == 0)
            {
                nn::pia::transport::ReliableProtocol* protocol = nn::pia::transport::g_piaTransport->m_protocolManager
                                                                     .GetProtocol<nn::pia::transport::ReliableProtocol>(
                                                                         m_reliableProtocolHandle);
                if (m_host)
                {
                    if (protocol->IsInCommunication(m_connectedStations[index].stationId))
                        m_connectedStations[index].state |= 1;
                }
                else if (protocol->IsInCommunication(m_hostStationId))
                {
                    m_connectedStations[index].state |= 1;
                    SendJoinData();
                }
            }
        }

        for (unsigned int index = 0; index < m_players.size(); ++index)
        {
            if (!m_players[index]->IsConnectionComplete())
            {
                if (m_host)
                {
                    ConnectedStation* station = GetConnectedStation(m_players[index]->GetStationId());
                    if (station != NULL && (station->state & 1) != 0)
                        NetworkPlayerConnectionComplete(m_players[index]);
                }
                else
                {
                    ConnectedStation* station = GetConnectedStation(m_hostStationId);
                    if (station != NULL && (station->state & 1) != 0)
                        NetworkPlayerConnectionComplete(m_players[index]);
                }
            }
        }
    }
    LeaveCriticalSection(&m_connectedStationMutex);
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

bool NQRNetworkManager_WiiU::AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData,
                                                     EJoinFailedReason* reason)
{
    EnterCriticalSection(&m_playerMutex);

    for (int index = 0; index < 4; ++index)
    {
        if ((joinData.playerMask & (1 << index)) != 0)
        {
            for (int existing = 0; existing < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++existing)
            {
                if (m_roomPlayers[existing] == joinData.playerUIDs[existing]
                    && (m_roomPlayers[existing].flags & PlayerUID::FlagMiiPrincipal) == 0)
                {
                    if (reason != NULL)
                        *reason = eJoinFailedReason_MiiInUse;
                    LeaveCriticalSection(&m_playerMutex);
                    return false;
                }
            }
        }
    }

    int incoming = 0;
    for (int index = 0; index < 4; ++index)
    {
        if ((joinData.playerMask & (1 << index)) != 0)
            ++incoming;
    }

    if (m_roomPlayers[0].m_uiPlayerCountMinusOne + incoming + 1
        > static_cast<int>(CGameNetworkManager::sInstance.GetMaxPlayers()))
    {
        if (reason != NULL)
            *reason = eJoinFailedReason_GameFull;
        LeaveCriticalSection(&m_playerMutex);
        return false;
    }

    int stationIndex = -1;
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomPlayers[index].m_uiStationId == stationId)
        {
            stationIndex = index;
            break;
        }
    }

    int insertIndex = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    if (stationIndex >= 0)
    {
        for (int index = stationIndex; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        {
            if (m_roomPlayers[index].m_uiStationId != stationId)
            {
                insertIndex = index;
                break;
            }
        }
    }

    for (int index = 0; index < 4; ++index)
    {
        if ((joinData.playerMask & (1 << index)) != 0)
        {
            for (int slot = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; slot > insertIndex; --slot)
                m_roomPlayers[slot] = m_roomPlayers[slot - 1];

            PlayerSyncData& record = m_roomPlayers[insertIndex];
            static_cast<PlayerUID&>(record) = joinData.playerUIDs[index];
            record.m_uiStationId = stationId;
            record.m_uiLocalPlayerIndex = index;
            record.m_uiSmallId = m_nextSmallId++;
            std::wcscpy(record.m_szMiiName, joinData.displayNames[index]);
            std::strncpy(record.m_szNetworkName, joinData.onlineIds[index], sizeof(joinData.onlineIds[index]));
            ++m_roomPlayers[0].m_uiPlayerCountMinusOne;
            ++insertIndex;

            NetworkPlayerConnectionComplete(m_players[index]);
            NQRNetworkPlayer* player = new NQRNetworkPlayer(this, NQRNetworkPlayer::eNQRNetworkPlayerType_Remote, true,
                                                            stationId, index, &joinData.playerUIDs[index],
                                                            record.m_szNetworkName, record.m_szMiiName);
            AddConnectedPlayer(player);
        }
    }

    MapRoomSlotPlayers(-1);
    SyncRoomData();
    LeaveCriticalSection(&m_playerMutex);
    return true;
}

void NQRNetworkManager_WiiU::SendJoinFailedData(unsigned long long stationId, unsigned int playerMask,
                                                EJoinFailedReason reason)
{
    if (m_localGame)
        return;
    unsigned int packet[2];
    packet[0] = playerMask;
    packet[1] = static_cast<unsigned int>(reason);
    RemoteDataSend(stationId, cJoinFailedChannel, packet, sizeof(packet));
}

void NQRNetworkManager_WiiU::PollReceiveData()
{
    if (nn::pia::transport::g_piaTransport != NULL)
    {
        nn::pia::transport::ReliableProtocol* protocol =
            nn::pia::transport::g_piaTransport->m_protocolManager.GetProtocol<nn::pia::transport::ReliableProtocol>(
                m_reliableProtocolHandle);
        if (m_sessionJoined)
        {
            unsigned char* packet = static_cast<unsigned char*>(std::malloc(cReceiveBufferSize));
            unsigned long long stationId;
            unsigned int size;
            while (protocol->Receive(&stationId, packet, &size, cReceiveBufferSize) == 0)
            {
                const unsigned char channel = packet[0];
                switch (channel)
                {
                case cRoomSyncChannel:
                    std::memcpy(m_roomPlayers, packet + 1, sizeof(m_roomPlayers));
                    MapRoomSlotPlayers(-1);
                    break;
                case cJoinChannel:
                {
                    ClientJoinData joinData;
                    std::memcpy(&joinData, packet + 1, sizeof(joinData));
                    EJoinFailedReason reason;
                    if (!AddRemotePlayersAndSync(stationId, joinData, &reason))
                        SendJoinFailedData(stationId, joinData.playerMask, reason);
                    break;
                }
                case cJoinFailedChannel:
                {
                    unsigned int failedData[2];
                    std::memcpy(failedData, packet + 1, sizeof(failedData));
                    if ((failedData[0] & 1) != 0)
                    {
                        if (failedData[1] == eJoinFailedReason_GameFull)
                            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_GameFull);
                        else if (failedData[1] == eJoinFailedReason_MiiInUse)
                            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_MiiInUse);
                        LeaveRoom(true);
                    }
                    else
                    {
                        if ((failedData[0] & 2) != 0)
                        {
                            gApp.SetAction(1, eXuiAction_RemoveLocalPlayer, NULL);
                            RemoveLocalPlayerByUserIndex(1);
                        }
                        if ((failedData[0] & 4) != 0)
                        {
                            gApp.SetAction(2, eXuiAction_RemoveLocalPlayer, NULL);
                            RemoveLocalPlayerByUserIndex(2);
                        }
                        if ((failedData[0] & 8) != 0)
                        {
                            gApp.SetAction(3, eXuiAction_RemoveLocalPlayer, NULL);
                            RemoveLocalPlayerByUserIndex(3);
                        }
                        int parameter = static_cast<int>(IDS_CONFIRM_OK);
                        if (failedData[1] == eJoinFailedReason_GameFull)
                            gConsoleUIController.RequestErrorMessage(cGameFullTitleId, cGameFullTextId, &parameter, 1,
                                                                     0xff, NULL, NULL, NULL, true);
                        else if (failedData[1] == eJoinFailedReason_MiiInUse)
                            gConsoleUIController.RequestErrorMessage(cMiiInUseTitleId, cMiiInUseTextId, &parameter, 1,
                                                                     0xff, NULL, NULL, NULL, true);
                        else
                            gConsoleUIController.RequestErrorMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK,
                                                                     &parameter, 1, 0xff, NULL, NULL, NULL, true);
                    }
                    break;
                }
                case cLeaveChannel:
                {
                    unsigned int playerMask;
                    std::memcpy(&playerMask, packet + 1, sizeof(playerMask));
                    RemoveRemotePlayersAndSync(stationId, static_cast<int>(playerMask));
                    if (stationId == m_hostStationId && (playerMask & 1) != 0)
                        SetState(eNQRNetworkManagerInternalState_Ending);
                    break;
                }
                case 0:
                case 1:
                case 2:
                case 3:
                    if (m_host)
                    {
                        EnterCriticalSection(&m_connectedPlayerMutex);
                        NQRNetworkPlayer* hostPlayer = m_roomSlotPlayers[0];
                        NQRNetworkPlayer* player = GetPlayerFromStationIdAndLocalIdx(stationId, channel);
                        if (hostPlayer != NULL && player != NULL && size >= 2)
                        {
                            if (size > 2)
                            {
                                hostPlayer->ReadAckRequest(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]), player);
                                m_listener->HandleDataReceived(player, hostPlayer, packet + 2, size - 2);
                            }
                            else
                                player->ReadAckReturn(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]));
                        }
                        LeaveCriticalSection(&m_connectedPlayerMutex);
                    }
                    else
                    {
                        EnterCriticalSection(&m_connectedPlayerMutex);
                        NQRNetworkPlayer* hostPlayer = m_roomSlotPlayers[0];
                        NQRNetworkPlayer* player = GetPlayerFromStationIdAndLocalIdx(m_localStationId, channel);
                        if (hostPlayer != NULL && player != NULL && size >= 2)
                        {
                            if (size > 2)
                            {
                                player->ReadAckRequest(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]), hostPlayer);
                                m_listener->HandleDataReceived(hostPlayer, player, packet + 2, size - 2);
                            }
                            else
                                player->ReadAckReturn(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]));
                        }
                        LeaveCriticalSection(&m_connectedPlayerMutex);
                    }
                    break;
                }
            }
            std::free(packet);
        }
    }
}

void NQRNetworkManager_WiiU::ShowErrorViewer()
{
    if (s_lastNetworkErrorCode != 0)
    {
        g_pInput->RequestErrorViewer(0, s_lastNetworkErrorCode, NULL, NULL);
        if (s_lastNetworkErrorCode == cErrorCode118_0008)
            s_piaShutdownPending = true;
        s_lastNetworkErrorCode = 0;
    }
}

void NQRNetworkManager_WiiU::RemoveLocalClientPlayers(int playerMask)
{
    EnterCriticalSection(&m_connectedPlayerMutex);
    std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin();
    while (it != m_players.end())
    {
        NQRNetworkPlayer* player = *it;
        if (player->GetStationId() == m_localStationId && ((1 << player->GetLocalIndex()) & playerMask) != 0)
        {
            if (m_listener != NULL)
                m_listener->HandlePlayerLeaving(player);
            for (int index = 0; index < 8; ++index)
                if (m_roomSlotPlayers[index] == player)
                    m_roomSlotPlayers[index] = NULL;
            RemoveConnectedPlayer(player);
            delete player;
        }
        else
            ++it;
    }
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

void NQRNetworkManager_WiiU::ErrorHandlingTick()
{
    switch (m_internalState)
    {
    case eNQRNetworkManagerInternalState_InitialiseFailed:
        ShowErrorViewer();
        if (m_receiving && m_localGame)
        {
            SetState(eNQRNetworkManagerInternalState_Playing);
            break;
        }
        m_unknownDCD = true;
        SetState(eNQRNetworkManagerInternalState_Idle);
        break;
    case eNQRNetworkManagerInternalState_HostCreateSessionFailed:
        ShowErrorViewer();
        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    case eNQRNetworkManagerInternalState_JoinNoMatchmakeSessionFound:
        m_nextIdleReasonIsFailure = true;
        SetState(eNQRNetworkManagerInternalState_Idle);
        break;
    case eNQRNetworkManagerInternalState_JoinSessionFailed:
    case eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed:
        if (s_lastNetworkErrorCode == cErrorCode118_0543 || s_lastNetworkErrorCode == cErrorCode118_0544)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_CannotJoinGame);
        else if (s_lastNetworkErrorCode == cErrorCode118_0541)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        else
            ShowErrorViewer();
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    case eNQRNetworkManagerInternalState_LeaveRoomFailed:
        if (!m_host)
            RemoveLocalClientPlayers(cAllLocalPlayersMask);
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    default:
        break;
    }
}

void NQRNetworkManager_WiiU::TryToConnectOnline()
{
    EnterCriticalSection(&s_connectMutex);
    if (s_piaShutdownPending)
    {
        s_connectRetryPending = true;
        LeaveCriticalSection(&s_connectMutex);
        return;
    }
    s_connectRetryPending = false;
    CGameNetworkManager::sInstance.ResetLeavingGame();
    if (!s_applicationConnected && !s_acConnectPending)
    {
        nn::Result result = nn::ac::ConnectAsync();
        if (result.IsSuccess())
            s_acConnectPending = true;
        else
            s_lastLoginErrorCode = 0;
    }
    if (!s_applicationConnected)
    {
        if (!s_acConnectPending)
        {
            s_connectCompleted = true;
            s_connectFailed = true;
        }
    }
    else if (!s_friendPresenceLoginPending)
    {
        s_connectionRecheckPending = true;
    }
    LeaveCriticalSection(&s_connectMutex);
}

void NQRNetworkManager_WiiU::TickFromThread()
{
    TickSendQueue();
    OnlineCheck();
    FriendSearchTick();
    UpdateApplicationConnected();
    TickNexAsyncProcess();
    PollUnconnectedStations();
    PollReceiveData();
    ErrorHandlingTick();
    NintendoVoiceChat_WiiU::Tick();
    if (s_extDataRequested)
    {
        s_extDataRequested = false;
        bool searchStarted = false;
        if (m_nexLoginState == eNEXLoginState_LoggedIn)
        {
            nn::nex::MatchmakeExtensionClient* client = static_cast<nn::nex::MatchmakeExtensionClient*>(
                Nex_GetMatchmakeClient());
            m_gatheringHolder = new nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String>();
            if (client->FindByID(m_nexCallContext, s_extDataSessionId, m_gatheringHolder))
            {
                SetNexAsyncProcess(eNEXAsyncProcess_Search);
                searchStarted = true;
            }
        }
        if (!searchStarted)
        {
            m_extDataCallback(false, m_extDataCallbackData);
            return;
        }
    }
    if (s_connectNetworkErrorPending)
    {
        s_connectNetworkErrorPending = false;
        int parameter = static_cast<int>(IDS_CONFIRM_OK);
        gConsoleUIController.RequestErrorMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, &parameter, 1, 0xff, NULL,
                                                 NULL, NULL, true);
    }
    if (s_piaShutdownPending)
        Pia_Shutdown();
    if (s_connectRetryPending)
        TryToConnectOnline();
    if (m_unknownDD0 != 0)
    {
        if (m_internalState == eNQRNetworkManagerInternalState_Playing)
        {
            if (--m_unknownDD0 == 0)
                UpdateExternalRoomData();
        }
        else
            m_unknownDD0 = 0;
    }
    if (m_internalState == eNQRNetworkManagerInternalState_JoinRoomReady && m_localPlayersJoined == m_localPlayerCount)
    {
        UpdateRichPresenceCustomData(&s_presenceInfo, sizeof(s_presenceInfo));
        SetState(eNQRNetworkManagerInternalState_StartingGame);
        SetState(eNQRNetworkManagerInternalState_Playing);
    }
    if (m_internalState == eNQRNetworkManagerInternalState_Ending)
    {
        EnterCriticalSection(&m_connectedStationMutex);
        if (m_connectedStations.size() == 0)
            ResetToIdle();
        LeaveCriticalSection(&m_connectedStationMutex);
    }
    EnterCriticalSection(&m_stateChangeLock);
    while (m_stateChanges.size() > 0)
    {
        if (m_listener != NULL)
        {
            m_listener->HandleStateChange(m_stateChanges.front().eOldState, m_stateChanges.front().eNewState,
                                          m_stateChanges.front().bError);
            if (m_stateChanges.front().eNewState == eNQRNetworkManagerState_Idle)
            {
                m_receiving = false;
                OSEnableHomeButtonMenu(TRUE);
            }
        }
        m_state = m_stateChanges.front().eNewState;
        m_stateChanges.pop_front();
    }
    LeaveCriticalSection(&m_stateChangeLock);
    if (s_connectCompleted && !s_connectThreadRunning)
    {
        s_connectCompleted = false;
        if (s_connectCallback != NULL)
        {
            if (s_connectFailed)
            {
                if (s_connectShowProgress)
                {
                    s_connectCallback(s_connectCallbackData, false, 0);
                    s_connectFailed = false;
                    s_connectCallback = NULL;
                    s_connectShowProgress = false;
                    return;
                }
            }
            else
            {
                s_connectCallback(s_connectCallbackData, true, 0);
            }
        }
        s_connectFailed = false;
        s_connectShowProgress = false;
        s_connectCallback = NULL;
    }
}

void NQRNetworkManager_WiiU::RunTickThread()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Vec3::CreateNewThreadStorage();
    AABB::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Compression::UseDefaultThreadStorage();
    Block::CreateNewThreadStorage();
    while (true)
    {
        if (m_tickEvent->WaitForSignal(cTickThreadWaitTime) == 0)
        {
            TickFromThread();
        }
        else if (s_tickConnectionsOnNetworkThread)
        {
            if (!IsHost())
            {
                TickFromThread();
                if (IsInSession())
                    LeaveRoom(true);
                minecraft->tickAllConnections();
            }
            else
            {
                TickNexAsyncProcess();
            }
        }
    }
}

int NQRNetworkManager_WiiU::TickThreadProc(void* context)
{
    static_cast<NQRNetworkManager_WiiU*>(context)->RunTickThread();
    return 0;
}

void NQRNetworkManager_WiiU::MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo)
{
    GameSessionData* sessionData = static_cast<GameSessionData*>(std::calloc(sizeof(GameSessionData), 1));
    if (presenceInfo->m_usTitleVersion != 0)
    {
        sessionData->m_version = presenceInfo->m_usTitleVersion;
        sessionData->m_texturePackParentId = presenceInfo->m_uiWorldId;
        sessionData->m_subTexturePackId = presenceInfo->m_ucGameMode;
        sessionData->m_bPublicGame = !presenceInfo->m_bJoinable;
        sessionData->m_bGameStarted = true;
        sessionData->m_gameModeId = presenceInfo->m_ucPlayerCount;
        sessionData->m_miniGameId = presenceInfo->m_ucMaxPlayers;
        sessionData->m_customGameModeHostOptions = presenceInfo->m_uiHostAttributes;
    }
    *extData = sessionData;
}

void NQRNetworkManager_WiiU::UpdateFriendFromPresence(unsigned int principalId)
{
    EnterCriticalSection(&m_friendSearchMutex);
    nn::fp::FriendPresence presence;
    nn::fp::GetFriendPresence(&presence, &principalId, 1);
    FriendSearchResult* friendResult = NULL;
    unsigned int index;
    for (index = 0; index < m_friendSearchResults.size(); ++index)
    {
        if (m_friendSearchResults[index].friendPrincipalId == principalId)
        {
            friendResult = &m_friendSearchResults[index];
            break;
        }
    }
    if (friendResult == NULL)
    {
        FriendSearchResult newResult;
        newResult.extData = NULL;
        unsigned int friendId = principalId;
        char accountId[17];
        if (nn::fp::GetFriendAccountId(accountId, &friendId, 1).IsFailure())
        {
            LeaveCriticalSection(&m_friendSearchMutex);
            return;
        }
        std::memcpy(newResult.friendAccountId, accountId, sizeof(newResult.friendAccountId));
        newResult.friendPrincipalId = principalId;
        m_friendSearchResults.push_back(newResult);
        friendResult = &m_friendSearchResults.back();
        index = m_friendSearchResults.size() - 1;
    }
    PresenceSyncInfo* presenceInfo = reinterpret_cast<PresenceSyncInfo*>(presence.gameMode.applicationArg);
    if (!presence.isValid || !presence.isOnline || presenceInfo->m_usTitleVersion != cNetworkProtocolVersion
        || presenceInfo->m_bJoinable)
    {
        m_friendSearchResults.erase(m_friendSearchResults.begin() + index);
        m_friendPresenceUpdated = true;
    }
    else
    {
        friendResult->hostPrincipalId = presence.gameMode.ownerPrincipalId;
        friendResult->joinable = true;
        friendResult->sessionId = presence.gameMode.joinGroupId;
        delete static_cast<GameSessionData*>(friendResult->extData);
        MallocAndSetExtDataFromNQRPresenceInfo(&friendResult->extData, presenceInfo);
        m_friendPresenceUpdated = true;
    }
    LeaveCriticalSection(&m_friendSearchMutex);
}

void NQRNetworkManager_WiiU::UpdateFriendFromPresence_callback(unsigned int principalId, void* context)
{
    static_cast<NQRNetworkManager_WiiU*>(context)->UpdateFriendFromPresence(principalId);
}

void NQRNetworkManager_WiiU::Initialise()
{
    s_lastNetworkErrorCode = 0;
    s_lastPiaResult = 0;
    gProfile->SetFriendPresenceChangeCallback(UpdateFriendFromPresence_callback, this);
    m_tickEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_tickThread = new C4JThreadImpl(TickThreadProc, this, "NQRNetworkManager_WiiU::TickThreadProc", 0);
    m_tickThread->SetProcessor(s_tickThreadProcessor);
    m_tickThread->Run();
    TryToConnectOnline();
    SetState(eNQRNetworkManagerInternalState_Idle);
    NintendoVoiceChat_WiiU::Initialise(this);
}

void NQRNetworkManager_WiiU::Terminate()
{
    if ((m_internalState == eNQRNetworkManagerInternalState_HostSessionCreated
         || m_internalState == eNQRNetworkManagerInternalState_HostRoomReady
         || m_internalState == eNQRNetworkManagerInternalState_JoinRoomReady
         || m_internalState == eNQRNetworkManagerInternalState_Playing)
        && !m_localGame)
    {
        LeaveRoom(true);
        int ticksRemaining = cTerminateLeaveRoomTicks;
        do
        {
            Tick();
            Sleep(cTerminateLeaveRoomSleepTime);
            if (--ticksRemaining <= 0)
                return;
        } while (m_internalState != eNQRNetworkManagerInternalState_Idle);
    }
}

NQRNetworkManager::eNQRNetworkManagerState NQRNetworkManager_WiiU::GetState()
{
    return m_state;
}

bool NQRNetworkManager_WiiU::IsHost()
{
    return m_host;
}

bool NQRNetworkManager_WiiU::IsReadyToPlayOrIdle()
{
    return m_stateChanges.empty()
           && (m_internalState == eNQRNetworkManagerInternalState_HostRoomReady
               || m_internalState == eNQRNetworkManagerInternalState_Playing
               || m_internalState == eNQRNetworkManagerInternalState_Idle);
}

bool NQRNetworkManager_WiiU::IsInSession()
{
    return m_receiving;
}

bool NQRNetworkManager_WiiU::IsMatchmakingEnabled()
{
    return m_matchmaking;
}

void NQRNetworkManager_WiiU::CreateAndJoinRoom(int userIndex, int localUserMask, void* extData, int extDataSize,
                                               int* attributes, int attributeCount, bool localGame,
                                               unsigned char gameMode)
{
    if (!localGame)
        OSEnableHomeButtonMenu(0);

    m_externalDataSize = extDataSize;
    m_receiving = true;
    m_sessionJoined = false;
    m_host = true;
    m_gameMode = gameMode;
    m_localGame = localGame;
    m_roomAttributes = attributes;
    m_roomAttributeCount = attributeCount;
    m_unknownDD0 = 0;
    m_externalData = extData;
    if (attributeCount > 0)
        m_matchmaking = true;
    m_unknownDC0 = 0;

    std::memset(m_roomPlayers, 0, sizeof(m_roomPlayers));
    m_roomPlayers[0].m_uiLocalPlayerIndex = userIndex;
    m_roomPlayers[0].m_uiPlayerCountMinusOne = 0;
    PlayerSyncData& primary = m_roomPlayers[0];
    primary.m_uiSmallId = m_nextSmallId++;
    gProfile->GetXUID(userIndex, &primary, !localGame);
    const std::wstring primaryMiiName(gProfile->GetMiiName(userIndex));
    const std::wstring primaryDisplayName(gProfile->GetDisplayName(userIndex));
    std::wcscpy(primary.m_szMiiName, primaryMiiName.c_str());
    std::wcstombs(primary.m_szNetworkName, primaryDisplayName.c_str(), 20);

    const unsigned int remainingMask = static_cast<unsigned int>(localUserMask) & ~(1u << userIndex);
    for (int localIndex = 0; localIndex < 4; ++localIndex)
    {
        if ((remainingMask & (1u << localIndex)) == 0)
            continue;
        PlayerSyncData& record = m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1];
        record.m_uiSmallId = m_nextSmallId++;
        record.m_uiLocalPlayerIndex = localIndex;
        gProfile->GetXUID(userIndex, &record, !localGame);
        const std::wstring miiName(gProfile->GetMiiName(localIndex));
        const std::wstring displayName(gProfile->GetDisplayName(localIndex));
        std::wcscpy(record.m_szMiiName, miiName.c_str());
        std::wcstombs(record.m_szNetworkName, displayName.c_str(), 20);
        ++m_roomPlayers[0].m_uiPlayerCountMinusOne;
    }
    m_localPlayerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;

    if (m_localGame)
    {
        SetState(eNQRNetworkManagerInternalState_HostSessionCreated);
        SetState(eNQRNetworkManagerInternalState_HostRoomReady);
        SetLocalPlayersAndSync();
    }
    else
    {
        Pia_SignalCreateSession();
    }
}

void NQRNetworkManager_WiiU::UpdateExternalRoomData()
{
    m_updateRequested = true;
}

bool NQRNetworkManager_WiiU::FriendRoomManagerIsBusy()
{
    return m_friendSearchState != eNQRNetworkManagerFriendSearchState_Idle;
}

bool NQRNetworkManager_WiiU::FriendRoomManagerSearch()
{
    EnterCriticalSection(&m_friendSearchMutex);
    if (m_internalState == eNQRNetworkManagerInternalState_Idle
        && m_friendSearchState == eNQRNetworkManagerFriendSearchState_Idle)
    {
        for (unsigned int index = 0; index < m_friendSearchResults.size(); ++index)
        {
            if (m_friendSearchResults[index].extData != NULL)
                std::free(m_friendSearchResults[index].extData);
            m_friendSearchResults[index].extData = NULL;
        }
        m_friendPresenceCount = 0;
        m_friendSearchState = eNQRNetworkManagerFriendSearchState_Searching;
        m_friendSearchResults.clear();
        m_friendSearchThread = new C4JThreadImpl(GetFriendsThreadProc, this, "GetFriendsThreadProc", 0);
        m_friendSearchThread->Run();
        LeaveCriticalSection(&m_friendSearchMutex);
        return true;
    }
    LeaveCriticalSection(&m_friendSearchMutex);
    return false;
}

bool NQRNetworkManager_WiiU::FriendRoomManagerSearch2()
{
    EnterCriticalSection(&m_friendSearchMutex);
    if (m_friendPresenceCount == 0)
    {
        m_friendSearchState = eNQRNetworkManagerFriendSearchState_Idle;
        LeaveCriticalSection(&m_friendSearchMutex);
        return false;
    }
    if (!m_friendSearchResults.empty())
    {
        boost::unordered_set<unsigned int> sessionIds;
        for (unsigned int index = 0; index < m_friendSearchResults.size(); ++index)
            if (m_friendSearchResults[index].joinable)
                sessionIds.insert(m_friendSearchResults[index].sessionId);
        for (unsigned int index = 0; index < m_friendSearchResults.size();)
        {
            if (sessionIds.find(m_friendSearchResults[index].sessionId) == sessionIds.end())
            {
                std::free(m_friendSearchResults[index].extData);
                m_friendSearchResults[index] = m_friendSearchResults.back();
                m_friendSearchResults.pop_back();
            }
            else
            {
                sessionIds.erase(m_friendSearchResults[index].sessionId);
                ++index;
            }
        }
    }
    m_friendSearchState = eNQRNetworkManagerFriendSearchState_Idle;
    LeaveCriticalSection(&m_friendSearchMutex);
    return true;
}

int NQRNetworkManager_WiiU::FriendRoomManagerGetCount()
{
    EnterCriticalSection(&m_friendSearchMutex);
    int count = static_cast<int>(m_friendSearchResults.size());
    LeaveCriticalSection(&m_friendSearchMutex);
    return count;
}

void NQRNetworkManager_WiiU::FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result)
{
    EnterCriticalSection(&m_friendSearchMutex);
    result->m_uiOwnerPrincipalId = m_friendSearchResults[index].friendPrincipalId;
    std::memcpy(result->m_szOwnerName, m_friendSearchResults[index].friendAccountId, sizeof(result->m_szOwnerName));
    result->m_uiOwnerLocalFriendIndex = m_friendSearchResults[index].hostPrincipalId;
    result->m_pExtData = m_friendSearchResults[index].extData;
    result->m_uiSessionId = m_friendSearchResults[index].sessionId;
    LeaveCriticalSection(&m_friendSearchMutex);
}

NQRNetworkManager::SessionSearchResult* NQRNetworkManager_WiiU::GetFriendRoomSearchResults(int* count)
{
    EnterCriticalSection(&m_friendSearchMutex);
    int resultCount = FriendRoomManagerGetCount();
    SessionSearchResult* results = new SessionSearchResult[resultCount];
    for (int index = 0; index < resultCount; ++index)
        FriendRoomManagerGetRoomInfo(index, &results[index]);
    *count = FriendRoomManagerGetCount();
    m_friendPresenceUpdated = false;
    LeaveCriticalSection(&m_friendSearchMutex);
    return results;
}

bool NQRNetworkManager_WiiU::JoinRoom(SessionSearchResult* result, int userIndexMask)
{
    SetNQRPresenceInfoFromExtData(&s_presenceInfo, result->m_pExtData);
    return JoinRoom(result->m_uiSessionId, userIndexMask, NULL);
}

bool NQRNetworkManager_WiiU::JoinRoom(unsigned int sessionId, int userIndexMask, const PresenceSyncInfo* presenceInfo)
{
    if (m_internalState == eNQRNetworkManagerInternalState_Idle)
        SetState(eNQRNetworkManagerInternalState_JoinRequested);
    OSEnableHomeButtonMenu(FALSE);
    if (presenceInfo != NULL)
        std::memcpy(&s_presenceInfo, presenceInfo, sizeof(PresenceSyncInfo));
    m_pendingUserMask = userIndexMask;
    m_pendingSessionId = sessionId;
    m_localPlayersJoined = 0;
    m_receiving = true;
    m_localGame = false;
    m_host = false;
    m_sessionJoined = false;
    m_localPlayerCount = 0;
    for (int i = 0; i < 4; i++)
    {
        if (userIndexMask & (1 << i))
            m_localPlayerCount++;
    }
    Pia_SignalJoinSession();
    return true;
}

bool NQRNetworkManager_WiiU::JoinRoom(int userIndexMask, const nn::fp::FriendPresence* friendPresence)
{
    return JoinRoom(friendPresence->gameMode.joinGroupId, userIndexMask,
                    reinterpret_cast<const PresenceSyncInfo*>(friendPresence->gameMode.applicationArg));
}

bool NQRNetworkManager_WiiU::FriendPresenceUpdatedRoomList()
{
    return m_friendPresenceUpdated;
}

void NQRNetworkManager_WiiU::GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*),
                                               void* callbackData)
{
    m_extDataCallback = callback;
    m_extDataCallbackData = callbackData;
    m_extDataBuffer = extData;
    s_extDataRequested = true;
    s_extDataSessionId = sessionId;
}

bool NQRNetworkManager_WiiU::FindAndJoinRoom(int userIndexMask, unsigned int searchParamCount,
                                             CMatchMakeSearchParam* searchParam, unsigned char gameMode)
{
    OSEnableHomeButtonMenu(0);
    m_gameMode = gameMode;
    m_pendingUserMask = userIndexMask;
    m_matchmaking = true;
    m_localPlayersJoined = 0;
    m_receiving = true;
    m_host = false;
    m_localGame = false;
    m_localPlayerCount = 0;
    m_searchCriteria.SetGameMode(gameMode);
    m_searchCriteria.SetMinParticipantNum(0);
    m_searchCriteria.SetMaxParticipantNum(cMaxSessionParticipants);
    for (unsigned int index = 0; index < nn::pia::inet::NexSessionSearchCriteria::cAttributeNum; ++index)
        m_searchCriteria.SetAttributeRange(index, 0, cSearchAttributeValueMax);
    for (int index = 0; index < searchParamCount; ++index)
    {
        switch (searchParam[index].comparison)
        {
        case CMatchMakeSearchParam::eComparison_Equal:
            m_searchCriteria.SetAttribute(searchParam[index].id, searchParam[index].value);
            break;
        case CMatchMakeSearchParam::eComparison_LessThanOrEqual:
            m_searchCriteria.SetAttributeRange(searchParam[index].id, 0, searchParam[index].value);
            break;
        case CMatchMakeSearchParam::eComparison_GreaterThanOrEqual:
            m_searchCriteria.SetAttributeRange(searchParam[index].id, searchParam[index].value,
                                               cSearchAttributeValueMax);
            break;
        case CMatchMakeSearchParam::eComparison_LessThan:
            m_searchCriteria.SetAttributeRange(searchParam[index].id, 0, searchParam[index].value - 1);
            break;
        case CMatchMakeSearchParam::eComparison_GreaterThan:
            m_searchCriteria.SetAttributeRange(searchParam[index].id, searchParam[index].value + 1,
                                               cSearchAttributeValueMax);
            break;
        }
    }
    m_searchCriteria.SetResultRange(cSearchResultOffsetRandom, cSearchResultCount);
    for (int i = 0; i < 4; i++)
    {
        if (userIndexMask & (1 << i))
            m_localPlayerCount++;
    }
    SetState(eNQRNetworkManagerInternalState_JoinSearchingMatchmakeSession);
    Pia_SignalBrowseSession();
    return true;
}

void NQRNetworkManager_WiiU::Pia_SignalBrowseSession()
{
    m_browseRequested = true;
}

unsigned int NQRNetworkManager_WiiU::GetFriendsFromPresence()
{
    EnterCriticalSection(&m_friendSearchMutex);
    m_friendSearchResults.clear();
    m_friendPresenceCount = 0;
    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
    {
        unsigned int principalId = nn::act::GetPrincipalId();
        unsigned int friendCount;
        unsigned int friendIds[cMaxFriends];
        char accountIds[cMaxFriends][17];
        nn::fp::FriendPresence presences[cMaxFriends];
        if (nn::fp::GetFriendList(friendIds, &friendCount, 0, cMaxFriends).IsSuccess()
            && nn::fp::GetFriendPresence(presences, friendIds, friendCount).IsSuccess()
            && nn::fp::GetFriendAccountId(&accountIds[0][0], friendIds, friendCount).IsSuccess() && friendCount != 0)
        {
            FriendSearchResult friendResult;
            for (unsigned int index = 0; index < friendCount; ++index)
            {
                ++m_friendPresenceCount;
                if (presences[index].isValid && presences[index].isOnline
                    && presences[index].gameMode.ownerPrincipalId != principalId)
                {
                    friendResult.friendPrincipalId = friendIds[index];
                    std::memcpy(friendResult.friendAccountId, accountIds[index], sizeof(friendResult.friendAccountId));
                    PresenceSyncInfo* presenceInfo = reinterpret_cast<PresenceSyncInfo*>(
                        presences[index].gameMode.applicationArg);
                    if (presenceInfo->m_usTitleVersion == cNetworkProtocolVersion && !presenceInfo->m_bJoinable)
                    {
                        friendResult.hostPrincipalId = presences[index].gameMode.ownerPrincipalId;
                        friendResult.sessionId = presences[index].gameMode.joinGroupId;
                        friendResult.joinable = true;
                        MallocAndSetExtDataFromNQRPresenceInfo(&friendResult.extData, presenceInfo);
                        m_friendSearchResults.push_back(friendResult);
                    }
                    else
                        friendResult.joinable = false;
                }
            }
            m_friendPresenceUpdated = false;
        }
    }
    LeaveCriticalSection(&m_friendSearchMutex);
    return 0;
}

int NQRNetworkManager_WiiU::GetFriendsThreadProc(void* context)
{
    return static_cast<NQRNetworkManager_WiiU*>(context)->GetFriendsFromPresence();
}

void NQRNetworkManager_WiiU::StartGame()
{
    SetState(eNQRNetworkManagerInternalState_StartingGame);
    SetState(eNQRNetworkManagerInternalState_Playing);
}

void NQRNetworkManager_WiiU::ClearRichPresenceCustomData()
{
    m_presenceGameMode.joinAvailabilityFlag = FALSE;
    m_presenceGameMode.matchmakeSystemType = cPresenceMatchmakeSystemType;
    m_presenceGameMode.joinGameId = s_nexGameServerId;
    m_presenceGameMode.joinGameMode = 0;
    m_presenceGameMode.ownerPrincipalId = 0;
    m_presenceGameMode.joinGroupId = 0;
    std::memset(m_presenceGameMode.applicationArg, 0, sizeof(m_presenceGameMode.applicationArg));
    gProfile->SetRichPresenceGameModeData(m_presenceGameMode);
}

void NQRNetworkManager_WiiU::LeaveRoom(bool actuallyLeaveRoom)
{
    if (m_localGame)
    {
        if (m_internalState != eNQRNetworkManagerInternalState_Playing)
            return;
        SetState(eNQRNetworkManagerInternalState_LeavingRoom);
        SetState(eNQRNetworkManagerInternalState_Ending);
        ResetToIdle();
        return;
    }
    if (m_internalState == eNQRNetworkManagerInternalState_LeavingRoom)
        return;
    ClearRichPresenceCustomData();
    if (actuallyLeaveRoom)
    {
        switch (m_internalState)
        {
        case eNQRNetworkManagerInternalState_HostSessionCreated:
        case eNQRNetworkManagerInternalState_HostRoomReady:
        case eNQRNetworkManagerInternalState_JoinRoomReady:
        case eNQRNetworkManagerInternalState_Playing:
            SetState(eNQRNetworkManagerInternalState_LeavingRoom);
            if (m_unknown13A0)
                Pia_SignalLeaveSession();
            else
                SetState(eNQRNetworkManagerInternalState_Ending);
            break;
        default:
            if (m_internalState == eNQRNetworkManagerInternalState_HostCreatingSession)
                SetState(eNQRNetworkManagerInternalState_Ending);
            else
                SetState(eNQRNetworkManagerInternalState_Idle);
            break;
        }
        if (m_host)
        {
            EnterCriticalSection(&m_connectedStationMutex);
            while (m_connectedStations.size() != 0)
            {
                RemoveRemotePlayersAndSync(m_connectedStations[0].stationId, cAllLocalPlayersMask);
                RemoveConnectedStation(m_connectedStations[0].stationId);
            }
            LeaveCriticalSection(&m_connectedStationMutex);
        }
        else
            RemoveAllPlayersOnClient();
    }
    else
        SetState(eNQRNetworkManagerInternalState_Ending);
}

void NQRNetworkManager_WiiU::EndGame()
{
    return;
}

bool NQRNetworkManager_WiiU::SessionHasSpace(int extraPlayers)
{
    return static_cast<int>(m_roomPlayers[0].m_uiPlayerCountMinusOne + 1 + extraPlayers)
           <= static_cast<int>(CGameNetworkManager::sInstance.GetMaxPlayers());
}

bool NQRNetworkManager_WiiU::SessionHasMii(int userIndex)
{
    PlayerUID uid(0);
    gProfile->GetXUID(userIndex, &uid, true);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        if (m_roomPlayers[index] == uid)
            return true;
    return false;
}

void NQRNetworkManager_WiiU::UpdateRemotePlay()
{
    int localPlayerCount = 0;
    if (GetLocalPlayerByUserIndex(0) != NULL)
        ++localPlayerCount;
    if (GetLocalPlayerByUserIndex(1) != NULL)
        ++localPlayerCount;
    if (GetLocalPlayerByUserIndex(2) != NULL)
        ++localPlayerCount;
    if (GetLocalPlayerByUserIndex(3) != NULL)
        ++localPlayerCount;
    g_pInput->SetLocalMultiplayer(localPlayerCount > 1);
}

bool NQRNetworkManager_WiiU::AddLocalPlayerByUserIndex(int userIndex)
{
    if (m_host)
    {
        if (CGameNetworkManager::sInstance.GetMaxPlayers() == m_roomPlayers[0].m_uiPlayerCountMinusOne + 1)
            return false;
        int playerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
        int insertIndex;
        for (insertIndex = 0; insertIndex < playerCount; ++insertIndex)
        {
            if (m_roomPlayers[insertIndex].m_uiStationId != m_localStationId)
                break;
            if (m_roomPlayers[insertIndex].m_uiLocalPlayerIndex == userIndex)
                return false;
        }
        for (int index = playerCount; index > insertIndex; --index)
            m_roomPlayers[index] = m_roomPlayers[index - 1];
        PlayerSyncData& record = m_roomPlayers[insertIndex];
        record.m_uiLocalPlayerIndex = userIndex;
        record.m_uiStationId = m_localStationId;
        record.m_uiSmallId = m_nextSmallId++;
        std::wstring miiName(gProfile->GetMiiName(userIndex));
        std::wstring displayName = gProfile->GetDisplayName(userIndex);
        std::wcscpy(record.m_szMiiName, miiName.c_str());
        std::wcstombs(record.m_szNetworkName, displayName.c_str(), cNetworkNameLength);
        ++m_roomPlayers[0].m_uiPlayerCountMinusOne;
        MapRoomSlotPlayers(-1);
        SyncRoomData();
        UpdateRemotePlay();
        return true;
    }
    if (m_roomPlayers[0].m_uiPlayerCountMinusOne + 1 == CGameNetworkManager::sInstance.GetMaxPlayers())
        return false;
    unsigned int playerMask = 1 << userIndex;
    if ((m_pendingUserMask & playerMask) != 0)
        return false;
    m_pendingUserMask |= playerMask;
    SendJoinData();
    UpdateRemotePlay();
    return true;
}

void NQRNetworkManager_WiiU::SendLeaveData(unsigned int playerMask)
{
    if (!m_localGame)
        RemoteDataSend(m_hostStationId, cLeaveChannel, &playerMask, sizeof(playerMask));
}

bool NQRNetworkManager_WiiU::RemoveLocalPlayerByUserIndex(int userIndex)
{
    if (!m_host)
    {
        unsigned int playerMask = 1 << userIndex;
        if ((m_pendingUserMask & playerMask) == 0)
            return false;
        m_pendingUserMask &= ~playerMask;
        SendLeaveData(playerMask);
        RemoveLocalClientPlayers(playerMask);
        UpdateRemotePlay();
        return true;
    }
    EnterCriticalSection(&m_playerMutex);
    int previousPlayerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    for (int index = 0; index < previousPlayerCount; ++index)
    {
        if (m_roomPlayers[index].m_uiStationId == m_localStationId
            && m_roomPlayers[index].m_uiLocalPlayerIndex == userIndex)
        {
            m_roomPlayers[0].m_uiPlayerCountMinusOne--;
            for (int slot = index; slot < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++slot)
                m_roomPlayers[slot] = m_roomPlayers[slot + 1];
            std::memset(&m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1], 0, sizeof(PlayerSyncData));
            MapRoomSlotPlayers(previousPlayerCount);
            m_roomSlotPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1] = NULL;
            SyncRoomData();
            LeaveCriticalSection(&m_playerMutex);
            return true;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    UpdateRemotePlay();
    return false;
}

bool NQRNetworkManager_WiiU::IsHandlingBootInvite()
{
    return g_bHandlingGameBootInvite;
}

int NQRNetworkManager_WiiU::GetPlayerCount()
{
    return m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
}

int NQRNetworkManager_WiiU::GetOnlinePlayerCount()
{
    int count = 0;
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        if (m_roomPlayers[index].m_uiStationId != m_localStationId)
            ++count;
    return count;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerIfReady(NQRNetworkPlayer* player)
{
    return player != NULL && player->IsReady() ? player : NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerByIndex(int index)
{
    return index < 8 ? GetPlayerIfReady(m_roomSlotPlayers[index]) : NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerBySmallId(int smallId)
{
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomPlayers[index].m_uiSmallId == smallId)
        {
            NQRNetworkPlayer* player = GetPlayerIfReady(m_roomSlotPlayers[index]);
            LeaveCriticalSection(&m_playerMutex);
            return player;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerByXuid(PlayerUID uid)
{
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomPlayers[index] == uid)
        {
            NQRNetworkPlayer* player = GetPlayerIfReady(m_roomSlotPlayers[index]);
            LeaveCriticalSection(&m_playerMutex);
            return player;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return NULL;
}

std::wstring NQRNetworkManager_WiiU::GetDisplayNameByGamertag(std::wstring gamertag)
{
    if (m_miiNames.find(gamertag) != m_miiNames.end())
        return m_miiNames[gamertag];
    return gamertag;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetLocalPlayerByUserIndex(int userIndex)
{
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomPlayers[index].m_uiStationId == m_localStationId
            && m_roomPlayers[index].m_uiLocalPlayerIndex == userIndex)
        {
            NQRNetworkPlayer* player = GetPlayerIfReady(m_roomSlotPlayers[index]);
            LeaveCriticalSection(&m_playerMutex);
            return player;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetHostPlayer()
{
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* player = m_roomSlotPlayers[0];
    LeaveCriticalSection(&m_playerMutex);
    return player;
}

void NQRNetworkManager_WiiU::SetAppSuspended(bool suspended)
{
    s_appSuspended = suspended;
}

void NQRNetworkManager_WiiU::LocalDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data,
                                           unsigned int size)
{
    if (m_listener != NULL)
        m_listener->HandleDataReceived(from, to, static_cast<unsigned char*>(const_cast<void*>(data)), size);
}

void NQRNetworkManager_WiiU::RemoteDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data,
                                            unsigned int size)
{
    unsigned char channel = from->GetLocalPlayerIndex();
    RemoteDataSend(to->GetStationId(), channel, data, size);
}

int NQRNetworkManager_WiiU::GetSessionIndex(NQRNetworkPlayer* player)
{
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        if (m_roomSlotPlayers[index] == player)
            return index;
    return 0;
}

void NQRNetworkManager_WiiU::SetPresenceDataStartHostingGame()
{
    if (m_localGame)
    {
        ClearRichPresenceCustomData();
        return;
    }
    PresenceSyncInfo presenceInfo;
    SetNQRPresenceInfoFromExtData(&presenceInfo, m_externalData);
    UpdateRichPresenceCustomData(&presenceInfo, sizeof(presenceInfo));
}

int NQRNetworkManager_WiiU::GetJoiningReadyPercentage()
{
    if (m_internalState == eNQRNetworkManagerInternalState_HostWaitingForLogin
        || m_internalState == eNQRNetworkManagerInternalState_JoinWaitingForLogin)
        return 50;
    return 100;
}

void NQRNetworkManager_WiiU::SetApplicationDisconnected()
{
    s_applicationConnected = false;
    s_applicationConnectedPrevious = false;
    gProfile->SetNetworkStatusWithoutSigninChange(false);
}

void NQRNetworkManager_WiiU::Pia_ShutdownForLan()
{
    s_shutdownForLan = true;
    if (s_piaInitialised)
    {
        s_shutdownTickPending = true;
        s_piaShutdownPending = true;
        SetApplicationDisconnected();
    }
}

void NQRNetworkManager_WiiU::TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData,
                                                bool showProgress)
{
    s_connectCallback = callback;
    s_connectShowProgress = showProgress;
    s_connectCallbackData = callbackData;
    TryToConnectOnline();
}

bool NQRNetworkManager_WiiU::isAdhocConnected()
{
    return false;
}

bool NQRNetworkManager_WiiU::connectToAdhoc()
{
    return false;
}

bool NQRNetworkManager_WiiU::isLanConnected()
{
    return false;
}

void NQRNetworkManager_WiiU::SendInviteGUI()
{
}
