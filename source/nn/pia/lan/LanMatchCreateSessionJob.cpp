#include "nn/pia/lan/LanMatchCreateSessionJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/lan/LanCreateSessionSetting.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;

enum
{
    cResultCancelled = 0x6c05
};

inline void SelectStep(common::StepSequenceJob::Step& slot, ExecuteResult (LanMatchCreateSessionJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

LanMatchCreateSessionJob::LanMatchCreateSessionJob()
    : session::CreateSessionJob(),
      m_result(0),
      m_matchmakeSession(0),
      m_isOpenSession(true)
{
}

LanMatchCreateSessionJob::~LanMatchCreateSessionJob()
{
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::CreateMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &LanMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::lan::LanMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->CreateMatchmakeSession(&m_callContext);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
    }
    SelectStep(m_currentStep, &LanMatchCreateSessionJob::WaitCreateMatchmake,
               "LanMatchCreateSessionJob::WaitCreateMatchmake");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::WaitCreateMatchmake()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    unsigned int sessionId = 0;
    if (m_matchmakeSession->WaitCreateMatchmakeSession(&sessionId))
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = sessionId;
            SetMatchmakeStatus(session::Session::cMatchmakeStatusSearching);
            if (session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] == sessionId)
            {
                session::g_piaSession->SetCurrentSessionOpenStatus(
                    m_isOpenSession ? session::Session::cSessionOpenStatusOpen
                                    : session::Session::cSessionOpenStatusParticipationClosed);
            }
            else if (session::g_piaSession->GetJointSessionId() == sessionId)
            {
                session::g_piaSession->SetBufferSessionOpenStatus(
                    m_isOpenSession ? session::Session::cSessionOpenStatusOpen
                                    : session::Session::cSessionOpenStatusParticipationClosed);
            }
            SelectStep(m_currentStep, &LanMatchCreateSessionJob::MeshStartup, "CreateSessionJob::MeshStartup");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::OpenParticipate()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &LanMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::lan::LanMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = m_matchmakeSession->OpenParticipate(
        &m_callContext, session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchCreateSessionJob::WaitOpenParticipate,
               "LanMatchCreateSessionJob::WaitOpenParticipate");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::WaitOpenParticipate()
{
    if (m_matchmakeSession->WaitOpenParticipate())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            session::g_piaSession->m_meshLayerController->MeshCleanup();
            m_result = m_callContext.m_result;
            SelectStep(m_currentStep, &LanMatchCreateSessionJob::UnregisterGathering,
                       "LanMatchCreateSessionJob::UnregisterGathering");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &LanMatchCreateSessionJob::CompleteProcess, "CreateSessionJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::UnregisterGathering()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &LanMatchCreateSessionJob::CompleteFailure,
                   "nn::pia::lan::LanMatchCreateSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = m_matchmakeSession->UnregisterMatchmakeSession(
        &m_callContext, session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        m_callerContext->SignalFailure(m_result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchCreateSessionJob::WaitUnregisterGathering,
               "LanMatchCreateSessionJob::WaitUnregisterGathering");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::WaitUnregisterGathering()
{
    if (session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
            ->WaitUnregisterMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
            m_result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(m_result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LanMatchCreateSessionJob::Cleanup()
{
    session::CreateSessionJob::Cleanup();
    m_matchmakeSession = 0;
}

void LanMatchCreateSessionJob::Trace(unsigned long long) const
{
}

Result LanMatchCreateSessionJob::StartupImpl(const session::CreateSessionSetting* setting)
{
    session::Session* piaSession = session::g_piaSession;
    m_matchmakeSession = static_cast<LanMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return transport::cResultInvalidState;
    if (setting->GetMaxParticipantNum() == 0)
        return transport::cResultInvalidArgument;
    m_matchmakeSession->ReflectSettingToMatchmakeSession(setting);
    m_isOpenSession = static_cast<const LanCreateSessionSetting*>(setting)->IsOpenSession();
    session::g_piaSession->m_fieldc2[session::g_piaSession->m_currentSessionIndex] = setting->GetMaxParticipantNum();
    SelectStep(m_currentStep, &LanMatchCreateSessionJob::CreateMatchmakeSession,
               "LanMatchCreateSessionJob::CreateMatchmakeSession");
    return 0;
}

void LanMatchCreateSessionJob::SetNextStepAtWaitCreateMesh()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        SelectStep(m_currentStep, &LanMatchCreateSessionJob::CompleteFailure,
                   "LanMatchCreateSessionJob::CompleteFailure");
    }
    else
    {
        m_matchmakeSession->UpdateStationInfoList();
        m_matchmakeSession->CopySessionInfo();
        SelectStep(m_currentStep, &LanMatchCreateSessionJob::CompleteProcess, "CreateSessionJob::CompleteProcess");
    }
}

LanMatchCreateSessionJob::ExecuteResult LanMatchCreateSessionJob::SetNextStepAtFailedCreateMesh(Result result)
{
    m_result = result;
    SelectStep(m_currentStep, &LanMatchCreateSessionJob::UnregisterGathering,
               "LanMatchCreateSessionJob::UnregisterGathering");
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}
