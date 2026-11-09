#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class BandwidthCheckProtocol;
class MissingStationHandler;

}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class KickoutManageJob;
class SessionNetworkFactory;
class ProcessHostMigrationJob;
class ProcessDestroyMeshJob;
class ProcessUpdateMeshJob;
class ProcessJoinRequestJob;
class RelayRouteManageJob;
class CreateMeshJob;
class JoinMeshJob;
class LeaveMeshJob;
class LeaveWithHostMigrationJob;
class DestroyMeshJob;
class MeshProtocol;
class MeshEventListener;

class Mesh
{
public:
    struct Setting
    {
        SessionNetworkFactory* m_networkFactory;
        unsigned int m_networkTopologyType;
        unsigned int m_bitRateCheckMode;
    };

    struct StartupSetting
    {
        unsigned int m_field00;
        unsigned int m_field04;
        bool m_hostMigrationEnabled;
        const transport::Station::PlayerInfo* m_playerInfo;
        unsigned char m_playerInfoSize;
        bool m_playersCountedAsParticipants;
        const unsigned long long* m_field14;
        const common::CryptoSetting* m_cryptoSetting;
        unsigned int m_bandwidthCheckMeasurementTime;
        unsigned int m_bandwidthCheckIpPacketSize;
        unsigned int m_bandwidthCheckMeasurementCount;
        bool m_skipBandwidthCheck;
        const transport::Station::IdentificationToken* m_identificationToken;
        bool m_recentPlayRecordEnabled;
    };

    struct DebugSetting
    {
        DebugSetting() : m_relayEmulationEnabled(0), m_checkStartupSettingRange(1) {}

        unsigned char m_relayEmulationEnabled;
        unsigned char m_checkStartupSettingRange;
    };

    enum State
    {
        cStateNone = 0,
        cStateCreated = 1,
        cStateStarted = 2
    };

    enum EventType
    {
        cEventTypeJoin = 0,
        cEventTypeLeave = 1
    };

    enum DisconnectReason
    {
        cDisconnectReasonNone = 0
    };

    enum EmulationType
    {
        cEmulationTypeNone = 0
    };

    typedef bool (*JoiningMeshApprovalCallback)(const transport::Station::IdentificationToken*);
    typedef unsigned char (*HostCandidateRankingCallback)(StationIndex, bool);
    typedef void (*MeshEventCallback)(EventType, unsigned long long);
    typedef unsigned int (*GetSessionIdCallback)();

    enum AsyncProcessingName
    {
        cAsyncCreateMesh = 1,
        cAsyncDestroyMesh = 2,
        cAsyncLeaveMeshWithHostMigration = 3,
        cAsyncJoinMesh = 4,
        cAsyncLeaveMesh = 5
    };
    unsigned int GetHostMigrationMode() const;
    bool IsRecentPlayRecordEnabled() const;
    bool IsBandwidthCheckProtocolSetupParamValid(int measurementTime, unsigned int ipPacketSize,
                                                 int measurementCount) const;
    void SetSyncClockPulseInterval(int interval);
    void SetSkipCleanupReport(bool skip);
    Result CheckConnectionError() const;
    unsigned int GetDisconnectReason() const;
    KickoutManageJob* GetKickoutManageJobPtr();

    Result JoinMeshAsync(const transport::StationConnectionInfo& info);
    void ResetInternalAsyncState();
    bool IsJoinMeshCompleted();
    Result GetJoinMeshResult();
    Result CancelJoinMeshAsync();
    Result LeaveMeshAsync();
    bool IsLeaveMeshCompleted();
    Result GetLeaveMeshResult();
    Result LeaveMeshWithHostMigrationAsync();
    bool IsLeaveMeshWithHostMigrationCompleted();
    Result GetLeaveMeshWithHostMigrationResult();
    Result DestroyMeshAsync();
    bool IsDestroyMeshCompleted();
    Result GetDestroyMeshResult();
    int GetJoinMeshState() const;
    void SetSkipReport(bool skip);
    void SendReport(unsigned char reason);
    unsigned int GetMeshState() const;
    StationIndex GetHostStationIndex() const;
    StationIndex GetLocalStationIndex() const;
    transport::BandwidthCheckProtocol* GetBandwidthCheckProtocol();
    void RegisterJoiningMeshApprovalCallback(JoiningMeshApprovalCallback callback);
    void UnregisterJoiningMeshApprovalCallback();
    Result RegisterCheckHostCandidateRankingCallback(HostCandidateRankingCallback callback);
    void UnregisterCheckHostCandidateRankingCallback();
    void RegisterEventListener(MeshEventListener* listener);
    void UnregisterEventListener();
    void RegisterGetSessionIdCallback(GetSessionIdCallback callback);
    void UnregisterGetSessionIdCallback();
    MeshProtocol* GetMeshProtocol();
    void* GetStationProtocol();
    void SetHostMigrationStarted(bool started);
    bool IsHostMigrationStarted();
    void SetFatalError(DisconnectReason reason);
    unsigned int GetNetworkTopologyType();
    void SetupSendReport();
    unsigned char IsProcessedAtMeshBegin() const;
    bool IsProcessingHostMigration() const;
    unsigned int GetLocalPlayerCount() const;
    bool IsPlayersCountedAsParticipants() const;
    void CleanupMonitoringBeginData();
    common::MonitoringDataSender* GetMonitoringDataSender();
    unsigned int GetBitRateCheckMode();
    void SetDenyJoinRequestByInvalidState(bool deny);
    void SetDenyJoinRequestBySessionIsFull(bool deny);
    Result SetupLocalPlayerInfo();
    Result SetupProtocols();
    void StartUse(StationIndex index);
    void FixConnectedId(StationIndex index);
    void JoinResponseComplete(StationIndex index);
    void SetMeshStartTime(common::Time time);
    void ProcessingAtMeshBegin();
    void ProcessingAtEndHM();
    void NotifyLeaveStationAddress(const common::StationAddress& address);
    void AddPlayHistory(transport::Station* station, transport::Station::PlayerInfo* info, unsigned int count);
    void NoticeMeshEvent(EventType type, StationIndex index);
    bool IsHost() const;
    unsigned short GetStationNum() const;
    bool IsMeshStartTimeValid() const;
    common::TimeSpan CalcDiffWithMeshStartTime(common::Time time) const;
    void MonitoringProcess(DisconnectReason reason, unsigned char value);
    void ProcessingAtMeshEnd(DisconnectReason reason);
    Result DestroyMesh(common::CallContext* callerContext);
    bool IsHostMigrationEnabled() const;
    Result LeaveMeshWithHostMigration(common::CallContext* callerContext);
    bool IsProcessingLeaveMesh() const;
    StationIndex GetUsableStationIndex();
    unsigned char CheckApprovalJoin(transport::Station* station);
    Result JoinMeshCore(const transport::StationConnectionInfo& info, common::CallContext* callerContext);
    Result LeaveMesh(common::CallContext* callerContext);
    long long GetTime() const;
    ProcessHostMigrationJob* GetProcessHostMigrationJobPtr();
    ProcessDestroyMeshJob* GetProcessDestroyMeshJobPtr();
    ProcessUpdateMeshJob* GetProcessUpdateMeshJobPtr();
    static Result setDebugSettingImpl(const DebugSetting& setting);
    static Result SetDebugSettingForInternal(const DebugSetting& setting);
    void TraceStationConnectionInfoList(unsigned long long value);
    void Trace(unsigned long long value) const;

    void JoinMesh(const transport::StationConnectionInfo& info, common::CallContext* callerContext);
    void KickoutStationByIndex(StationIndex index);
    void KickoutStation(unsigned long long stationId);
    unsigned long long GetHostStationId() const;
    unsigned long long GetLocalStationId() const;
    bool IsValidStationId(unsigned long long stationId) const;
    unsigned int GetValidStationIndexBitmap() const;
    bool IsBitRateCheckCompleted() const;
    Result GetUplinkBitRate(int* bitRate);
    void RegisterMeshEventCallback(MeshEventCallback callback);
    void UnregisterMeshEventCallback();
    int GetSyncClockPulseInterval() const;
    void SetMaxNumOfRelayRoutesAssignedToStation(unsigned short value);
    void SetMaxRtt(unsigned short value);
    Result GetConnectionRoute(StationIndex index, StationIndex* route);
    void RelayEmulation(StationIndex from, StationIndex to, EmulationType type);
    void ResetRelayEmulation();
    unsigned int GetHostMigrationSequence() const;
    unsigned int GetHostMigrationState() const;
    unsigned int GetProcessUpdateMeshJobStepSequence() const;
    bool IsMultiPlayerEnabled() const;
    void SetSyncClockBaseTimeForDebug(const common::Time& time);

    Mesh(const Setting& setting);
    ~Mesh();
    Result Initialize(const Setting& setting, bool relayEmulationEnabled);
    static Result CreateInstance(const Setting& setting);
    static void DestroyInstance();
    void CleanupStationsJobs();
    void CleanupJobs();
    bool IsValidStationIndex(StationIndex index) const;
    void StopUse(StationIndex index);
    void UnfixDisconnectedId(StationIndex index);
    void ResetStationIndexUsage();
    void SetMeshState(State state);
    void CleanupStatus();
    void Cleanup();
    Result CheckPlayerInfo(const transport::Station::PlayerInfo& info) const;
    Result CheckPlayerInfoSetting(const transport::Station::PlayerInfo* infos, unsigned char count) const;
    Result Startup(StartupSetting& setting);
    Result CreateMesh(common::CallContext* callerContext);
    Result CreateMeshAsync();
    bool IsAsyncProcessCompleted(AsyncProcessingName process);
    bool IsCreateMeshCompleted();
    Result GetAsyncProcessResult(AsyncProcessingName process);
    Result GetCreateMeshResult();
    Result CancelAsyncProcess(AsyncProcessingName process);

    CreateMeshJob* m_createMeshJob;
    JoinMeshJob* m_joinMeshJob;
    LeaveMeshJob* m_leaveMeshJob;
    ProcessJoinRequestJob* m_unknown0c;
    ProcessUpdateMeshJob* m_processUpdateMeshJob;
    DestroyMeshJob* m_destroyMeshJob;
    ProcessDestroyMeshJob* m_processDestroyMeshJob;
    ProcessHostMigrationJob* m_processHostMigrationJob;
    LeaveWithHostMigrationJob* m_leaveWithHostMigrationJob;
    RelayRouteManageJob* m_unknown24;
    KickoutManageJob* m_kickoutManageJob;
    transport::MissingStationHandler* m_unknown2c;
    MeshEventCallback m_meshEventCallback;
    JoiningMeshApprovalCallback m_joiningMeshApprovalCallback;
    HostCandidateRankingCallback m_hostCandidateRankingCallback;
    GetSessionIdCallback m_getSessionIdCallback;
    transport::ProtocolId m_meshProtocolId;
    MeshProtocol* m_meshProtocol;
    void* m_stationProtocol;
    transport::ProtocolId m_bandwidthCheckProtocolId;
    transport::BandwidthCheckProtocol* m_bandwidthCheckProtocol;
    unsigned short m_unknown54;
    unsigned char m_unknown56;
    StationIndex m_unknown58;
    StationIndex m_unknown5c;
    unsigned int m_meshState;
    unsigned int m_disconnectReason;
    unsigned int m_hostMigrationMode;
    unsigned int m_networkTopologyType;
    unsigned int m_bitRateCheckMode;
    bool m_hostMigrationEnabled;
    unsigned char m_unknown75;
    bool m_recentPlayRecordEnabled;
    unsigned char m_multiplayerPermitted;
    unsigned int m_connectedStationBitmap;
    unsigned int m_validStationBitmap;
    unsigned int m_unknown80;
    unsigned int m_unknown84;
    unsigned long long m_unknown88;
    unsigned short m_syncClockProtocolId;
    unsigned int m_unknown94;
    unsigned long long m_meshStartTime;
    bool m_meshStartTimeValid;
    unsigned char m_startedUp;
    bool m_hostMigrationStarted;
    unsigned char m_recentPlayRecordRequested;
    common::MonitoringDataSender* m_monitoringDataSender;
    common::CallContext m_callContext;
    unsigned int m_asyncProcess;
    MeshEventListener* m_meshEventListener;
    bool m_skipReport;
    bool m_skipCleanupReport;
    bool m_playersCountedAsParticipants;
    unsigned char m_localPlayerCount;
    unsigned int m_unknownc8;
    unsigned char m_unknowncc;
    unsigned char m_denyJoinRequestByInvalidState;
    unsigned char m_denyJoinRequestBySessionIsFull;
    transport::Station::PlayerInfo m_playerInfo[1];
    unsigned char m_playerInfoCount;
};

extern Mesh* g_piaMesh;

#if !defined(RECON_NATIVE)
#endif

}
}
}

#endif
