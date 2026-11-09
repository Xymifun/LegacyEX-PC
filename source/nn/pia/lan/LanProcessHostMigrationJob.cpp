#include "nn/pia/lan/LanProcessHostMigrationJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/Assert.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/lan/LanSessionInfo.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cResultConnectionError = 0x6c50
};

enum
{
    cRankDecisionWaitingGreeting = 1,
    cRankDecisionBecomingHost = 2
};

enum
{
    cDecideNextHostBecomeHost = 1,
    cDecideNextHostWaitGreeting = 2
};

enum
{
    cSequenceCleanupOldHostInfo = 1,
    cSequenceDecideNextHost = 2,
    cSequenceSendRankDecision = 3,
    cSequenceGetMatchMakingClientHost = 4,
    cSequencePrepareForBecomingHost = 5,
    cSequenceLastConfirmation = 9
};

const int cRelayWaitMarginMsec = 2000;
const int cOldHostDisconnectionCheckDelayMsec = 2000;
const int cLastConfirmationDelayMsec = 2000;
const int cBecomingHostMultiMarginMsec = 1000;
const int cRankWaitStepMsec = 3000;
const int cRankWaitBaseMsec = 1000;
const int cMultiWaitStepMsec = 3000;
const int cMultiWaitBaseMsec = 2500;
const unsigned short cPollIntervalMsec = 100;
const unsigned long long cInvalidPrincipalId = ~0ULL;

inline void setStep(common::StepSequenceJob::Step& slot, const common::StepSequenceJob::Step& step)
{
    slot = step;
}

inline LanMatchmakeSession* GetCurrentMatchmakeSession()
{
    return static_cast<LanMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
}

inline long long GetTicksFromMSec(int msec)
{
    return common::TimeSpan::GetTicksPerMSec().m_tick * msec;
}

inline const common::Time& GetDispatchTime()
{
    return common::g_piaJobScheduler->m_dispatchTime;
}
}

LanProcessHostMigrationJob::LanProcessHostMigrationJob()
    : session::ProcessHostMigrationJob(), m_callContext(), m_rankDecisionWaitTime(), m_reselectNewHostTime(),
      m_oldHostDisconnectionCheckTime(), m_lastConfirmationTime()
{
    if (m_unknownBC)
        OSBlockSet(m_rankDecisionMessageId, 0, sizeof(m_rankDecisionMessageId));
    m_isFatalError = false;
}

LanProcessHostMigrationJob::~LanProcessHostMigrationJob()
{
    PIA_ASSERT(m_callContext.m_state != common::CallContext::cStateInProgress);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanCleanupOldHostInfo()
{
    m_hostMigrationSequence = cSequenceCleanupOldHostInfo;
    if (!CleanupOldHostInfoCommonProc())
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    switch (DecideNextHostCommonProc())
    {
    case cDecideNextHostBecomeHost:
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHost),
                             "LanProcessHostMigrationJob::LanPrepareForBecomingHost");
        break;
    case cDecideNextHostWaitGreeting:
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                             "LanProcessHostMigrationJob::WaitNewHostGreeting");
        break;
    default:
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        break;
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanDecideNextHost()
{
    m_hostMigrationSequence = cSequenceDecideNextHost;
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (session::g_piaMesh->GetHostStationIndex() != i &&
            session::g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(i)))
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
            setStep(m_currentStep,
                    Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                         "LanProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (relayStation != m_newHostIndex)
        {
            m_waitDeadline = m_waitDeadline + common::TimeSpan(GetTicksFromMSec(cRelayWaitMarginMsec));
            isRelayed = true;
        }
    }
    switch (decision)
    {
    case cDecideNextHostBecomeHost:
        if (isRelayed)
        {
            m_oldHostDisconnectionCheckTime.m_tick = GetDispatchTime().m_tick + GetTicksFromMSec(cOldHostDisconnectionCheckDelayMsec);
            setStep(m_currentStep,
                    Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanGetMatchMakingClientHost),
                         "LanProcessHostMigrationJob::LanGetMatchMakingClientHost"));
        }
        else
        {
            setStep(m_currentStep,
                    Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHost),
                         "LanProcessHostMigrationJob::LanPrepareForBecomingHost"));
        }
        break;
    case cDecideNextHostWaitGreeting:
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                     "LanProcessHostMigrationJob::WaitNewHostGreeting"));
        break;
    default:
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        break;
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanPrepareForBecomingHost()
{
    m_hostMigrationSequence = cSequencePrepareForBecomingHost;
    if (!PreprocessForBecomingHostCommon())
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    PostprocessForBecomingHostCommon();
    m_callContext.Reset();
    LanFacade::GetInstance()->GetLanProtocol()->RequestUpdateHostMessage();
    setStep(m_currentStep,
            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::SendGreetingMessage),
                 "LanProcessHostMigrationJob::SendGreetingMessage"));
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanCleanupOldHostInfoOnMultiCandidate()
{
    if (!CleanupOldHostInfoCommonProc())
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(m_currentStep,
            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanSendRankDecision),
                 "LanProcessHostMigrationJob::LanSendRankDecision"));
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanMakeHostCandidateRanking()
{
    if (MakeHostCandidateRanking(m_oldHostIndex, m_candidateRanking, &m_unknownE0, &m_unknownE4, false) != 0)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(m_currentStep,
            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanSendRankDecision),
                 "LanProcessHostMigrationJob::LanSendRankDecision"));
    return MakeExecuteResult(cExecuteKindContinue);
}

int LanProcessHostMigrationJob::CalcTimeToWaitOtherClients(unsigned int rank, unsigned int)
{
    return rank * cRankWaitStepMsec + cRankWaitBaseMsec;
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanSendRankDecision()
{
    m_hostMigrationSequence = cSequenceSendRankDecision;
    session::Mesh* mesh = session::g_piaMesh;
    if (mesh->CheckConnectionError() == cResultConnectionError)
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const StationIndex localStation = mesh->GetLocalStationIndex();
    if (localStation > cStationIndexMax)
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_unknownE0 == 0)
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    unsigned int rank = cStationNumMax;
    unsigned int candidateNum = 0;
    for (unsigned int i = 0; i < cStationNumMax; ++i)
    {
        if (m_candidateRanking[i] == static_cast<unsigned char>(localStation))
            rank = i;
        if (m_candidateRanking[i] != cStationIndexInvalid)
            ++candidateNum;
    }
    const int waitTimeMsec = CalcTimeToWaitOtherClients(rank, candidateNum);
    m_rankDecisionWaitTime.m_tick = GetDispatchTime().m_tick + GetTicksFromMSec(waitTimeMsec);
    const int timeoutMsec = m_waitTimeMsec;
    m_waitDeadline.m_tick = GetDispatchTime().m_tick + GetTicksFromMSec(timeoutMsec);
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (i != localStation && mesh->IsValidStationIndex(static_cast<StationIndex>(i)))
        {
            unsigned int ackId = 0;
            if (mesh->GetMeshProtocol()->SendMultiMigrationRankDecision(static_cast<StationIndex>(i),
                                                                       m_waitDeadline.m_tick, &ackId))
                m_rankDecisionMessageId[i] = ackId;
        }
    }
    m_unknownA5 = true;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanWaitRankDecision),
                         "LanProcessHostMigrationJob::LanWaitRankDecision");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanWaitRankDecision()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_waitDeadline <= GetDispatchTime())
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_greetingSenderIndex != cStationIndexInvalid)
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                             "LanProcessHostMigrationJob::WaitNewHostGreeting");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
        {
            m_currentStep.m_name = "LanProcessHostMigrationJob::WaitNewHostGreeting";
            m_currentStep.m_function = static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting);
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    if (m_rankDecisionWaitTime <= GetDispatchTime())
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanGetMatchMakingClientHost),
                             "LanProcessHostMigrationJob::LanGetMatchMakingClientHost");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanGetMatchMakingClientHost()
{
    m_hostMigrationSequence = cSequenceGetMatchMakingClientHost;
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_waitDeadline <= GetDispatchTime())
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    m_callContext.Reset();
    Result inquiryResult = matchmakeSession->InquiryOwnerPrincipalId(&m_callContext, matchmakeSession->GetGatheringId());
    if (inquiryResult != 0)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
    }
    else if (m_unknownBC)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanWaitMatchMakingClientHost),
                     "LanProcessHostMigrationJob::LanWaitMatchMakingClientHost"));
    }
    else
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanCheckMatchMakingClientHost),
                     "LanProcessHostMigrationJob::LanCheckMatchMakingClientHost"));
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanWaitMatchMakingClientHost()
{
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    if (!matchmakeSession->WaitInquiryOwnerPrincipalId() ||
        m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() != cResultConnectionError &&
            GetDispatchTime() < m_waitDeadline)
            return MakeExecuteResult(cExecuteKindNextStep);
        matchmakeSession->CancelInquiryOwnerPrincipalId();
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_greetingSenderIndex != cStationIndexInvalid)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                     "LanProcessHostMigrationJob::WaitNewHostGreeting"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    bool isBecomingHostDecided = false;
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
        {
            setStep(m_currentStep,
                    Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                         "LanProcessHostMigrationJob::WaitNewHostGreeting"));
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_rankDecision[i] == cRankDecisionBecomingHost)
            isBecomingHostDecided = true;
    }
    bool isOwnerAbsent = false;
    if (matchmakeSession->GetMatchmakeOwnerUniqueId() == cInvalidPrincipalId)
    {
        isOwnerAbsent = true;
    }
    else
    {
        const StationIndex ownerStation = GetStationIndexByPrincipalID(matchmakeSession->GetMatchmakeOwnerUniqueId());
        if (ownerStation <= cStationIndexMax && ownerStation == m_oldHostIndex)
        {
            if (!m_isRankingReceived && !isBecomingHostDecided)
            {
                m_oldHostDisconnectionCheckTime.m_tick =
                    GetDispatchTime().m_tick + GetTicksFromMSec(cOldHostDisconnectionCheckDelayMsec);
                setStep(m_currentStep,
                        Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanCheckOldHostDisconnection),
                             "LanProcessHostMigrationJob::LanCheckOldHostDisconnection"));
                return MakeExecuteResult(cExecuteKindNextStep);
            }
            isOwnerAbsent = true;
        }
    }
    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    bool canBecomeHost = isOwnerAbsent;
    if (canBecomeHost && !isBecomingHostDecided && relayRouteManager != 0)
    {
        StationIndex relayStation;
        if (relayRouteManager->GetRelayRoute(session::g_piaMesh->GetLocalStationIndex(), m_oldHostIndex,
                                             &relayStation) != 0 ||
            relayStation != m_oldHostIndex)
            canBecomeHost = false;
    }
    if (canBecomeHost)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti),
                     "LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti"));
    }
    else
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanCheckOldHostDisconnection()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!(m_oldHostDisconnectionCheckTime <= GetDispatchTime()))
    {
        if (m_unknownBC)
        {
            if (m_greetingSenderIndex != cStationIndexInvalid)
            {
                setStep(m_currentStep,
                        Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                             "LanProcessHostMigrationJob::WaitNewHostGreeting"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
            bool isBecomingHostDecided = false;
            for (int i = 0; i < cStationNumMax; ++i)
            {
                if (m_rankDecision[i] == cRankDecisionWaitingGreeting)
                {
                    setStep(m_currentStep,
                            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::WaitNewHostGreeting),
                                 "LanProcessHostMigrationJob::WaitNewHostGreeting"));
                    return MakeExecuteResult(cExecuteKindContinue);
                }
                if (m_rankDecision[i] == cRankDecisionBecomingHost)
                    isBecomingHostDecided = true;
            }
            if (isBecomingHostDecided)
            {
                setStep(m_currentStep,
                        Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti),
                             "LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti"));
                return MakeExecuteResult(cExecuteKindContinue);
            }
        }
        return MakeExecuteResult(cExecuteKindWait, cPollIntervalMsec);
    }
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    m_callContext.Reset();
    Result inquiryResult = matchmakeSession->InquiryOwnerPrincipalId(&m_callContext, matchmakeSession->GetGatheringId());
    if (inquiryResult != 0)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
    }
    else
    {
        if (m_unknownBC)
            m_waitDeadline.m_tick += GetTicksFromMSec(cOldHostDisconnectionCheckDelayMsec);
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanWaitCheckOldHostDisconnection),
                     "LanProcessHostMigrationJob::LanWaitCheckOldHostDisconnection"));
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanWaitCheckOldHostDisconnection()
{
    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError ||
        m_waitDeadline <= GetDispatchTime())
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    if (!matchmakeSession->WaitInquiryOwnerPrincipalId() ||
        m_callContext.m_state == common::CallContext::cStateInProgress)
        return MakeExecuteResult(cExecuteKindWait, cPollIntervalMsec);
    if (matchmakeSession->GetMatchmakeOwnerUniqueId() == cInvalidPrincipalId)
    {
        if (m_unknownBC)
        {
            m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti),
                                 "LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti");
        }
        else
        {
            m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHost),
                                 "LanProcessHostMigrationJob::LanPrepareForBecomingHost");
        }
    }
    else
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                             "LanProcessHostMigrationJob::HostMigrationFailure");
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

void LanProcessHostMigrationJob::StopRankDecisionResending()
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

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti()
{
    m_newHostIndex = session::g_piaMesh->GetLocalStationIndex();
    m_unknownA5 = false;
    StopRankDecisionResending();
    DisconnectStation(m_oldHostIndex);
    m_waitDeadline.m_tick -= GetTicksFromMSec(cBecomingHostMultiMarginMsec);
    m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHost),
                         "LanProcessHostMigrationJob::LanPrepareForBecomingHost");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanGetMatchMakingClientHostLastConfirmation()
{
    m_hostMigrationSequence = cSequenceLastConfirmation;
    Result connectionError = session::g_piaMesh->CheckConnectionError();
    if (connectionError == cResultConnectionError)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "ProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (GetDispatchTime() < m_lastConfirmationTime)
        return MakeExecuteResult(cExecuteKindNextStep);
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    m_callContext.Reset();
    Result inquiryResult = matchmakeSession->InquiryOwnerPrincipalId(&m_callContext, matchmakeSession->GetGatheringId());
    if (inquiryResult != 0)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
    }
    else
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanWaitMatchMakingClientHostLastConfirmation),
                     "LanProcessHostMigrationJob::LanWaitMatchMakingClientHostLastConfirmation"));
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanWaitMatchMakingClientHostLastConfirmation()
{
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    if (!matchmakeSession->WaitInquiryOwnerPrincipalId() ||
        m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError ||
            m_waitDeadline <= GetDispatchTime())
        {
            matchmakeSession->CancelInquiryOwnerPrincipalId();
            setStep(m_currentStep,
                    Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                         "LanProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (matchmakeSession->GetMatchmakeOwnerUniqueId() != cInvalidPrincipalId &&
        matchmakeSession->GetMatchmakeOwnerUniqueId() != LanFacade::GetInstance()->GetLocalPrincipalId())
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    matchmakeSession->SetMatchmakeOwnerUniqueId(LanFacade::GetInstance()->GetLocalPrincipalId());
    matchmakeSession->SetMatchmakeOwner(true);
    matchmakeSession->UpdateStationInfoList();
    if (m_waitDeadline <= GetDispatchTime())
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                     "LanProcessHostMigrationJob::HostMigrationFailure"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const StationIndex ownerStation = static_cast<StationIndex>(
        transport::g_stationConnectionInfoTable->GetStationIndexByPrincipalID(
            matchmakeSession->GetMatchmakeOwnerUniqueId()));
    if (ownerStation <= cStationIndexMax && ownerStation == session::g_piaMesh->GetLocalStationIndex())
    {
        m_callContext.Reset();
        LanFacade::GetInstance()->GetLanProtocol()->RequestUpdateSessionMessage();
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanWaitSendUpdateSessionMessage),
                     "LanProcessHostMigrationJob::LanWaitSendUpdateSessionMessage"));
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(m_currentStep,
            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                 "LanProcessHostMigrationJob::HostMigrationFailure"));
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanWaitSendUpdateSessionMessage()
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsUpdateSessionMessageCompleted())
    {
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError ||
            m_waitDeadline <= GetDispatchTime())
        {
            m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                                 "LanProcessHostMigrationJob::HostMigrationFailure");
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    session::Session::SessionOpenStatus openStatus = session::Session::cSessionOpenStatusParticipationClosed;
    if (GetCurrentMatchmakeSession()->GetSessionInfoPtr()->IsOpened())
        openStatus = session::Session::cSessionOpenStatusOpen;
    if (session::g_piaSession != 0)
        session::g_piaSession->SetCurrentSessionOpenStatus(openStatus);
    m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::SendMigrationFinish),
                         "ProcessHostMigrationJob::SendMigrationFinish");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanProcessHostMigrationJob::ExecuteResult LanProcessHostMigrationJob::LanCheckMatchMakingClientHost()
{
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    if (!matchmakeSession->WaitInquiryOwnerPrincipalId() ||
        m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError ||
            m_waitDeadline <= GetDispatchTime())
        {
            matchmakeSession->CancelInquiryOwnerPrincipalId();
            setStep(m_currentStep,
                    Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&ProcessHostMigrationJob::HostMigrationFailure),
                         "LanProcessHostMigrationJob::HostMigrationFailure"));
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        return MakeExecuteResult(cExecuteKindWait, cPollIntervalMsec);
    }
    if (matchmakeSession->GetMatchmakeOwnerUniqueId() == cInvalidPrincipalId)
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanPrepareForBecomingHost),
                     "LanProcessHostMigrationJob::LanPrepareForBecomingHost"));
    }
    else
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanCheckOldHostDisconnection),
                     "LanProcessHostMigrationJob::LanCheckOldHostDisconnection"));
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

unsigned int LanProcessHostMigrationJob::IsFatalErrorOccur()
{
    return m_isFatalError;
}

unsigned int LanProcessHostMigrationJob::PreprocessForSendingMigrationFinish()
{
    const common::TimeSpan confirmationDelay = common::TimeSpan::FromMSec(cLastConfirmationDelayMsec);
    m_lastConfirmationTime = GetDispatchTime() + confirmationDelay;
    setStep(m_currentStep,
            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanGetMatchMakingClientHostLastConfirmation),
                 "LanProcessHostMigrationJob::LanGetMatchMakingClientHostLastConfirmation"));
    return 1;
}

void LanProcessHostMigrationJob::Trace(unsigned long long) const
{
}

unsigned int LanProcessHostMigrationJob::StartupImpl(bool multi, StationIndex newHost)
{
    if (multi)
    {
        m_oldHostIndex = session::g_piaMesh->GetHostStationIndex();
        m_newHostIndex = newHost;
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanCleanupOldHostInfo),
                     "LanProcessHostMigrationJob::LanCleanupOldHostInfo"));
    }
    else
    {
        setStep(m_currentStep,
                Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanDecideNextHost),
                     "LanProcessHostMigrationJob::LanDecideNextHost"));
    }
    return 1;
}

void LanProcessHostMigrationJob::CleanupImpl()
{
    if (m_unknownBC)
        m_isFatalError = false;
}

unsigned int LanProcessHostMigrationJob::StartupMultiImpl(bool rankingReceived, unsigned short stationNum)
{
    m_waitTimeMsec = stationNum * cMultiWaitStepMsec + cMultiWaitBaseMsec;
    const int reselectTimeMsec = m_waitTimeMsec * 2;
    const common::TimeSpan reselectTime = common::TimeSpan::FromMSec(reselectTimeMsec);
    m_reselectNewHostTime = GetDispatchTime() + reselectTime;
    m_oldHostIndex = session::g_piaMesh->GetHostStationIndex();
    if (rankingReceived)
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanCleanupOldHostInfoOnMultiCandidate),
                             "LanProcessHostMigrationJob::LanCleanupOldHostInfoOnMultiCandidate");
        m_isRankingReceived = rankingReceived;
    }
    else
    {
        m_currentStep = Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanMakeHostCandidateRanking),
                             "LanProcessHostMigrationJob::LanMakeHostCandidateRanking");
        m_isRankingReceived = rankingReceived;
    }
    return 1;
}

void LanProcessHostMigrationJob::CleanupStatus()
{
    StopRankDecisionResending();
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
        if (matchmakeSession != 0 && !matchmakeSession->WaitInquiryOwnerPrincipalId())
            matchmakeSession->CancelInquiryOwnerPrincipalId();
        else
            m_callContext.SignalCancel();
    }
    ProcessHostMigrationJob::CleanupStatus();
}

unsigned int LanProcessHostMigrationJob::CheckWhetherReselectNewHost()
{
    if (!m_unknownBC || m_reselectNewHostTime <= GetDispatchTime())
        return 0;
    if (m_greetingSenderIndex != cStationIndexInvalid)
        m_oldHostIndex = m_greetingSenderIndex;
    setStep(m_currentStep,
            Step(static_cast<ExecuteResult (StepSequenceJob::*)()>(&LanProcessHostMigrationJob::LanSendRankDecision),
                 "LanProcessHostMigrationJob::LanSendRankDecision"));
    StopRankDecisionResending();
    OSBlockSet(m_rankDecision, 0, sizeof(m_rankDecision));
    m_isRankingReceived = false;
    return 1;
}

unsigned int LanProcessHostMigrationJob::IsValidHostMigrationSetting()
{
    return 1;
}

}}}
