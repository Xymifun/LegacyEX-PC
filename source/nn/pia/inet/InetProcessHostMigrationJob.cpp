#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"

#include <coreinit/memory.h>

#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"

namespace
{

class NexNotificationEventHandler4Pia : public nn::nex::NotificationEventHandler
{
public:
    enum
    {
        cNotificationTypeDivisor = 1000,
        cNotificationTypeHostChanged = 110
    };

    explicit NexNotificationEventHandler4Pia(nn::pia::inet::InetProcessHostMigrationJob* job) : m_job(job) {}

    virtual ~NexNotificationEventHandler4Pia() {}

    virtual void ProcessNotificationEvent(const nn::nex::NotificationEvent& event)
    {
        if (event.GetType() / cNotificationTypeDivisor == cNotificationTypeHostChanged)
            m_job->HostChangeEventNotice(event.GetParam1());
    }

private:
    nn::pia::inet::InetProcessHostMigrationJob* m_job;
};

}

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cResultConnectionError = 0x6c50
};

enum
{
    cDecideNextHostBecomeHost = 1,
    cDecideNextHostWaitGreeting = 2
};

enum
{
    cRankDecisionWaitingGreeting = 1,
    cRankDecisionBecomingHost = 2
};

enum
{
    cSequenceManualHostMigration = 1,
    cSequenceAutomaticHostMigration = 2,
    cSequenceSendRankDecision = 3,
    cSequenceGetMatchMakingClientHost = 4,
    cSequenceUpdateSessionOpenStatus = 9
};

const int cReselectNewHostTimeMsec = 60000;
const int cMultiHostMigrationWaitTimeMsec = 15000;
const int cRelayRouteWaitExtensionMsec = 30000;
const int cOldHostDisconnectionCheckIntervalMsec = 6000;
const int cOldHostDisconnectionTimeoutMsec = 30000;
const int cUpdateSessionHostTimeoutMsec = 10000;
const int cBecomingHostMultiMarginMsec = 3000;
const int cMigrationFinishMarginMsec = 2000;
const int cRankDecisionWaitMarginMsec = 5000;
const int cRankDecisionWaitTimeMinMsec = 5000;
const unsigned short cCheckMatchMakingClientHostIntervalMsec = 100;

typedef common::Job::ExecuteResult (InetProcessHostMigrationJob::*StepFunction)();
typedef common::Job::ExecuteResult (common::StepSequenceJob::*SequenceStepFunction)();

inline SequenceStepFunction stepFunction(StepFunction function)
{
    return static_cast<SequenceStepFunction>(function);
}

inline void setStep(common::StepSequenceJob::Step& slot, const common::StepSequenceJob::Step& step)
{
    slot = step;
}
}

InetProcessHostMigrationJob::InetProcessHostMigrationJob()
    : session::ProcessHostMigrationJob(),
      m_callContext(),
      m_rankDecisionWaitTime(),
      m_reselectNewHostTime(),
      m_reselectNewHostTimeMsec(cReselectNewHostTimeMsec),
      m_oldHostDisconnectionCheckTime(),
      m_oldHostDisconnectionTimeout(),
      m_updateSessionHostTimeout()
{
    m_protocolCallContext = common::NewObj<nex::ProtocolCallContext>();
    if (m_unknownBC)
    {
        m_stationUrlList = common::NewObj<nex::qList<nex::StationURL> >();
        OSBlockSet(m_rankDecisionMessageId, 0, sizeof(m_rankDecisionMessageId));
        m_waitTimeMsec = cMultiHostMigrationWaitTimeMsec;
        void* buffer = pead::allocateFromSeadHeap(sizeof(NexNotificationEventHandler4Pia),
                                                  common::HeapManager::GetHeap());
        if (buffer == 0)
            m_notificationEventHandler = 0;
        else
            m_notificationEventHandler = ::new (buffer) NexNotificationEventHandler4Pia(this);
    }
    else
    {
        if (transport::g_piaTransport->m_relayRouteManager != 0)
            m_stationUrlList = common::NewObj<nex::qList<nex::StationURL> >();
        else
            m_stationUrlList = 0;
        m_notificationEventHandler = 0;
    }
    m_isHostChanged = false;
}

InetProcessHostMigrationJob::~InetProcessHostMigrationJob()
{
    if (m_protocolCallContext != 0)
    {
        if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
        common::DeleteObj(m_protocolCallContext);
        m_protocolCallContext = 0;
    }
    if (m_stationUrlList != 0)
    {
        m_stationUrlList->clear();
        common::DeleteObj(m_stationUrlList);
        m_stationUrlList = 0;
    }
    if (m_notificationEventHandler != 0)
    {
        NexFacade::GetInstance()->UnregisterNexNotificationEventHandler4Pia(m_notificationEventHandler);
        common::DeleteObj(m_notificationEventHandler);
        m_notificationEventHandler = 0;
    }
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::ManualHostMigration()
{
    m_hostMigrationSequence = cSequenceManualHostMigration;
    switch (DecideNextHostCommonProc())
    {
    case cDecideNextHostBecomeHost:
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHost),
                                    "InetProcessHostMigrationJob::InetPrepareForBecomingHost"));
        break;
    case cDecideNextHostWaitGreeting:
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                    "InetProcessHostMigrationJob::WaitNewHostGreeting"));
        break;
    default:
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        break;
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::AutomaticHostMigration()
{
    m_hostMigrationSequence = cSequenceAutomaticHostMigration;
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (session::g_piaMesh->GetHostStationIndex() != i
            && session::g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(i)))
        {
            m_newHostIndex = static_cast<StationIndex>(i);
            break;
        }
    }
    const unsigned int decision = DecideNextHostCommonProc();
    bool isRelayed = false;
    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    if (relayRouteManager != 0)
    {
        StationIndex relayStation;
        if (relayRouteManager->GetRelayRoute(m_oldHostIndex, m_newHostIndex, &relayStation) != 0)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (relayStation != m_newHostIndex)
        {
            const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
            m_waitDeadline.m_tick += ticksPerMSec.m_tick * cRelayRouteWaitExtensionMsec;
            isRelayed = true;
        }
    }
    if (decision == cDecideNextHostBecomeHost)
    {
        if (isRelayed)
        {
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetGetMatchMakingClientHost),
                                        "InetProcessHostMigrationJob::InetGetMatchMakingClientHost"));
            m_oldHostDisconnectionCheckTime.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                                                     + common::TimeSpan::GetTicksPerMSec().m_tick
                                                           * cOldHostDisconnectionCheckIntervalMsec;
            m_oldHostDisconnectionTimeout.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                                                   + common::TimeSpan::GetTicksPerMSec().m_tick
                                                         * cOldHostDisconnectionTimeoutMsec;
        }
        else
        {
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHost),
                                        "InetProcessHostMigrationJob::InetPrepareForBecomingHost"));
        }
    }
    else if (decision == cDecideNextHostWaitGreeting)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                    "InetProcessHostMigrationJob::WaitNewHostGreeting"));
    }
    else
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetPrepareForBecomingHost()
{
    if (!PreprocessForBecomingHostCommon())
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int gatheringId = NexFacade::GetInstance()->m_gatheringId;
    if (gatheringId == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (UpdateSessionHostAsync(gatheringId))
    {
        session::Session* piaSession = session::g_piaSession;
        if (piaSession != 0 && piaSession->GetStatus() == session::Session::cStatusBufferMatched)
        {
            piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->UpdateMatchmakeSessionHost(
                piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
        }
        const common::TimeSpan timeout = common::TimeSpan::FromMSec(cUpdateSessionHostTimeoutMsec);
        m_updateSessionHostTimeout.m_tick = timeout.m_tick + common::g_piaJobScheduler->m_dispatchTime.m_tick;
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::WaitAfterPrepareForBecomingHost),
                                    "InetProcessHostMigrationJob::WaitAfterPrepareForBecomingHost"));
    }
    else
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::WaitAfterPrepareForBecomingHost()
{
    if (!IsUpdateSessionHostCompleted())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (m_updateSessionHostTimeout <= common::g_piaJobScheduler->m_dispatchTime)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int gatheringId = NexFacade::GetInstance()->m_gatheringId;
    if (gatheringId == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    session::Session* piaSession = session::g_piaSession;
    if (piaSession != 0)
    {
        if (piaSession->GetStatus() == session::Session::cStatusBufferMatched)
        {
            if (gatheringId != piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0])
            {
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                            "InetProcessHostMigrationJob::HostMigrationFailure"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            if (!piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner()
                || !piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex == 0]->IsMatchmakeOwner())
            {
                return MakeExecuteResult(cExecuteKindNextStep);
            }
        }
        else if (piaSession->GetStatus() == session::Session::cStatusMatched)
        {
            if (gatheringId != piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            {
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                            "InetProcessHostMigrationJob::HostMigrationFailure"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            if (!piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
                return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    PostprocessForBecomingHostCommon();
    setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::SendGreetingMessage),
                                "InetProcessHostMigrationJob::SendGreetingMessage"));
    return MakeExecuteResult(cExecuteKindContinue);
}

int InetProcessHostMigrationJob::CalcTimeToWaitOtherClients(unsigned int rank, unsigned int candidateNum)
{
    if (rank == 0)
        return 0;
    const unsigned int halfNum = candidateNum >> 1;
    const unsigned int unitTime = m_rankDecisionWaitTimeMsec / (halfNum * (halfNum - 1) + halfNum + (candidateNum & 1));
    if (rank < halfNum)
    {
        int unitCount = 0;
        for (unsigned int i = 1; i <= rank; ++i)
            unitCount += (halfNum - i) * 2;
        return unitTime * unitCount;
    }
    return m_rankDecisionWaitTimeMsec - unitTime * (candidateNum - rank - 1);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetSendRankDecision()
{
    m_hostMigrationSequence = cSequenceSendRankDecision;
    session::Mesh* mesh = session::g_piaMesh;
    if (mesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const StationIndex localStationIndex = mesh->GetLocalStationIndex();
    if (localStationIndex > cStationIndexMax)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_unknownE0 == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    unsigned int rank = cStationNumMax;
    unsigned int candidateNum = 0;
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_candidateRanking[i] == static_cast<unsigned char>(localStationIndex))
            rank = i;
        if (m_candidateRanking[i] != cStationIndexInvalid)
            ++candidateNum;
    }
    const int waitTimeMsec = CalcTimeToWaitOtherClients(rank, candidateNum);
    m_rankDecisionWaitTime.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * waitTimeMsec
                                    + common::g_piaJobScheduler->m_dispatchTime.m_tick;
    m_waitDeadline.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * m_waitTimeMsec
                            + common::g_piaJobScheduler->m_dispatchTime.m_tick;
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (i != localStationIndex && i != m_oldHostIndex && mesh->IsValidStationIndex(static_cast<StationIndex>(i)))
        {
            unsigned int messageId = 0;
            if (mesh->GetMeshProtocol()->SendMultiMigrationRankDecision(static_cast<StationIndex>(i),
                                                                        m_waitDeadline.m_tick, &messageId))
            {
                m_rankDecisionMessageId[i] = messageId;
            }
        }
    }
    m_unknownA5 = true;
    m_greetingSenderPrincipalId = 0;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetWaitRankDecision),
                                "InetProcessHostMigrationJob::InetWaitRankDecision"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetWaitRankDecision()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_greetingSenderIndex != cStationIndexInvalid)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                    "InetProcessHostMigrationJob::WaitNewHostGreeting"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                        "InetProcessHostMigrationJob::WaitNewHostGreeting"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    if (m_rankDecisionWaitTime <= common::g_piaJobScheduler->m_dispatchTime)
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetGetMatchMakingClientHost),
                                    "InetProcessHostMigrationJob::InetGetMatchMakingClientHost"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetGetMatchMakingClientHost()
{
    m_hostMigrationSequence = cSequenceGetMatchMakingClientHost;
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int gatheringId = NexFacade::GetInstance()->m_gatheringId;
    if (gatheringId == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
        m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
    m_protocolCallContext->Reset();
    if (!NexFacade::GetInstance()->m_matchmakeExtensionClient->GetSessionURLs(m_protocolCallContext, gatheringId,
                                                                              m_stationUrlList))
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!m_unknownBC)
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetCheckMatchMakingClientHost),
                                    "InetProcessHostMigrationJob::InetCheckMatchMakingClientHost"));
    }
    else
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetWaitMatchMakingClientHost),
                                    "InetProcessHostMigrationJob::InetWaitMatchMakingClientHost"));
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetWaitMatchMakingClientHost()
{
    nex::ProtocolCallContext* protocolCallContext = m_protocolCallContext;
    if (protocolCallContext->GetState() == nex::CallContext::CallInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
        {
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_greetingSenderIndex != cStationIndexInvalid)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                    "InetProcessHostMigrationJob::WaitNewHostGreeting"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    bool isBecomingHost = false;
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                        "InetProcessHostMigrationJob::WaitNewHostGreeting"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_rankDecision[i] == cRankDecisionBecomingHost)
            isBecomingHost = true;
    }
    const nex::qResult result = protocolCallContext->GetResult();
    if (result)
    {
        const unsigned int principalId = m_stationUrlList->front().GetPrincipalID();
        if (principalId == 0)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        const StationIndex hostIndex = GetStationIndexByPrincipalID(principalId);
        if (hostIndex > cStationIndexMax)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (hostIndex == m_oldHostIndex)
        {
            if (!m_isRankingReceived && !isBecomingHost)
            {
                m_oldHostDisconnectionCheckTime.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                                                         + common::TimeSpan::GetTicksPerMSec().m_tick
                                                               * cOldHostDisconnectionCheckIntervalMsec;
                m_oldHostDisconnectionTimeout.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                                                       + common::TimeSpan::GetTicksPerMSec().m_tick
                                                             * cOldHostDisconnectionTimeoutMsec;
                setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetCheckOldHostDisconnection),
                                            "InetProcessHostMigrationJob::InetCheckOldHostDisconnection"));
                return MakeExecuteResult(cExecuteKindNextStep);
            }
        }
        else if (hostIndex != session::g_piaMesh->GetLocalStationIndex())
        {
            if (session::g_piaMesh->IsValidStationIndex(hostIndex))
            {
                m_reselectNewHostTime = common::g_piaJobScheduler->m_dispatchTime;
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                            "InetProcessHostMigrationJob::WaitNewHostGreeting"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    if (!isBecomingHost && relayRouteManager != 0)
    {
        StationIndex relayStation;
        if (relayRouteManager->GetRelayRoute(session::g_piaMesh->GetLocalStationIndex(), m_oldHostIndex, &relayStation)
                != 0
            || relayStation != m_oldHostIndex)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti),
                                "InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti"));
    return MakeExecuteResult(cExecuteKindContinue);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetCheckOldHostDisconnection()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_oldHostDisconnectionCheckTime <= common::g_piaJobScheduler->m_dispatchTime)
    {
        const unsigned int gatheringId = NexFacade::GetInstance()->m_gatheringId;
        if (gatheringId == 0)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
        m_protocolCallContext->Reset();
        if (!NexFacade::GetInstance()->m_matchmakeExtensionClient->GetSessionURLs(m_protocolCallContext, gatheringId,
                                                                                  m_stationUrlList))
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_unknownBC)
        {
            const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
            m_waitDeadline.m_tick += ticksPerMSec.m_tick * cOldHostDisconnectionCheckIntervalMsec;
        }
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetWaitCheckOldHostDisconnection),
                                    "InetProcessHostMigrationJob::InetWaitCheckOldHostDisconnection"));
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_unknownBC)
    {
        if (m_greetingSenderIndex != cStationIndexInvalid)
        {
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                        "InetProcessHostMigrationJob::WaitNewHostGreeting"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        bool isBecomingHost = false;
        for (int i = 0; i < cStationNumMax; ++i)
        {
            if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
            {
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                            "InetProcessHostMigrationJob::WaitNewHostGreeting"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            if (m_rankDecision[i] == cRankDecisionBecomingHost)
                isBecomingHost = true;
        }
        if (isBecomingHost)
        {
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti),
                                        "InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetWaitCheckOldHostDisconnection()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_unknownBC)
    {
        if (m_greetingSenderIndex != cStationIndexInvalid)
        {
            if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
                m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                        "InetProcessHostMigrationJob::WaitNewHostGreeting"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        bool isBecomingHost = false;
        for (int i = 0; i < cStationNumMax; ++i)
        {
            if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
            {
                if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
                    m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                            "InetProcessHostMigrationJob::WaitNewHostGreeting"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            if (m_rankDecision[i] == cRankDecisionBecomingHost)
                isBecomingHost = true;
        }
        if (isBecomingHost)
        {
            if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
                m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti),
                                        "InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
        return MakeExecuteResult(cExecuteKindNextStep);
    const nex::qResult result = m_protocolCallContext->GetResult();
    if (!result)
    {
        if (!m_unknownBC)
        {
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHost),
                                        "InetProcessHostMigrationJob::InetPrepareForBecomingHost"));
        }
        else
        {
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti),
                                        "InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti"));
        }
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int principalId = m_stationUrlList->front().GetPrincipalID();
    if (principalId != 0)
    {
        const StationIndex hostIndex = GetStationIndexByPrincipalID(principalId);
        if (hostIndex < cStationNumMax && m_oldHostIndex == hostIndex
            && common::g_piaJobScheduler->m_dispatchTime < m_oldHostDisconnectionTimeout
            && common::g_piaJobScheduler->m_dispatchTime < m_waitDeadline)
        {
            m_oldHostDisconnectionCheckTime.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                                                     + common::TimeSpan::GetTicksPerMSec().m_tick
                                                           * cOldHostDisconnectionCheckIntervalMsec;
            setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetCheckOldHostDisconnection),
                                        "InetProcessHostMigrationJob::InetCheckOldHostDisconnection"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                "InetProcessHostMigrationJob::HostMigrationFailure"));
    return MakeExecuteResult(cExecuteKindContinue);
}

void InetProcessHostMigrationJob::StopRankDecisionResending()
{
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_rankDecisionMessageId[i] != 0)
        {
            transport::g_piaResendingMessageManager->StopResending(m_rankDecisionMessageId[i]);
            m_rankDecisionMessageId[i] = 0;
        }
    }
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetPrepareForBecomingHostMulti()
{
    m_newHostIndex = session::g_piaMesh->GetLocalStationIndex();
    m_unknownA5 = false;
    StopRankDecisionResending();
    DisconnectStation(m_oldHostIndex);
    m_waitDeadline.m_tick -= common::TimeSpan::FromMSec(cBecomingHostMultiMarginMsec).m_tick;
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHost),
                                "InetProcessHostMigrationJob::InetPrepareForBecomingHost"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::UpdateSessionOpenStatus()
{
    m_hostMigrationSequence = cSequenceUpdateSessionOpenStatus;
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    NexFacade* facade = NexFacade::GetInstance();
    const unsigned int gatheringId = facade->m_gatheringId;
    if (gatheringId == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (session::g_piaSession == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationSuccess),
                                    "InetProcessHostMigrationJob::HostMigrationSuccess"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    nex::MatchmakeExtensionClient* matchmakeExtensionClient = facade->m_matchmakeExtensionClient;
    nex::MatchmakeSession* matchmakeSession = facade->m_matchmakeSession;
    if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
        m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
    m_protocolCallContext->Reset();
    matchmakeSession->Reset();
    if (!matchmakeExtensionClient->FindMatchmakeSessionByGatheringIdDetail(m_protocolCallContext, gatheringId,
                                                                           matchmakeSession))
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::WaitUpdateSessionOpenStatus),
                                "InetProcessHostMigrationJob::WaitUpdateSessionOpenStatus"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::WaitUpdateSessionOpenStatus()
{
    const nex::CallContext::State state = m_protocolCallContext->GetState();
    if (state == nex::CallContext::CallInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() != cResultConnectionError
            && common::g_piaJobScheduler->m_dispatchTime < m_waitDeadline)
        {
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    NexFacade* facade = NexFacade::GetInstance();
    nex::MatchmakeSession* matchmakeSession = facade->m_matchmakeSession;
    if (matchmakeSession == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (state == nex::CallContext::CallSucceeded)
    {
        const unsigned int gatheringId = matchmakeSession->m_uiValue1;
        if (gatheringId == facade->m_gatheringId)
        {
            const session::Session::SessionOpenStatus openStatus =
                matchmakeSession->m_bOpenParticipation ? session::Session::cSessionOpenStatusOpen
                                                       : session::Session::cSessionOpenStatusParticipationClosed;
            if (gatheringId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
            {
                if (session::g_piaSession != 0)
                    session::g_piaSession->SetCurrentSessionOpenStatus(openStatus);
            }
            else if (gatheringId
                     == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0])
            {
                if (session::g_piaSession != 0)
                    session::g_piaSession->SetBufferSessionOpenStatus(openStatus);
            }
            else
            {
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                            "InetProcessHostMigrationJob::HostMigrationFailure"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            if (session::g_piaSession->m_localStationId != matchmakeSession->m_uiValue3)
            {
                setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                            "InetProcessHostMigrationJob::HostMigrationFailure"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::SendMigrationFinish),
                                        "InetProcessHostMigrationJob::SendMigrationFinish"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                "InetProcessHostMigrationJob::HostMigrationFailure"));
    return MakeExecuteResult(cExecuteKindContinue);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetCheckMatchMakingClientHost()
{
    nex::ProtocolCallContext* protocolCallContext = m_protocolCallContext;
    if (protocolCallContext->GetState() == nex::CallContext::CallInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError
            || m_waitDeadline <= common::g_piaJobScheduler->m_dispatchTime)
        {
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindWait, cCheckMatchMakingClientHostIntervalMsec);
    }
    const nex::qResult result = protocolCallContext->GetResult();
    if (!result)
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetPrepareForBecomingHost),
                                    "InetProcessHostMigrationJob::InetPrepareForBecomingHost"));
    }
    else
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetCheckOldHostDisconnection),
                                    "InetProcessHostMigrationJob::InetCheckOldHostDisconnection"));
        const common::TimeSpan checkInterval = common::TimeSpan::FromMSec(cOldHostDisconnectionCheckIntervalMsec);
        m_oldHostDisconnectionCheckTime = common::g_piaJobScheduler->m_dispatchTime + checkInterval;
        const common::TimeSpan timeout = common::TimeSpan::FromMSec(cOldHostDisconnectionTimeoutMsec);
        m_oldHostDisconnectionTimeout = common::g_piaJobScheduler->m_dispatchTime + timeout;
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

void InetProcessHostMigrationJob::HostChangeEventNotice(unsigned int gatheringId)
{
    if (NexFacade::GetInstance()->m_gatheringId == gatheringId)
        m_isHostChanged = true;
}

unsigned int InetProcessHostMigrationJob::IsFatalErrorOccur()
{
    if (m_isHostChanged && session::g_piaSession == 0)
        return 1;
    return 0;
}

void InetProcessHostMigrationJob::CheckWhetherNewHostIsUnknown(unsigned long long principalId)
{
    if (principalId != m_greetingSenderPrincipalId && principalId != m_oldHostPrincipalId)
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetCheckMatchMakingClientHostIsUpdated),
                                    "InetProcessHostMigrationJob::InetCheckMatchMakingClientHostIsUpdated"));
    }
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetCheckMatchMakingClientHostIsUpdated()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int gatheringId = NexFacade::GetInstance()->m_gatheringId;
    if (gatheringId == 0)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
        m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
    m_protocolCallContext->Reset();
    if (!NexFacade::GetInstance()->m_matchmakeExtensionClient->GetSessionURLs(m_protocolCallContext, gatheringId,
                                                                              m_stationUrlList))
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetWaitMatchMakingClientHostIsUpdated),
                                "InetProcessHostMigrationJob::InetWaitMatchMakingClientHostIsUpdated"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetProcessHostMigrationJob::ExecuteResult InetProcessHostMigrationJob::InetWaitMatchMakingClientHostIsUpdated()
{
    nex::ProtocolCallContext* protocolCallContext = m_protocolCallContext;
    if (protocolCallContext->GetState() == nex::CallContext::CallInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
        {
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
            setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                        "InetProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_greetingSenderIndex != cStationIndexInvalid)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                    "InetProcessHostMigrationJob::WaitNewHostGreeting"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const nex::qResult result = protocolCallContext->GetResult();
    if (!result)
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                    "InetProcessHostMigrationJob::WaitNewHostGreeting"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else
    {
        setStep(m_currentStep, Step(stepFunction(&ProcessHostMigrationJob::HostMigrationFailure),
                                    "InetProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
}

unsigned int InetProcessHostMigrationJob::IsValidHostMigrationSetting()
{
    if ((m_unknownBC && NexFacade::GetInstance()->m_ngsBridge == 0) || NexFacade::GetInstance()->m_gatheringId == 0)
    {
        return 0;
    }

    if (m_unknownBC)
    {
        m_isHostChanged = false;
        NexFacade::GetInstance()->RegisterNexNotificationEventHandler4Pia(m_notificationEventHandler);
    }
    return 1;
}

unsigned int InetProcessHostMigrationJob::PreprocessForSendingMigrationFinish()
{
    m_waitDeadline = m_waitDeadline + common::TimeSpan::FromMSec(cMigrationFinishMarginMsec);
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::UpdateSessionOpenStatus),
                                "InetProcessHostMigrationJob::UpdateSessionOpenStatus"));
    return 1;
}

void InetProcessHostMigrationJob::Trace(unsigned long long) const
{
}

unsigned int InetProcessHostMigrationJob::StartupImpl(bool isManual, StationIndex newHost)
{
    if (isManual)
    {
        SetOldHostStation(session::g_piaMesh->GetHostStationIndex(), 0);
        m_newHostIndex = newHost;
        CleanupOldHostInfoCommonProc();
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::ManualHostMigration),
                                    "InetProcessHostMigrationJob::ManualHostMigration"));
    }
    else
    {
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::AutomaticHostMigration),
                                    "InetProcessHostMigrationJob::AutomaticHostMigration"));
    }
    return 1;
}

void InetProcessHostMigrationJob::CleanupImpl()
{
    if (m_unknownBC)
    {
        NexFacade::GetInstance()->UnregisterNexNotificationEventHandler4Pia(m_notificationEventHandler);
        m_isHostChanged = false;
    }
}

unsigned int InetProcessHostMigrationJob::UpdateSessionHostAsync(unsigned int gatheringId)
{
    unsigned int id = gatheringId;
    if (gatheringId == 0)
    {
        id = NexFacade::GetInstance()->m_gatheringId;
        if (id == 0)
            return 0;
    }
    if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
        m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
    m_protocolCallContext->Reset();
    if (!NexFacade::GetInstance()->m_matchmakeExtensionClient->UpdateSessionHost(m_protocolCallContext, id, true))
    {
        return 0;
    }
    return 1;
}

unsigned int InetProcessHostMigrationJob::IsUpdateSessionHostCompleted()
{
    if (m_protocolCallContext->GetState() == nex::CallContext::CallInProgress)
    {
        if (m_updateSessionHostTimeout <= common::g_piaJobScheduler->m_dispatchTime)
        {
            m_protocolCallContext->Cancel(nex::CallContext::CallCanceled);
            m_protocolCallContext->Reset();
            return 1;
        }
        return 0;
    }
    return 1;
}

unsigned int InetProcessHostMigrationJob::StartupMultiImpl(bool rankingReceived, unsigned short)
{
    int waitTimeMsec = m_waitTimeMsec - cRankDecisionWaitMarginMsec;
    if (waitTimeMsec < cRankDecisionWaitTimeMinMsec)
        waitTimeMsec = cRankDecisionWaitTimeMinMsec;
    m_rankDecisionWaitTimeMsec = waitTimeMsec;
    const common::TimeSpan reselectTime = common::TimeSpan::FromMSec(m_reselectNewHostTimeMsec);
    m_reselectNewHostTime = common::g_piaJobScheduler->m_dispatchTime + reselectTime;
    SetOldHostStation(session::g_piaMesh->GetHostStationIndex(), 0);
    if (rankingReceived)
    {
        CleanupOldHostInfoCommonProc();
        m_isRankingReceived = rankingReceived;
        setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetSendRankDecision),
                                    "InetProcessHostMigrationJob::InetSendRankDecision"));
        return 1;
    }
    MakeHostCandidateRanking(m_oldHostIndex, m_candidateRanking, &m_unknownE0, &m_unknownE4, false);
    m_isRankingReceived = rankingReceived;
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetSendRankDecision),
                                "InetProcessHostMigrationJob::InetSendRankDecision"));
    return 1;
}

void InetProcessHostMigrationJob::CleanupStatus()
{
    StopRankDecisionResending();
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    ProcessHostMigrationJob::CleanupStatus();
}

unsigned int InetProcessHostMigrationJob::CheckWhetherReselectNewHost()
{
    if (!m_unknownBC || m_reselectNewHostTime <= common::g_piaJobScheduler->m_dispatchTime)
    {
        return 0;
    }

    if (m_greetingSenderIndex != cStationIndexInvalid)
        SetOldHostStation(m_greetingSenderIndex, m_greetingSenderPrincipalId);
    m_unknownA5 = false;
    m_unknownB8 = false;
    m_finishReceived = false;
    m_finishSuccess = false;
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_greetingSenderPrincipalId = 0;
    ++m_unknown6e;
    setStep(m_currentStep, Step(stepFunction(&InetProcessHostMigrationJob::InetSendRankDecision),
                                "InetProcessHostMigrationJob::InetSendRankDecision"));
    StopRankDecisionResending();
    OSBlockSet(m_rankDecision, 0, sizeof(m_rankDecision));
    m_isRankingReceived = false;
    return 1;
}

}
}
}
