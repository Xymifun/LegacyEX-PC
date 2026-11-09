#include "nn/pia/inet/NexMatchJoinSessionJob.hpp"

#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexSessionInfo.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinSessionSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;
typedef ExecuteResult (session::JoinSessionJob::*JoinStep)();

enum
{
    cResultRequestFailed = 0xac5a,
    cResultNexError = 0xe437,
    cResultCancelled = 0x6c05,
    cResultNatServerAddressUnavailable = 0x6436,
    cResultPortDetectionFailed = 0x6434,
    cResultJoinMesh6443 = 0x6443,
    cResultJoinMesh6444 = 0x6444,
    cResultJoinMesh6445 = 0x6445,
    cResultJoinMesh643F = 0x643f
};

enum
{
    cJoinStateLeaveJoinedSession = 1,
    cJoinStateMatchmakeJoining = 2,
    cJoinStateNatSessionStarting = 3,
    cJoinStateRetryWait = 8
};

enum
{
    cJoinMeshStateNone = 0,
    cJoinMeshStateJoining = 3
};

enum
{
    cTraceFlagSession = 0x08000000,
    cMonitoringValueInvalid = 0xffff
};

enum
{
    cNotificationTimeoutMsec = 10000,
    cRetryTimeoutMsec = 30000,
    cRetryWaitMsec = 10000,
    cSessionInfoRetryWaitMsec = 60000,
    cJoinMeshRetryMax = 2
};

inline void SelectStep(common::StepSequenceJob::Step& slot, JoinStep function, const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

inline void CountRetry(unsigned char& counter)
{
    if (counter == 0xff)
        counter = 1;
    else
        ++counter;
}

inline common::Time GetTimeAfter(unsigned int msec)
{
    const common::TimeSpan span = common::TimeSpan::FromMSec(msec);
    return common::g_piaJobScheduler->m_dispatchTime + span;
}

inline NexMatchMeshLayerController* GetMeshLayerController()
{
    return static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController);
}
}

NexMatchJoinSessionJob::NexMatchJoinSessionJob()
    : session::JoinSessionJob(),
      m_retryTimeoutDeadline(),
      m_retryWaitDeadline(),
      m_notificationDeadline(),
      m_joinMatchmakeStartTime()
{
}

NexMatchJoinSessionJob::~NexMatchJoinSessionJob()
{
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::LeaveJoinedMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    m_joinState = cJoinStateLeaveJoinedSession;
    if (m_joinedSessionIdNum == 0)
    {
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::JoinMatchmakeSession),
                   "NexMatchJoinSessionJob::JoinMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int sessionId = m_joinedSessionIds[m_joinedSessionIdNum - 1];
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    m_callContext.Reset();
    const Result result = matchmakeSession->LeaveMatchmakeSession(&m_callContext, sessionId);
    if (result != 0)
    {
        if (result == cResultRequestFailed)
        {
            m_result = cResultRequestFailed;
            SelectStep(m_currentStep, &NexMatchJoinSessionJob::MeshCleanup, "NexMatchJoinSessionJob::MeshCleanup");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_joinedSessionIdNum != 0)
            --m_joinedSessionIdNum;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitLeaveJoinedMatchmakeSession),
               "NexMatchJoinSessionJob::WaitLeaveJoinedMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitLeaveJoinedMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
            ->WaitLeaveMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            if (m_callContext.m_result == session::Session::cResultStatusDisconnectedNetwork)
                m_result = m_callContext.m_result;
            else if (m_callContext.m_result == cResultNexError)
                m_result = cResultNexError;
        }
        if (m_joinedSessionIdNum != 0)
            --m_joinedSessionIdNum;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveJoinedMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveJoinedMatchmakeSession");
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::JoinMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (CheckContextCallCancelled())
    {
        m_result = cResultCancelled;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::MeshCleanup, "NexMatchJoinSessionJob::MeshCleanup");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_ownerPrincipalId = 0;
    m_joinMatchmakeStartTime = common::g_piaJobScheduler->m_dispatchTime;
    GetMeshLayerController()->CleanupEventHandler();
    m_isNgsInterfaceBound = false;
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    const Result result = matchmakeSession->JoinMatchmakeSession(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_result = result;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_joinState = cJoinStateMatchmakeJoining;
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitJoinMatchmake),
               "NexMatchJoinSessionJob::WaitJoinMatchmake");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitJoinMatchmake()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    if (matchmakeSession->WaitJoinMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_result = m_callContext.m_result;
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                       "NexMatchJoinSessionJob::LeaveMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = m_sessionId;
            SetMatchmakeStatus(session::Session::cMatchmakeStatusSearching);
            m_ownerPrincipalId = matchmakeSession->GetMatchmakeOwnerUniqueId();
            m_callContext.Reset();
            if (GetMeshLayerController()->GetNgsInterface(0) != 0)
                m_isNgsInterfaceBound = true;
            common::Time deadline;
            deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                              + common::TimeSpan::GetTicksPerMSec().m_tick * cNotificationTimeoutMsec;
            m_notificationDeadline = deadline;
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitNotification),
                       "NexMatchJoinSessionJob::WaitNotification");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitNotification()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (CheckContextCallCancelled())
    {
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_notificationDeadline < common::g_piaJobScheduler->m_dispatchTime)
    {
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        m_result = cResultRequestFailed;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    NexMatchMeshLayerController* meshLayerController = GetMeshLayerController();
    nex::NgsBridgeInterface* ngsInterface = meshLayerController->GetNgsInterface(0);
    if (ngsInterface != 0 && !meshLayerController->IsParticipated(m_sessionId, ngsInterface->GetPrincipalID()))
    {
        meshLayerController->Trace(cTraceFlagSession);
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::GetStationConnectionInfo),
               "NexMatchJoinSessionJob::GetStationConnectionInfo");
    const common::TimeSpan elapsed(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_joinMatchmakeStartTime.m_tick);
    common::g_sessionBeginMonitoringContent.m_metric80 = elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick;
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::GetStationConnectionInfo()
{
    if (CheckContextCallCancelled())
    {
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    const unsigned int sessionId = session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex];
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    transport::StationConnectionInfo stationConnectionInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(stationConnectionInfo);
    const Result result = matchmakeSession->GetStationConnectionInfo(&m_callContext, sessionId);
    if (result != 0)
    {
        if (result == session::Session::cResultStatusDisconnectedNetwork
            || result == session::Session::cResultStatusUnknown6 || result == cResultRequestFailed)
        {
            m_result = session::Session::cResultStatusUnknown6;
            SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                       "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        m_result = m_callContext.m_result;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitGetStationConnectionInfo),
               "NexMatchJoinSessionJob::WaitGetStationConnectionInfo");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitGetStationConnectionInfo()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (!session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
             ->WaitGetStationConnectionInfo(&m_stationConnectionInfo))
    {
        CheckContextCallCancelled();
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        m_result = m_callContext.m_result;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_callContext.m_state == common::CallContext::cStateSuccess)
    {
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::StartNatSession),
                   "NexMatchJoinSessionJob::StartNatSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callContext.m_state == common::CallContext::cStateCancelled)
    {
        m_result = cResultCancelled;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::StartNatSession()
{
    if (CheckContextCallCancelled())
    {
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    const Result result = GetMeshLayerController()->StartNatSession();
    if (result != 0)
    {
        m_result = result;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_joinState = cJoinStateNatSessionStarting;
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitStartNatSession),
               "NexMatchJoinSessionJob::WaitStartNatSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitStartNatSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (CheckContextCallCancelled())
        GetMeshLayerController()->CancelStartNatSession();
    Result result = 0;
    const ExecuteKind kind = GetMeshLayerController()->WaitStartNatSession(&result);
    if (kind == cExecuteKindContinue)
    {
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::MeshStartup, "JoinSessionJob::MeshStartup");
    }
    else if (kind == cExecuteKindFinished)
    {
        if (result == cResultNatServerAddressUnavailable || result == cResultPortDetectionFailed
            || result == cResultCancelled || result == transport::cResultInvalidState)
        {
            m_result = result;
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                       "NexMatchJoinSessionJob::LeaveMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusDisconnectedNetwork)
        {
            SetConnectionStatus(session::Session::cConnectionStatusDisconnectedNetwork);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                       "NexMatchJoinSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown6)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown4);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                       "NexMatchJoinSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown7)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown5);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                       "NexMatchJoinSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown8)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown6);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                       "NexMatchJoinSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        m_result = result;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    return MakeExecuteResult(kind);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::LeaveMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::Session* piaSession = session::g_piaSession;
    NexMatchMeshLayerController* meshLayerController = GetMeshLayerController();
    if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] != 0)
    {
        const unsigned int bufferIndex = piaSession->m_currentSessionIndex == 0;
        session::IMatchmakeSession* bufferSession = piaSession->m_matchmakeSession[bufferIndex];
        const unsigned int bufferSessionId = piaSession->m_sessionId[bufferIndex];
        m_callContext.Reset();
        const Result result = bufferSession->LeaveMatchmakeSession(&m_callContext, bufferSessionId);
        if (result == 0)
        {
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitLeaveBufferMatchmakeSession),
                       "NexMatchJoinSessionJob::WaitLeaveBufferMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == cResultRequestFailed)
            m_result = cResultRequestFailed;
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0] = 0;
    }
    if (meshLayerController != 0)
    {
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            const unsigned int sessionId = meshLayerController->GetSessionId(i);
            if (sessionId == 0)
                continue;
            if (!meshLayerController->IsParticipated(sessionId, NexFacade::GetInstance()->GetLocalPrincipalId()))
            {
                meshLayerController->ClearSessionId(sessionId);
                continue;
            }
            if (sessionId != session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
            {
                session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0] = sessionId;
                session::IMatchmakeSession* bufferSession =
                    session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0];
                m_callContext.Reset();
                const Result result = bufferSession->LeaveMatchmakeSession(&m_callContext, sessionId);
                if (result == 0)
                {
                    SelectStep(m_currentStep,
                               static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitLeaveBufferMatchmakeSession),
                               "NexMatchJoinSessionJob::WaitLeaveBufferMatchmakeSession");
                    return MakeExecuteResult(cExecuteKindNextStep);
                }
                if (result == cResultRequestFailed)
                    m_result = cResultRequestFailed;
                session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0] = 0;
            }
        }
    }
    if (session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] != 0)
    {
        const unsigned int currentIndex = session::g_piaSession->m_currentSessionIndex;
        session::IMatchmakeSession* currentSession = session::g_piaSession->m_matchmakeSession[currentIndex];
        const unsigned int currentSessionId = session::g_piaSession->m_sessionId[currentIndex];
        m_callContext.Reset();
        const Result result = currentSession->LeaveMatchmakeSession(&m_callContext, currentSessionId);
        if (result == 0)
        {
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitLeaveCurrentMatchmakeSession),
                       "NexMatchJoinSessionJob::WaitLeaveCurrentMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == cResultRequestFailed)
            m_result = cResultRequestFailed;
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
    }
    SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure, "NexMatchJoinSessionJob::CompleteFailure");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitLeaveCurrentMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
            ->WaitLeaveMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            if (m_callContext.m_result == session::Session::cResultStatusDisconnectedNetwork)
                m_result = m_callContext.m_result;
            else if (m_callContext.m_result == cResultNexError)
                m_result = cResultNexError;
        }
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitLeaveBufferMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0]
            ->WaitLeaveMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0] = 0;
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            if (m_callContext.m_result == session::Session::cResultStatusDisconnectedNetwork)
                m_result = m_callContext.m_result;
            else if (m_callContext.m_result == cResultNexError)
                m_result = cResultNexError;
        }
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::CleanupForRetryJoinMesh()
{
    CountRetry(common::g_sessionBeginMonitoringContent.m_joinRetryCount);
    session::Session* piaSession = session::g_piaSession;
    piaSession->m_postponeNotice = false;
    piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
    piaSession->m_meshLayerController->MeshCleanup();
    if (m_sessionInfoRetry)
    {
        m_retryTimeoutDeadline = GetTimeAfter(cRetryTimeoutMsec);
        if (m_ownerChanged)
            m_retryWaitDeadline = GetTimeAfter(cRetryWaitMsec);
        else
            m_retryWaitDeadline = GetTimeAfter(cSessionInfoRetryWaitMsec);
        m_isWaitingForRetry = true;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitForRetryJoinMesh),
                   "NexMatchJoinSessionJob::WaitForRetryJoinMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_retryReasonB || m_retryReasonA || m_ownerChanged)
    {
        m_retryTimeoutDeadline = GetTimeAfter(cRetryTimeoutMsec);
        m_retryWaitDeadline = GetTimeAfter(cRetryWaitMsec);
        m_isWaitingForRetry = true;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::WaitForRetryJoinMesh),
                   "NexMatchJoinSessionJob::WaitForRetryJoinMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_connectionInfoRetry)
    {
        m_stationConnectionInfo.SetStationConnectionInfo(transport::StationConnectionInfo());
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::GetStationConnectionInfo),
                   "NexMatchJoinSessionJob::GetStationConnectionInfo");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
               "NexMatchJoinSessionJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::WaitForRetryJoinMesh()
{
    switch (session::g_piaSession->GetStatus())
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    m_joinState = cJoinStateRetryWait;
    if (CheckContextCallCancelled())
    {
        m_isWaitingForRetry = false;
        m_result = cResultCancelled;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_retryWaitDeadline < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_stationConnectionInfo.SetStationConnectionInfo(transport::StationConnectionInfo());
        m_isWaitingForRetry = false;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::GetStationConnectionInfo),
                   "NexMatchJoinSessionJob::GetStationConnectionInfo");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_retryTimeoutDeadline < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_isWaitingForRetry = false;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_ownerPrincipalId != 0 && NexFacade::GetInstance()->GetLocalPrincipalId() == m_ownerPrincipalId)
    {
        m_isWaitingForRetry = false;
        SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
                   "NexMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchJoinSessionJob::NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId)
{
    if (m_sessionId != sessionId)
        return;
    m_ownerPrincipalId = ownerId;
    if (NexFacade::GetInstance()->GetLocalPrincipalId() == ownerId
        || (session::g_piaMesh->GetJoinMeshState() == cJoinMeshStateNone && !m_isWaitingForRetry))
    {
        m_sessionInvalidated = true;
        return;
    }
    if (session::g_piaMesh->IsHostMigrationEnabled()
        && (session::g_piaMesh->IsHostMigrationStarted() || session::g_piaMesh->IsProcessingHostMigration())
        && session::g_piaMesh->GetJoinMeshState() == cJoinMeshStateJoining)
        return;
    m_ownerChanged = true;
    if (m_isWaitingForRetry)
    {
        m_retryWaitDeadline = GetTimeAfter(cRetryWaitMsec);
        if (m_retryTimeoutDeadline < m_retryWaitDeadline)
            m_retryTimeoutDeadline = m_retryWaitDeadline;
    }
}

void NexMatchJoinSessionJob::NoticeUnregisterEvent(unsigned int sessionId)
{
    if (m_sessionId == sessionId)
        m_sessionInvalidated = true;
}

void NexMatchJoinSessionJob::Trace(unsigned long long) const
{
}

void NexMatchJoinSessionJob::SetNextStepAtWaitJoinMesh()
{
    session::Session* piaSession = session::g_piaSession;
    NexSessionInfo* sessionInfo = static_cast<NexSessionInfo*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->GetSessionInfo());
    sessionInfo->SetHostPrincipalId(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId());
    sessionInfo->SetCurrentParticipantNum(piaSession->GetMatchmakeSessionParticipantNum());
    SelectStep(m_currentStep, &NexMatchJoinSessionJob::CompleteProcess, "JoinSessionJob::CompleteProcess");
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::SetNextStepAtFailedJoinMesh(Result result)
{
    m_result = result;
    if (session::g_piaMesh->GetBitRateCheckMode() != 0)
    {
        transport::BandwidthCheckProtocol* bandwidthCheckProtocol = session::g_piaMesh->GetBandwidthCheckProtocol();
        if (bandwidthCheckProtocol->IsReady())
            bandwidthCheckProtocol->CancelSend();
    }
    ++m_retryCount;
    if (m_retryCount < cJoinMeshRetryMax && !m_sessionInvalidated)
    {
        if (m_ownerChanged)
        {
            CountRetry(common::g_sessionBeginMonitoringContent.m_ownerChangeRetryCount);
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::CleanupForRetryJoinMesh),
                       "NexMatchJoinSessionJob::CleanupForRetryJoinMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_sessionInfoRetry)
        {
            CountRetry(common::g_sessionBeginMonitoringContent.m_sessionInfoRetryCount);
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::CleanupForRetryJoinMesh),
                       "NexMatchJoinSessionJob::CleanupForRetryJoinMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_retryReasonB)
        {
            CountRetry(common::g_sessionBeginMonitoringContent.m_retryReasonBCount);
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::CleanupForRetryJoinMesh),
                       "NexMatchJoinSessionJob::CleanupForRetryJoinMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_retryReasonA)
        {
            CountRetry(common::g_sessionBeginMonitoringContent.m_retryReasonACount);
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::CleanupForRetryJoinMesh),
                       "NexMatchJoinSessionJob::CleanupForRetryJoinMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (!m_connectionInfoRetry
            && (result == cResultJoinMesh643F || result == cResultJoinMesh6445 || result == cResultJoinMesh6444
                || result == cResultJoinMesh6443))
        {
            m_connectionInfoRetry = true;
            CountRetry(common::g_sessionBeginMonitoringContent.m_connectionInfoRetryCount);
            SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::CleanupForRetryJoinMesh),
                       "NexMatchJoinSessionJob::CleanupForRetryJoinMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    session::Session* piaSession = session::g_piaSession;
    piaSession->m_postponeNotice = false;
    piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
    SelectStep(m_currentStep, &NexMatchJoinSessionJob::MeshCleanup, "NexMatchJoinSessionJob::MeshCleanup");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::SetNextStepAtWaitLeaveMesh()
{
    SelectStep(m_currentStep, &NexMatchJoinSessionJob::MeshCleanup, "NexMatchJoinSessionJob::MeshCleanup");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchJoinSessionJob::ExecuteResult NexMatchJoinSessionJob::SetNextStepAtMeshCleanup()
{
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveMatchmakeSession),
               "NexMatchJoinSessionJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

Result NexMatchJoinSessionJob::StartupImpl(const session::JoinSessionSetting* setting)
{
    session::Session* piaSession = session::g_piaSession;
    if (setting->GetSessionId() == 0)
        return transport::cResultInvalidArgument;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
    matchmakeSession->ReflectJoinSetting(setting);
    m_isWaitingForRetry = false;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    m_joinedSessionIdNum = 0;
    for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
    {
        if (meshLayerController->GetSessionId(i) != 0)
        {
            m_joinedSessionIds[m_joinedSessionIdNum] = meshLayerController->GetSessionId(i);
            ++m_joinedSessionIdNum;
        }
    }
    SelectStep(m_currentStep, static_cast<JoinStep>(&NexMatchJoinSessionJob::LeaveJoinedMatchmakeSession),
               "NexMatchJoinSessionJob::LeaveJoinedMatchmakeSession");
    common::g_sessionBeginMonitoringContent.m_metric137 = cMonitoringValueInvalid;
    common::g_sessionBeginMonitoringContent.m_metric138 = cMonitoringValueInvalid;
    return 0;
}

void NexMatchJoinSessionJob::CleanupImpl()
{
    m_ownerPrincipalId = 0;
    m_isWaitingForRetry = false;
}

}
}
}
