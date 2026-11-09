#ifndef NN_PIA_SESSION_PROCESSUPDATEMESHJOB_HPP
#define NN_PIA_SESSION_PROCESSUPDATEMESHJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace transport
{
class StationConnectionInfo;
}
}
}

namespace nn
{
namespace pia
{
namespace session
{

class ProcessUpdateMeshJob : public common::StepSequenceJob
{
public:
    enum
    {
        cStationNumMax = 32,
        cStartupResultStarted = 4
    };

    ProcessUpdateMeshJob();
    virtual ~ProcessUpdateMeshJob();
    virtual void Trace(unsigned long long value) const;

    int SetStationDataList(const unsigned char* data, unsigned int size);
    void CalcTimeLimit(bool relay);
    unsigned char GetContextIndex(StationIndex stationIndex) const;
    void ClearContextIndex(StationIndex stationIndex);
    unsigned char GetUsableContextIndex() const;
    void SetContextIndex(StationIndex stationIndex, unsigned char contextIndex);
    bool UpdateMesh();
    int Startup(const unsigned char* data, unsigned int size, bool relay);
    void UpdateJoinMeshFailureResult(StationIndex stationIndex, unsigned char failure, bool relay);
    ExecuteResult WaitDirectConnection();
    bool SendConnectionReport(StationIndex stationIndex, unsigned int version, unsigned int* stationBitmap);
    ExecuteResult SendConnectionReport();
    ExecuteResult StartRelayConnection();
    ExecuteResult CheckConnectionAll();
    ExecuteResult UpdateFailed();
    ExecuteResult WaitLeaveMesh();
    void CleanupInternalContexts();
    ExecuteResult CleanupByProcessFailure();
    void UpdateDataTakeover(unsigned short stationCount);
    int UpdateStationDataList(const unsigned char* data, unsigned int size, bool relay);
    void Cleanup();
    StationIndex GetStationIndexByPrincipalID(unsigned long long principalId) const;
    bool CheckEdmByStationIndex(StationIndex stationIndex) const;
    void SetConnectionFailureNotice(StationIndex stationIndex, unsigned char failure);
    void InitializeMonitoringDataVar();
    void SetMonitoringData();
    void ClearStationIndex(StationIndex stationIndex);
    void GetStepSequence() const;
    void CheckDuplicationDataInUpdateMeshDataList() const;

    common::Time m_updateDeadline;
    int m_updateTimeLimitMsec;
    int m_updateExtraTimeMsec;
    common::Time m_reportDeadline;
    int m_reportTimeLimitMsec;
    common::CallContext* m_connectContexts;
    unsigned char m_contextIndex[cStationNumMax];
    unsigned short m_stationNum;
    transport::StationConnectionInfo* m_stationConnectionInfos;
    StationIndex* m_stationIndices;
    transport::StationConnectionInfo* m_partConnectionInfos;
    StationIndex* m_partStationIndices;
    bool m_partReceived[2];
    unsigned short m_partStationCount;
    unsigned char m_partHostSlot;
    unsigned int m_partVersion;
    unsigned char m_partCount;
    unsigned short m_stationCount;
    unsigned char* m_failureCodes;
    common::Time m_failureNoticeTime;
    unsigned int m_reportCount;
    unsigned int m_relayRouteVersion;
    unsigned int m_updateVersion;
    bool m_started;
    bool m_relayRequested;
    bool m_leaveMeshOnFailure;
    unsigned int m_relayAttemptCount;
    bool m_updateMeshSucceeded;
    common::CallContext m_callContext;
};

}
}
}
#endif
