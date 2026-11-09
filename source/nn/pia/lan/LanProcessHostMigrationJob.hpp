#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_PROCESS_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_PROCESS_HOST_MIGRATION_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanProcessHostMigrationJob : public session::ProcessHostMigrationJob
{
public:
    LanProcessHostMigrationJob();
    virtual ~LanProcessHostMigrationJob();

    virtual void Trace(unsigned long long value) const;
    virtual unsigned int PreprocessForSendingMigrationFinish();
    virtual unsigned int IsFatalErrorOccur();
    virtual unsigned int IsValidHostMigrationSetting();
    virtual unsigned int StartupImpl(bool multi, StationIndex newHost);
    virtual void CleanupImpl();
    virtual void CleanupStatus();
    virtual unsigned int StartupMultiImpl(bool rankingReceived, unsigned short stationNum);
    virtual unsigned int CheckWhetherReselectNewHost();

    ExecuteResult LanCleanupOldHostInfo();
    ExecuteResult LanDecideNextHost();
    ExecuteResult LanPrepareForBecomingHost();
    ExecuteResult LanCleanupOldHostInfoOnMultiCandidate();
    ExecuteResult LanMakeHostCandidateRanking();
    int CalcTimeToWaitOtherClients(unsigned int rank, unsigned int candidateNum);
    ExecuteResult LanSendRankDecision();
    ExecuteResult LanWaitRankDecision();
    ExecuteResult LanGetMatchMakingClientHost();
    ExecuteResult LanWaitMatchMakingClientHost();
    ExecuteResult LanCheckOldHostDisconnection();
    ExecuteResult LanWaitCheckOldHostDisconnection();
    void StopRankDecisionResending();
    ExecuteResult LanPrepareForBecomingHostMulti();
    ExecuteResult LanGetMatchMakingClientHostLastConfirmation();
    ExecuteResult LanWaitMatchMakingClientHostLastConfirmation();
    ExecuteResult LanWaitSendUpdateSessionMessage();
    ExecuteResult LanCheckMatchMakingClientHost();

    common::CallContext m_callContext;
    common::Time m_rankDecisionWaitTime;
    unsigned int m_rankDecisionWaitTimeMsec;
    common::Time m_reselectNewHostTime;
    unsigned int m_rankDecisionMessageId[cStationNumMax];
    bool m_isRankingReceived;
    bool m_isFatalError;
    common::Time m_oldHostDisconnectionCheckTime;
    common::Time m_lastConfirmationTime;
};

}
}
}

#endif
