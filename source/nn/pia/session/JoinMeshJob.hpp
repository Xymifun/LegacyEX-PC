#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_MESH_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class JoinMeshJob : public common::StepSequenceJob
{
public:
    enum
    {
        cReceivedFlagNum = 3
    };

    JoinMeshJob();
    virtual ~JoinMeshJob();
    bool Startup(const transport::StationConnectionInfo&, common::CallContext*);
    ExecuteResult SetupLocalPlayerInfo();
    ExecuteResult SetupLocalStation();
    void CleanupStations();
    void CalcProcessingTimeOfJoin() const;
    void CheckRelayConnectionForMonitoring() const;
    unsigned int CheckContextCallCancelled();
    ExecuteResult StartConnectingToHost();
    unsigned int CheckTransportConnectionStatus();
    ExecuteResult WaitUntilConnectToHost();
    ExecuteResult SendJoinRequest();
    unsigned int CheckConnectionStateWithHostStation();
    ExecuteResult WaitRequestAck();
    ExecuteResult WaitJoinResponse();
    unsigned int UpdateLocalAndHostInformation(StationIndex, StationIndex);
    unsigned int SetStationDataList(const unsigned char*, unsigned int);
    ExecuteResult AnalyzeJoinResponse();
    ExecuteResult WaitAllConnection();
    ExecuteResult CompleteProcess();
    ExecuteResult SetupLocalPlayerInfoFailure();
    ExecuteResult StartDisconnectStations();
    ExecuteResult WaitDisconnectStations();
    ExecuteResult LeaveMeshWithHostMigration();
    ExecuteResult LeaveMesh();
    ExecuteResult WaitLeaveMesh();
    ExecuteResult CompleteCancelWithLeaveMesh();
    void Cleanup(Result);
    int GetJoinMeshState() const;
    virtual void Trace(unsigned long long) const;
    virtual unsigned int StartupImpl();
    virtual void CleanupImpl();
    virtual void SetHostInfoToMonitoringData(const transport::StationConnectionInfo&);
    virtual void SetNextStepAtSetupLocalPlayerInfoImpl();
    virtual ExecuteResult SetNextStepAfterWaitAllConnection();
    void CleanupInternalContexts();

    common::CallContext* m_callerContext;
    common::CallContext* m_internalContext;
    unsigned int m_requestAckId;
    unsigned short m_stationCount;
    unsigned short m_stationCapacity;
    transport::StationConnectionInfo* m_stationInfos;
    unsigned int* m_stationIndices;
    bool m_waitingJoinResponse;
    bool m_received[cReceivedFlagNum];
    unsigned char m_responseResult;
    unsigned short m_responseStationCount;
    unsigned char m_responseHostSlot;
    unsigned char m_responseLocalSlot;
    unsigned char m_responsePartCount;
    unsigned int m_lastUpdateCount;
    unsigned char* m_stationConnected;
    unsigned int m_joinState;
    unsigned int m_joinResult;
    unsigned char m_reserved74[4];
    unsigned long long m_principalId;
    common::Time m_startTime;
    common::Time m_disconnectDeadline;
    common::CallContext m_callContext;
};

}
}
}
#endif
