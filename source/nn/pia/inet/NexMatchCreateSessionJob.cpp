#include "nn/pia/inet/NexMatchCreateSessionJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;

enum
{
    cResultRequestTimeout = 0x6c4b,
    cResultRequestFailed = 0xac5a,
    cResultNexError = 0xe437,
    cResultCancelled = 0x6c05,
    cResultNatServerAddressUnavailable = 0x6436,
    cResultPortDetectionFailed = 0x6434
};

enum
{
    cTraceFlagSession = 0x08000000,
    cMonitoringValueInvalid = 0xffff
};

enum
{
    cNotificationTimeoutMsec = 10000
};

inline void SelectStep(common::StepSequenceJob::Step& slot, ExecuteResult (NexMatchCreateSessionJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexMatchCreateSessionJob::NexMatchCreateSessionJob()
    : session::CreateSessionJob(),
      m_matchmakeSession(0),
      m_notificationDeadline(),
      m_isOpenSession(false),
      m_createMatchmakeStartTime()
{
}

NexMatchCreateSessionJob::~NexMatchCreateSessionJob()
{
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::LeaveJoinedMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_joinedSessionIdNum == 0)
    {
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CreateMatchmakeSession,
                   "NexMatchCreateSessionJob::CreateMatchmakeSession");
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
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_joinedSessionIdNum != 0)
            --m_joinedSessionIdNum;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::WaitLeaveJoinedMatchmakeSession,
               "NexMatchCreateSessionJob::WaitLeaveJoinedMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::WaitLeaveJoinedMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
            ->WaitLeaveMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure
            && m_callContext.m_result != cResultRequestTimeout
            && m_callContext.m_result == session::Session::cResultStatusDisconnectedNetwork)
            m_result = m_callContext.m_result;
        if (m_joinedSessionIdNum != 0)
            --m_joinedSessionIdNum;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::LeaveJoinedMatchmakeSession,
                   "NexMatchCreateSessionJob::LeaveJoinedMatchmakeSession");
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::CreateMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    m_createMatchmakeStartTime = common::g_piaJobScheduler->m_dispatchTime;
    static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)->CleanupEventHandler();
    const Result result = m_matchmakeSession->CreateMatchmakeSession(&m_callContext);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::WaitCreateMatchmake,
               "NexMatchCreateSessionJob::WaitCreateMatchmake");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::WaitCreateMatchmake()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    unsigned int sessionId = 0;
    if (m_matchmakeSession->WaitCreateMatchmakeSession(&sessionId))
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_result = m_callContext.m_result;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                       "NexMatchCreateSessionJob::UnregisterGathering");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = sessionId;
            SetMatchmakeStatus(session::Session::cMatchmakeStatusSearching);
            session::g_piaSession->SetCurrentSessionOpenStatus(
                m_isOpenSession ? session::Session::cSessionOpenStatusOpen
                                : session::Session::cSessionOpenStatusParticipationClosed);
            common::Time deadline;
            deadline.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * cNotificationTimeoutMsec
                              + common::g_piaJobScheduler->m_dispatchTime.m_tick;
            m_notificationDeadline = deadline;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::WaitNotification,
                       "NexMatchCreateSessionJob::WaitNotification");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::WaitNotification()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_result = cResultCancelled;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                   "NexMatchCreateSessionJob::UnregisterGathering");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_notificationDeadline < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_result = cResultRequestFailed;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                   "NexMatchCreateSessionJob::UnregisterGathering");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    nex::NgsBridgeInterface* ngsInterface = meshLayerController->GetNgsInterface(0);
    if (ngsInterface != 0
        && !meshLayerController->IsParticipated(
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex],
            ngsInterface->GetPrincipalID()))
    {
        meshLayerController->Trace(cTraceFlagSession);
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::StartNatSession, "NexMatchCreateSessionJob::StartNatSession");
    common::g_sessionBeginMonitoringContent.m_metric80 = static_cast<unsigned int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_createMatchmakeStartTime.m_tick)
        / common::TimeSpan::GetTicksPerMSec().m_tick);
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::StartNatSession()
{
    const Result result =
        static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)->StartNatSession();
    if (result != 0)
    {
        m_result = result;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                   "NexMatchCreateSessionJob::UnregisterGathering");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::WaitStartNatSession,
               "NexMatchCreateSessionJob::WaitStartNatSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::WaitStartNatSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                   "NexMatchCreateSessionJob::UnregisterGathering");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    Result result = 0;
    const ExecuteKind kind = static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                                 ->WaitStartNatSession(&result);
    if (kind == cExecuteKindContinue)
    {
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::MeshStartup, "CreateSessionJob::MeshStartup");
    }
    else if (kind == cExecuteKindFinished)
    {
        if (result == cResultNatServerAddressUnavailable || result == cResultPortDetectionFailed
            || result == cResultCancelled || result == transport::cResultInvalidState)
        {
            m_result = result;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                       "NexMatchCreateSessionJob::UnregisterGathering");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusDisconnectedNetwork)
        {
            SetConnectionStatus(session::Session::cConnectionStatusDisconnectedNetwork);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown6)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown4);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown7)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown5);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown8)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown6);
            m_result = result;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        m_result = transport::cResultInvalidState;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
                   "NexMatchCreateSessionJob::UnregisterGathering");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    return MakeExecuteResult(kind);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::UnregisterGathering()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::Session* piaSession = session::g_piaSession;
    const Result result = piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->UnregisterMatchmakeSession(
        &m_callContext, piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
    if (result == 0)
    {
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::WaitUnregisterGathering,
                   "NexMatchCreateSessionJob::WaitUnregisterGathering");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_result = result;
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure, "NexMatchCreateSessionJob::CompleteFailure");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::WaitUnregisterGathering()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::inet::NexMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
            ->WaitUnregisterMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            if (m_callContext.m_result == transport::cResultInvalidState)
                m_result = cResultRequestFailed;
            else if (m_callContext.m_result == cResultNexError)
                m_result = cResultNexError;
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteFailure,
                       "NexMatchCreateSessionJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchCreateSessionJob::Cleanup()
{
    session::CreateSessionJob::Cleanup();
    m_matchmakeSession = 0;
    m_isOpenSession = false;
}

void NexMatchCreateSessionJob::Trace(unsigned long long) const
{
}

Result NexMatchCreateSessionJob::StartupImpl(const session::CreateSessionSetting* setting)
{
    session::Session* piaSession = session::g_piaSession;
    if (piaSession->GetLocalPlayerCount() == 0)
        return transport::cResultInvalidState;
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return transport::cResultInvalidState;
    if (setting->GetMaxParticipantNum() == 0)
        return transport::cResultInvalidArgument;
    const NexCreateSessionSetting* nexSetting = static_cast<const NexCreateSessionSetting*>(setting);
    const unsigned short participantNum = setting->GetMaxParticipantNum() + nexSetting->GetExtraParticipantNum();
    if (transport::g_piaTransport->m_stationNum < participantNum)
        return transport::cResultInvalidArgument;
    m_matchmakeSession->ReflectCreateSetting(setting, piaSession->IsHostMigrationEnabled());
    m_isOpenSession = nexSetting->IsOpenSession();
    session::g_piaSession->m_fieldc2[session::g_piaSession->m_currentSessionIndex] = participantNum;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    m_joinedSessionIdNum = 0;
    for (unsigned int index = 0; index < meshLayerController->GetSessionIdArraySize(); ++index)
    {
        if (meshLayerController->GetSessionId(index) != 0)
            m_joinedSessionIds[m_joinedSessionIdNum++] = meshLayerController->GetSessionId(index);
    }
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::LeaveJoinedMatchmakeSession,
               "NexMatchCreateSessionJob::LeaveJoinedMatchmakeSession");
    common::g_sessionBeginMonitoringContent.m_metric137 = cMonitoringValueInvalid;
    common::g_sessionBeginMonitoringContent.m_metric138 = cMonitoringValueInvalid;
    return 0;
}

void NexMatchCreateSessionJob::SetNextStepAtWaitCreateMesh()
{
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::CompleteProcess, "CreateSessionJob::CompleteProcess");
}

NexMatchCreateSessionJob::ExecuteResult NexMatchCreateSessionJob::SetNextStepAtFailedCreateMesh(Result result)
{
    m_result = result;
    SelectStep(m_currentStep, &NexMatchCreateSessionJob::UnregisterGathering,
               "NexMatchCreateSessionJob::UnregisterGathering");
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}
