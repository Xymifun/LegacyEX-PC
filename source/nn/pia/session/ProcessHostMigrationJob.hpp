#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_PROCESS_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_PROCESS_HOST_MIGRATION_JOB_HPP

#include <coreinit/memory.h>

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class ProcessHostMigrationJob : public common::StepSequenceJob
{
public:
    enum
    {
        cStationNumMax = 32,
        cStationIndexMax = 31,
        cStationIndexInvalid = 0xfd
    };

    ProcessHostMigrationJob();
    virtual ~ProcessHostMigrationJob();

    virtual void Trace(unsigned long long value) const;
    virtual unsigned int CleanupOldHostInfoCommonProc();
    virtual unsigned int PreprocessForSendingMigrationFinish() = 0;
    virtual unsigned int IsFatalErrorOccur() { return 0; }
    virtual void CheckWhetherNewHostIsUnknown(unsigned long long) {}
    virtual unsigned int IsValidHostMigrationSetting() = 0;
    virtual unsigned int StartupImpl(bool multi, StationIndex station) = 0;
    virtual void CleanupImpl() {}
    virtual void CleanupStatus();
    virtual void DisconnectStation(StationIndex station);
    virtual unsigned int StartupMultiImpl(bool multi, unsigned short count);
    virtual unsigned int CheckWhetherReselectNewHost() { return 0; }

    void ProcessingAtEndHM(bool stationCountValid);
    void SearchRefugeRelayRoute();
    void ProcessingAtStartHM();
    bool Startup(bool multi, StationIndex station);
    void SetOldHostStation(StationIndex station, unsigned long long principalId);
    unsigned int DecideNextHostCommonProc();
    unsigned int PreprocessForBecomingHostCommon();
    void PostprocessForBecomingHostCommon();
    ExecuteResult SendGreetingMessage();
    ExecuteResult SendMigrationFinish();
    ExecuteResult WaitNewHostGreeting();
    ExecuteResult WaitGreetingResponse();
    ExecuteResult WaitNewHostFinished();
    ExecuteResult HostMigrationSuccess();
    ExecuteResult HostMigrationFailure();
    void ReceiveGreetingResponse(StationIndex station);
    void ReceiveGreeting(StationIndex station);
    void ReceiveFinish(bool success);
    void Cleanup();
    bool StartupMulti(bool multi, const unsigned char* ranking, unsigned int a, unsigned int b);
    Result MakeHostCandidateRanking(StationIndex station, unsigned char* ranking, unsigned int* meshValue,
                                    unsigned int* relayValue, bool fromConnectionInfo);
    Result CheckHostCandidateRanking(StationIndex first, StationIndex second, bool* firstIsHigher,
                                     unsigned int* valueE0, unsigned int* valueE4) const;
    void SetRankDecision(StationIndex station, bool value);
    void InitializeMonitoringDataVar();
    void SetMonitoringData();
    StationIndex GetStationIndexByPrincipalID(unsigned long long principalId);

    unsigned int GetHostMigrationSequence() const { return m_hostMigrationSequence; }

    void SetHostCandidateRanking(const unsigned char* ranking, unsigned int count, unsigned int valueE0,
                                 unsigned int valueE4)
    {
        OSBlockMove(m_candidateRanking, ranking, count, FALSE);
        m_unknownE0 = valueE0;
        m_unknownE4 = valueE4;
    }

    common::Time m_waitDeadline;
    int m_waitTimeMsec;
    common::Time m_greetingDeadline;
    common::Time m_startTime;
    common::TimeSpan m_elapsedTime;
    unsigned int m_hostMigrationCount;
    unsigned char m_stationCountAtStart;
    unsigned char m_stationCountAtEnd;
    unsigned char m_unknown6e;
    bool m_started;
    bool m_restartRequested;
    StationIndex m_oldHostIndex;
    unsigned long long m_oldHostPrincipalId;
    StationIndex m_newHostIndex;
    unsigned char m_greetingPending[cStationNumMax];
    bool m_greetingSent;
    bool m_unknownA5;
    StationIndex m_greetingSenderIndex;
    unsigned long long m_greetingSenderPrincipalId;
    bool m_unknownB8;
    bool m_finishReceived;
    bool m_finishSuccess;
    bool m_unknownBB;
    bool m_unknownBC;
    unsigned char m_candidateRanking[cStationNumMax];
    unsigned int m_unknownE0;
    unsigned int m_unknownE4;
    unsigned char m_rankDecision[cStationNumMax];
    unsigned int m_hostMigrationSequence;
};

}
}
}

#endif
