#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INET_PROCESS_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INET_PROCESS_HOST_MIGRATION_JOB_HPP

#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"

namespace nn
{
namespace nex
{
class NotificationEventHandler;
class ProtocolCallContext;
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class InetProcessHostMigrationJob : public session::ProcessHostMigrationJob
{
public:
    InetProcessHostMigrationJob();
    virtual ~InetProcessHostMigrationJob();

    virtual void Trace(unsigned long long value) const;
    virtual unsigned int PreprocessForSendingMigrationFinish();
    virtual unsigned int IsFatalErrorOccur();
    virtual void CheckWhetherNewHostIsUnknown(unsigned long long principalId);
    virtual unsigned int IsValidHostMigrationSetting();
    virtual unsigned int StartupImpl(bool multi, StationIndex newHost);
    virtual void CleanupImpl();
    virtual void CleanupStatus();
    virtual unsigned int StartupMultiImpl(bool rankingReceived, unsigned short stationNum);
    virtual unsigned int CheckWhetherReselectNewHost();
    virtual unsigned int UpdateSessionHostAsync(unsigned int gatheringId);
    virtual unsigned int IsUpdateSessionHostCompleted();

    ExecuteResult ManualHostMigration();
    ExecuteResult AutomaticHostMigration();
    ExecuteResult InetPrepareForBecomingHost();
    ExecuteResult WaitAfterPrepareForBecomingHost();
    int CalcTimeToWaitOtherClients(unsigned int rank, unsigned int candidateNum);
    ExecuteResult InetSendRankDecision();
    ExecuteResult InetWaitRankDecision();
    ExecuteResult InetGetMatchMakingClientHost();
    ExecuteResult InetWaitMatchMakingClientHost();
    ExecuteResult InetCheckOldHostDisconnection();
    ExecuteResult InetWaitCheckOldHostDisconnection();
    void StopRankDecisionResending();
    ExecuteResult InetPrepareForBecomingHostMulti();
    ExecuteResult UpdateSessionOpenStatus();
    ExecuteResult WaitUpdateSessionOpenStatus();
    ExecuteResult InetCheckMatchMakingClientHost();
    void HostChangeEventNotice(unsigned int gatheringId);
    ExecuteResult InetCheckMatchMakingClientHostIsUpdated();
    ExecuteResult InetWaitMatchMakingClientHostIsUpdated();

    nex::ProtocolCallContext* m_protocolCallContext;
    common::CallContext m_callContext;
    common::Time m_rankDecisionWaitTime;
    unsigned int m_rankDecisionWaitTimeMsec;
    common::Time m_reselectNewHostTime;
    int m_reselectNewHostTimeMsec;
    nex::qList<nex::StationURL>* m_stationUrlList;
    unsigned int m_rankDecisionMessageId[cStationNumMax];
    bool m_isRankingReceived;
    bool m_isHostChanged;
    nex::NotificationEventHandler* m_notificationEventHandler;
    common::Time m_oldHostDisconnectionCheckTime;
    common::Time m_oldHostDisconnectionTimeout;
    common::Time m_updateSessionHostTimeout;
};

}
}
}

#endif
