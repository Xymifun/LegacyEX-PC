#include "nn/pia/inet/NexMatchLeaveSessionJob.hpp"

#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;
typedef ExecuteResult (session::LeaveSessionJob::*LeaveStep)();

enum
{
    cWaitHostMigrationTimeoutMsec = 15000,
    cMigrationCandidateMax = 31
};

inline void SelectStep(common::StepSequenceJob::Step& slot, LeaveStep function, const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexMatchLeaveSessionJob::NexMatchLeaveSessionJob() : session::LeaveSessionJob()
{
    m_sessionIdNum = 0;
    for (unsigned char i = 0; i < cSessionIdMax; ++i)
    {
        m_sessionIds[i] = 0;
        m_isSessionInfoRequested[i] = false;
    }
}

NexMatchLeaveSessionJob::~NexMatchLeaveSessionJob()
{
}

void NexMatchLeaveSessionJob::SetNextStepAtWaitForcedTerminatingOfJointSessionJob()
{
    m_sessionIdNum = session::g_piaSession->m_stationIdStatusTable->GetSessionIdArray(m_sessionIds);
    for (unsigned char i = 0; i < m_sessionIdNum; ++i)
        m_isSessionInfoRequested[i] = false;
    SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::GetMatchmakeSessionOwners),
               "NexMatchLeaveSessionJob::GetMatchmakeSessionOwners");
}

void NexMatchLeaveSessionJob::SetNextStepAtWaitHostMigratedTimeout()
{
    SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::MigrateMatchmakeSessionOwner),
               "NexMatchLeaveSessionJob::MigrateMatchmakeSessionOwner");
}

void NexMatchLeaveSessionJob::ChooseNextStepByCheckingBufferMatchmakeSessions(const common::StepSequenceJob::Step& step)
{
    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] != 0)
    {
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveBufferMatchmakeSession,
                   "LeaveSessionJob::LeaveBufferMatchmakeSession");
        return;
    }
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
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
                SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveBufferMatchmakeSession,
                           "LeaveSessionJob::LeaveBufferMatchmakeSession");
                return;
            }
        }
    }
    m_currentStep = step;
}

void NexMatchLeaveSessionJob::SetNextStepAtWaitLeaveCurrentMatchmakeSession()
{
    const common::StepSequenceJob::Step step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchLeaveSessionJob::SendMonitoringData),
        "LeaveSessionJob::SendMonitoringData");
    ChooseNextStepByCheckingBufferMatchmakeSessions(step);
}

void NexMatchLeaveSessionJob::SetNextStepAtLeaveCurrentMatchmakeSessionFailure()
{
    const common::StepSequenceJob::Step step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchLeaveSessionJob::SendMonitoringData),
        "LeaveSessionJob::SendMonitoringData");
    ChooseNextStepByCheckingBufferMatchmakeSessions(step);
}

void NexMatchLeaveSessionJob::SetNextStepAtWaitLeaveBufferMatchmakeSession()
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                                 &NexMatchLeaveSessionJob::LeaveCurrentMatchmakeSession),
                                             "LeaveSessionJob::LeaveCurrentMatchmakeSession");
    ChooseNextStepByCheckingBufferMatchmakeSessions(step);
}

void NexMatchLeaveSessionJob::SetNextStepAtMeshCleanup()
{
    SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::LeaveMatchmakeSessionCompanions),
               "NexMatchLeaveSessionJob::LeaveMatchmakeSessionCompanions");
}

bool NexMatchLeaveSessionJob::IsNeedSkipLeaveCurrentMatchmakeSession()
{
    return session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] == 0;
}

NexMatchLeaveSessionJob::ExecuteResult NexMatchLeaveSessionJob::LeaveMatchmakeSessionCompanions()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveMesh,
                   "nn::pia::inet::NexMatchLeaveSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                                 &NexMatchLeaveSessionJob::LeaveCurrentMatchmakeSession),
                                             "LeaveSessionJob::LeaveCurrentMatchmakeSession");
    ChooseNextStepByCheckingBufferMatchmakeSessions(step);
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchLeaveSessionJob::ExecuteResult NexMatchLeaveSessionJob::WaitLeaveMatchmakeSessionCompanions()
{
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    switch (session::g_piaSession->GetStatus())
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveMesh,
                   "nn::pia::inet::NexMatchLeaveSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (!matchmakeSession->WaitLeaveMatchmakeSessionCompanion())
        return MakeExecuteResult(cExecuteKindNextStep);
    SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::LeaveMatchmakeSessionCompanions),
               "NexMatchLeaveSessionJob::LeaveMatchmakeSessionCompanions");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchLeaveSessionJob::ExecuteResult NexMatchLeaveSessionJob::GetMatchmakeSessionOwners()
{
    session::Session* piaSession = session::g_piaSession;
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveMesh,
                   "nn::pia::inet::NexMatchLeaveSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    for (unsigned char i = 0; i < m_sessionIdNum; ++i)
    {
        if (m_sessionIds[i] == piaSession->m_sessionId[piaSession->m_currentSessionIndex]
            || m_isSessionInfoRequested[i])
            continue;
        m_isSessionInfoRequested[i] = true;
        NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
            piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
        if (matchmakeSession->RequestSessionInfoWithoutPermission(&m_callContext, m_sessionIds[i]) == 0)
        {
            SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::WaitGetMatchmakeSessionOwners),
                       "NexMatchLeaveSessionJob::WaitGetMatchmakeSessionOwners");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveMesh, "LeaveSessionJob::LeaveMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchLeaveSessionJob::ExecuteResult NexMatchLeaveSessionJob::WaitGetMatchmakeSessionOwners()
{
    session::Session* piaSession = session::g_piaSession;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
    switch (session::g_piaSession->GetStatus())
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveMesh,
                   "nn::pia::inet::NexMatchLeaveSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (!matchmakeSession->WaitRequestSessionInfoWithoutPermission())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (m_callContext.m_result != 0)
        SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::GetMatchmakeSessionOwners),
                   "NexMatchLeaveSessionJob::GetMatchmakeSessionOwners");
    piaSession->m_stationIdStatusTable->SetSessionHost(matchmakeSession->GetMatchmakeOwnerUniqueId(), true);
    SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::GetMatchmakeSessionOwners),
               "NexMatchLeaveSessionJob::GetMatchmakeSessionOwners");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchLeaveSessionJob::ExecuteResult NexMatchLeaveSessionJob::MigrateMatchmakeSessionOwner()
{
    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] == 0)
    {
        SetNextStepAtWaitLeaveMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (piaSession->m_matchmakeStatus != session::Session::cStatusDisconnectedSession
        && piaSession->m_matchmakeStatus != session::Session::cStatusBufferMatched)
    {
        SetNextStepAtWaitLeaveMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    unsigned long long candidateIds[cMigrationCandidateMax];
    unsigned short candidateNum = 0;
    const common::ListNode* end = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    session::StationIdStatusTable* statusTable = piaSession->m_stationIdStatusTable;
    for (const common::ListNode* node = end->next; node != end; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        unsigned int sessionId;
        if (statusTable->GetSessionId(entry->stationId, &sessionId)
            && sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            candidateIds[candidateNum++] = entry->stationId;
    }
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex == 0]);
    if (matchmakeSession->MigrateMatchmakeSessionOwner(
            &m_callContext, piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0], candidateIds, candidateNum)
        == 0)
    {
        SelectStep(m_currentStep, static_cast<LeaveStep>(&NexMatchLeaveSessionJob::WaitMigrateMatchmakeSessionOwner),
                   "NexMatchLeaveSessionJob::WaitMigrateMatchmakeSessionOwner");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SetNextStepAtWaitLeaveMesh();
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchLeaveSessionJob::ExecuteResult NexMatchLeaveSessionJob::WaitMigrateMatchmakeSessionOwner()
{
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0]);
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::LeaveMesh,
                   "nn::pia::inet::NexMatchLeaveSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchLeaveSessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchLeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (matchmakeSession->WaitMigrateMatchmakeSessionOwner())
    {
        SetNextStepAtWaitLeaveMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchLeaveSessionJob::Trace(unsigned long long) const
{
}

void NexMatchLeaveSessionJob::CleanupImpl()
{
    m_sessionIdNum = 0;
    for (unsigned char i = 0; i < cSessionIdMax; ++i)
    {
        m_sessionIds[i] = 0;
        m_isSessionInfoRequested[i] = false;
    }
}

unsigned int NexMatchLeaveSessionJob::GetWaitHostMigrationTimeout() const
{
    return cWaitHostMigrationTimeoutMsec;
}

}
}
}
