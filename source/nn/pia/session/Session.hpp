#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP

#include <cstddef>

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class Job;
class StepSequenceJob;
class String;
}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class BrowseMatchmakeJob;
class ClearMatchmakeSystemPasswordJob;
class CloseParticipationJob;
class CommunityManagementJob;
class CommunitySearchCriteria;
class CreateSessionJob;
class CreateSessionSetting;
class DestroySessionJob;
class GenerateMatchmakeSystemPasswordJob;
class ICommunityInfoList;
class IMatchmakeSession;
class ISessionInfo;
class ISessionInfoList;
class JoinRandomSessionSetting;
class JoinSessionJob;
class JoinSessionSetting;
class JointSessionJob;
class LeaveSessionJob;
class MeshEventListener;
class MeshLayerController;
class OpenParticipationJob;
class RandomMatchmakeJob;
class SessionNetworkFactory;
class SessionProtocol;
class SessionSearchCriteria;
class SessionStatusCheckJob;
class StationIdStatusTable;
class UpdateSessionSetting;
class UpdateSessionSettingJob;

class Session
{
public:
    enum Status
    {
        cStatusNone = 0,
        cStatusMatched = 1,
        cStatusUnknown2 = 2,
        cStatusBufferMatched = 3,
        cStatusDisconnectedSession = 4,
        cStatusDisconnectedNetwork = 5,
        cStatusUnknown6 = 6,
        cStatusUnknown7 = 7,
        cStatusUnknown8 = 8
    };

    enum StatusResult
    {
        cResultStatusDisconnectedSession = 0x8c56,
        cResultStatusDisconnectedNetwork = 0xc406,
        cResultStatusUnknown6 = 0xa471,
        cResultStatusUnknown7 = 0xc420,
        cResultStatusUnknown8 = 0xc421
    };

    enum MatchmakeStatus
    {
        cMatchmakeStatusNone = 0,
        cMatchmakeStatusSearching = 1,
        cMatchmakeStatusMatched = 2
    };

    enum ConnectionStatus
    {
        cConnectionStatusNone = 0,
        cConnectionStatusConnected = 1,
        cConnectionStatusDisconnectedSession = 2,
        cConnectionStatusDisconnectedNetwork = 3,
        cConnectionStatusUnknown4 = 4,
        cConnectionStatusUnknown5 = 5,
        cConnectionStatusUnknown6 = 6
    };

    enum ProcessType
    {
        cProcessTypeNone = 0,
        cProcessTypeCreateJointSession = 6,
        cProcessTypeJoinJointSession = 7,
        cProcessTypeJoinRandomJointSession = 8,
        cProcessTypeDestroyJointSession = 9,
        cProcessTypeLeaveJointSession = 10
    };

    enum EventType
    {
        cEventTypeJoin = 0,
        cEventTypeLeave = 1,
        cEventTypeUnknown02 = 2,
        cEventTypeUnknown03 = 3,
        cEventTypeUnknown04 = 4,
        cEventTypeUnknown15 = 0x15,
        cEventTypeSessionSystemPasswordSet = 0x16,
        cEventTypeSessionSystemPasswordClear = 0x17
    };

    enum SessionOpenStatus
    {
        cSessionOpenStatusClosed = 0,
        cSessionOpenStatusOpen = 1,
        cSessionOpenStatusParticipationClosed = 2
    };

    enum AsyncProcessingName
    {
        cAsyncProcessingNameNone = 0,
        cAsyncProcessingNameJoinRandomSession = 1,
        cAsyncProcessingNameBrowseSession = 2,
        cAsyncProcessingNameCreateSession = 3,
        cAsyncProcessingNameJoinSession = 4,
        cAsyncProcessingNameLeaveSession = 5,
        cAsyncProcessingNameUpdateSessionSetting = 0x11,
        cAsyncProcessingNameBrowseCommunity = 0x15,
        cAsyncProcessingNameUnknown18 = 0x18,
        cAsyncProcessingNameUnknown19 = 0x19
    };

    typedef void (*SessionEventCallback)(EventType eventType, unsigned long long stationId, void* param);

    typedef common::PooledList<unsigned long long> StationIdList;

    class StartupSetting
    {
    public:
        StartupSetting() : m_field00(10000), m_field04(1000), m_identificationToken(0) {}

        unsigned int m_field00;
        unsigned int m_field04;
        const transport::Station::IdentificationToken* m_identificationToken;
    };

    class Setting
    {
    public:
        SessionNetworkFactory* m_networkFactory;
        unsigned int m_networkTopologyType;
        unsigned short m_browseSessionInfoListSize;
        unsigned char m_browseCommunityInfoListSize;
        unsigned int m_bitRateCheckMode;
    };

    struct DebugSetting
    {
        DebugSetting() : m_relayEmulationEnabled(0), m_matchmakeAnalysisEnabled(0), m_checkStartupSettingRange(1) {}

        unsigned char m_relayEmulationEnabled;
        unsigned char m_matchmakeAnalysisEnabled;
        unsigned char m_checkStartupSettingRange;
    };

    static DebugSetting s_debugSetting;

    explicit Session(const Setting& setting);
    ~Session();

    static Result CreateInstance(const Setting& setting);
    static void DestroyInstance();
    void Cleanup();
    void CleanupStatus(bool keepStationIds);
    void ResetSessionSystemPassword();
    Result SetSessionSystemPassword(const common::String& password);
    void ClearWaitingStationIndex();
    bool LeaveNoticeToWaitingStationIndex(StationIndex stationIndex);
    bool IsAsyncProcessCompleted(AsyncProcessingName name) const;
    Result GetAsyncProcessResult(AsyncProcessingName name) const;
    Result JoinRandomSessionAsync(const JoinRandomSessionSetting* setting);
    bool IsJoinRandomSessionCompleted() const;
    Result GetJoinRandomSessionResult() const;
    Result BrowseSessionAsync(const SessionSearchCriteria* criteria);
    bool IsBrowseSessionCompleted() const;
    Result GetBrowseSessionResult() const;
    Result BrowseCommunityAsync(const CommunitySearchCriteria* criteria);
    ISessionInfoList* GetBrowsedSessionInfoList();
    ICommunityInfoList* GetBrowsedCommunityInfoList();
    Result CreateSessionAsync(const CreateSessionSetting* setting);
    bool IsCreateSessionCompleted() const;
    Result GetCreateSessionResult() const;
    bool IsJoinSessionCompleted() const;
    Result GetJoinSessionResult() const;
    Result LeaveSessionAsync();
    bool IsLeaveSessionCompleted() const;
    Result GetLeaveSessionResult() const;
    bool IsUpdateSessionSettingCompleted() const;
    Result GetUpdateSessionSettingResult() const;
    Result UpdateSelectionPriority(unsigned char selectionPriority) const;
    bool IsUpdatableSelectionPriority(unsigned char selectionPriority) const;
    unsigned long long ConvertCommunityIdToCommunityCode(unsigned int communityId);
    unsigned int ConvertCommunityCodeToCommunityId(unsigned long long communityCode);
    bool IsOwner() const;
    unsigned short GetMatchmakeSessionParticipantNum() const;
    Result CheckConnectionError() const;
    const ISessionInfo* GetSessionInfo() const;
    unsigned int GetJointSessionId() const;
    unsigned int GetJointSessionOpenStatus() const;
    const ISessionInfo* GetJointSessionInfo() const;
    Result RegisterSessionEventCallback(SessionEventCallback callback, void* param);
    void UnregisterSessionEventCallback();
    static void NoticeEventCallback(EventType eventType, StationIndex stationIndex);
    unsigned int GetSessionDisconnectReason() const;
    ProcessType GetJointSessionProcessType() const;
    void SetHostMigrationOccurred(bool occurred);
    void NotifyLeave(unsigned int sessionId, unsigned long long stationId);
    void NotifyUnregister(unsigned int sessionId);
    bool IsPlayersCountedAsParticipants() const;
    unsigned char GetLocalPlayerCount() const;
    unsigned int IsPreparedForCleanup() const;
    bool AddNewStationId(unsigned long long stationId, StationIndex stationIndex, unsigned long long uniqueId,
                         unsigned int sessionId);

    typedef int (*DisconnectionEventListener)(void* event);

    typedef bool (*JoiningApprovalCallback)(const transport::Station::IdentificationToken* identificationToken);

    Result Startup(const StartupSetting& setting);
    void SetSyncClockPulseInterval(int interval);
    bool IsHost() const;
    bool IsValidStationId(unsigned long long stationId) const;
    bool IsJointSessionHost() const;
    unsigned int GetStatus() const;
    int GetJoinMeshState() const;
    unsigned int GetJoinSessionStateForDebug() const;
    unsigned int GetSessionOpenStatus() const;
    void SetCurrentSessionOpenStatus(SessionOpenStatus status);
    void SetBufferSessionOpenStatus(SessionOpenStatus status);
    bool UpdateStationIdToBecomeClient();
    bool UpdateStationIdToBecomeHost();
    void ChangeStatusToDisconnectedSession();
    unsigned short GetParticipantNum() const;
    bool IsHostMigrationEnabled() const;
    unsigned int GetMatchmakeSessionStationNum() const;
    DisconnectionEventListener GetDisconnectionEventListener();
    MeshEventListener* GetMeshEventListenerPtr();
    static unsigned char CheckHostCandidateRankingCallback(StationIndex index, bool checkHost);
    static unsigned int GetSessionIdCallback();
    unsigned short GetValidStationIdNum() const;
    unsigned short GetStationNum() const;
    unsigned long long GetJointSessionHostStationId() const;
    void NotifyOwnershipChange(unsigned int sessionId, unsigned long long stationId);
    void CancelJoinProcessByFatalError();
    bool IsPostponeNotice() const;
    void invalidateStationId(unsigned long long stationId);
    bool RemoveStationId(unsigned long long stationId, StationIndex stationIndex, unsigned long long uniqueId,
                         unsigned int);
    void JoinNotice(unsigned long long stationId);
    void validateStationId(unsigned long long stationId);
    void UpdateSessionStationInfo();
    void ChangeStationIdConvertEnableFlag(bool enable);
    unsigned int GetJointSessionStateForDebug() const;
    void CheckStatusAtChangeMeshState();
    void ChangeStatusToDisconnectedNetwork();
    bool IsProcessingJoin() const;
    unsigned int GetJoinRandomSessionStateForDebug() const;
    void CallSessionEventCallback(EventType eventType, unsigned long long stationId);
    transport::Station::IdentificationToken* GetMatchmakeAnalysisToken();
    static bool SessionJoiningApprovalCallback(const transport::Station::IdentificationToken* identificationToken);

    void ResetInternalAsyncState();
    Result JoinSessionAsync(const JoinSessionSetting* setting);
    Result UpdateSessionSettingAsync(const UpdateSessionSetting* setting);
    Result UpdateSessionSettingAsyncCore(unsigned int sessionId, IMatchmakeSession* matchmakeSession,
                                         const UpdateSessionSetting* setting, bool isJointSession);

    bool m_selectionPriorityEnabled;
    MeshLayerController* m_meshLayerController;
    SessionStatusCheckJob* m_sessionStatusCheckJob;
    CreateSessionJob* m_createSessionJob;
    RandomMatchmakeJob* m_randomMatchmakeJob;
    BrowseMatchmakeJob* m_browseMatchmakeJob;
    JoinSessionJob* m_joinSessionJob;
    LeaveSessionJob* m_leaveSessionJob;
    DestroySessionJob* m_destroySessionJob;
    OpenParticipationJob* m_openParticipationJob;
    CloseParticipationJob* m_closeParticipationJob;
    GenerateMatchmakeSystemPasswordJob* m_generateMatchmakeSystemPasswordJob;
    ClearMatchmakeSystemPasswordJob* m_clearMatchmakeSystemPasswordJob;
    common::String* m_matchmakeSystemPassword;
    JointSessionJob* m_jointSessionJob;
    UpdateSessionSettingJob* m_updateSessionSettingJob;
    unsigned int m_field40;
    CommunityManagementJob* m_communityManagementJob;
    unsigned int m_field48;
    unsigned int m_field4c;
    SessionEventCallback m_sessionEventCallback;
    void* m_sessionEventCallbackParam;
    JoiningApprovalCallback m_joiningApprovalCallback;
    StationIdStatusTable* m_stationIdStatusTable;
    unsigned short m_protocolId;
    SessionProtocol* m_sessionProtocol;
    unsigned int m_startupState;
    unsigned int m_matchmakeStatus;
    unsigned int m_disconnectStatus;
    unsigned int m_field74;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    unsigned long long m_jointSessionHostStationId;
    unsigned long long m_field90;
    common::CallContext m_callContext;
    AsyncProcessingName m_asyncProcessingName;
    MeshEventListener* m_meshEventListener;
    unsigned int m_waitingStationIndex;
    unsigned char m_fieldb8;
    unsigned int m_sessionDisconnectReason;
    unsigned char m_fieldc0;
    bool m_postponeNotice;
    unsigned short m_fieldc2[2];
    unsigned char m_currentSessionIndex;
    IMatchmakeSession* m_matchmakeSession[2];
    ISessionInfoList* m_sessionInfoList;
    unsigned int m_sessionId[2];
    ICommunityInfoList* m_communityInfoList;
    unsigned int m_sessionOpenStatus[2];
    StationIdList m_validStationIdList;
    unsigned long long* m_validStationIdBuffer;
    unsigned int m_field110;
    bool m_hostMigrationOccurred;
    transport::Station::IdentificationToken m_matchmakeAnalysisToken;
    unsigned short m_playerNum;
    unsigned short m_participantNum;
    unsigned int m_field13c;
    unsigned char m_field140;
    unsigned int m_field144;
    unsigned char m_field148;
};


int DisconnectionEventListenerForSession(void* event);

extern Session* g_piaSession;

}
}
}

#endif
