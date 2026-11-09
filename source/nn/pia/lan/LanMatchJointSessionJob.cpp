#include "nn/pia/lan/LanMatchJointSessionJob.hpp"


namespace nn
{
namespace pia
{
namespace lan
{
namespace
{
typedef common::Job::ExecuteResult (LanMatchJointSessionJob::*Step)();

enum LanMatchResult
{
    cResultSessionDisconnected = 0xc406,
    cResultNotSupported = 0xc0b,
    cResultInvalidArgument = 0x10c07
};

enum
{
    cSessionStatusDisconnected = 5
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

LanMatchJointSessionJob::LanMatchJointSessionJob() : session::JointSessionJob(), m_stationConnectionInfo()
{
}

LanMatchJointSessionJob::~LanMatchJointSessionJob()
{
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::CallSessionEvent()
{
    NoticeStartingEventByProcessType();
    session::g_piaSession->m_postponeNotice = true;
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LanMatchJointSessionJob::WaitNextSessionId),
             "LanMatchJointSessionJob::WaitNextSessionId"));
    switch (m_processType)
    {
    case session::Session::cProcessTypeJoinJointSession:
        if (m_field9c != 0)
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &LanMatchJointSessionJob::StartJoinMatchmakeSession),
                                "LanMatchJointSessionJob::StartJoinMatchmakeSession"));
        }
        break;
    case session::Session::cProcessTypeJoinRandomJointSession:
        if (m_field9c != 0)
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &LanMatchJointSessionJob::StartRandomMatchmake),
                                "LanMatchJointSessionJob::StartRandomMatchmake"));
        }
        break;
    case session::Session::cProcessTypeDestroyJointSession:
    case session::Session::cProcessTypeLeaveJointSession:
        if (m_field9c == 0)
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &LanMatchJointSessionJob::SendAnswerToInvitation),
                                "LanMatchJointSessionJob::SendAnswerToInvitation"));
        }
        else
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &LanMatchJointSessionJob::SendInvitationAsCompanion),
                                "LanMatchJointSessionJob::SendInvitationAsCompanion"));
        }
        break;
    default:
        break;
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendInvitationAsCompanion()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitForAnswerToInvitation,
               "LanMatchJointSessionJob::WaitForAnswerToInvitation");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitForAnswerToInvitation()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartRandomMatchmake()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitRandomMatchmake,
               "LanMatchJointSessionJob::WaitRandomMatchmake");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitRandomMatchmake()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::SendNextSessionId,
               "LanMatchJointSessionJob::SendNextSessionId");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartJoinMatchmakeSession()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitJoinMatchmakeSession,
               "LanMatchJointSessionJob::WaitJoinMatchmakeSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitJoinMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::SendNextSessionId,
               "LanMatchJointSessionJob::SendNextSessionId");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendNextSessionId()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCompanionStationPrepared,
               "LanMatchJointSessionJob::WaitCompanionStationPrepared");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitCompanionStationPrepared()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartLeavePreviousMesh,
               "LanMatchJointSessionJob::StartLeavePreviousMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendAnswerToInvitation()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitNextSessionId,
               "LanMatchJointSessionJob::WaitNextSessionId");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitNextSessionId()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartUpdateSessionKey,
               "LanMatchJointSessionJob::StartUpdateSessionKey");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartUpdateSessionKey()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitUpdateSessionKey,
               "LanMatchJointSessionJob::WaitUpdateSessionKey");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitUpdateSessionKey()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::SendPreparedForMigrateSession,
               "LanMatchJointSessionJob::SendPreparedForMigrateSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendPreparedForMigrateSession()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh,
               "LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartLeavePreviousMesh,
               "LanMatchJointSessionJob::StartLeavePreviousMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartLeavePreviousMesh()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitLeavePreviousMesh,
               "LanMatchJointSessionJob::WaitLeavePreviousMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitLeavePreviousMesh()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession,
               "LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession,
               "LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::MeshRestart, "LanMatchJointSessionJob::MeshRestart");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::MeshRestart()
{
    if (m_field9d != 0)
    {
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LanMatchJointSessionJob::StartCreateNextMesh),
            "LanMatchJointSessionJob::StartCreateNextMesh"));
    }
    else
    {
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo),
                            "LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo"));
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartCreateNextMesh()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCreateNextMesh,
               "LanMatchJointSessionJob::WaitCreateNextMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitCreateNextMesh()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCompanionStation,
               "LanMatchJointSessionJob::WaitCompanionStation");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo,
               "LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartJoinNextMesh,
               "LanMatchJointSessionJob::StartJoinNextMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartJoinNextMesh()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitJoinNextMesh, "LanMatchJointSessionJob::WaitJoinNextMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitJoinNextMesh()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCompanionStation,
               "LanMatchJointSessionJob::WaitCompanionStation");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitCompanionStation()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitHostStationId,
               "LanMatchJointSessionJob::WaitHostStationId");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitHostStationId()
{
    if (session::g_piaSession->GetStatus() == cSessionStatusDisconnected)
    {
        ClearProcessTypeAndNoticeFailureEvent();
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchJointSessionJob::ProcessComplete, "LanMatchJointSessionJob::ProcessComplete");
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::ProcessComplete()
{
    ClearProcessTypeAndNoticeEndingEvent();
    session::Session* session = session::g_piaSession;
    session->m_postponeNotice = false;
    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    return MakeExecuteResult(cExecuteKindFinished);
}

void LanMatchJointSessionJob::Trace(unsigned long long value) const
{
}

Result LanMatchJointSessionJob::StartupJoinRandomJointSessionImpl(const session::JoinRandomSessionSetting* setting)
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupCreateJointSessionImpl(const session::CreateSessionSetting* setting)
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupJoinJointSessionImpl(const session::JoinSessionSetting* setting)
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupLeaveJointSessionImpl()
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupDestroyJointSessionImpl()
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupPassiveProcessImpl(session::Session::ProcessType processType,
                                                          session::SessionProtocol::MessageType messageType,
                                                          const unsigned long long* stationIds,
                                                          unsigned short stationIdNum,
                                                          unsigned long long senderStationId)
{
    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();

    const unsigned long long localStationId = session::g_piaSession->m_localStationId;
    bool senderFound = false;
    bool localFound = false;
    if (stationIdNum != 0)
    {
        do
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
            {
                const unsigned long long stationId = *stationIds;
                *companionStationId = stationId;
                if (stationId == senderStationId)
                    senderFound = true;
                if (stationId == localStationId)
                    localFound = true;
            }
            --stationIdNum;
            ++stationIds;
        } while (stationIdNum != 0);
    }

    if (!senderFound)
        return cResultInvalidArgument;
    if (!localFound)
        return cResultInvalidArgument;
    m_field98 = 0;
    SelectStep(m_currentStep, &LanMatchJointSessionJob::CallSessionEvent, "LanMatchJointSessionJob::CallSessionEvent");
    return 0;
}

Result LanMatchJointSessionJob::ReceiveLeaveInvitationImpl(session::Session::ProcessType processType,
                                                           const unsigned long long* stationIds,
                                                           unsigned short stationIdNum,
                                                           unsigned long long senderStationId)
{
    return cResultNotSupported;
}

unsigned int LanMatchJointSessionJob::GetJointSessionStateForDebug() const
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveNextSessionIdImpl(session::Session::ProcessType processType,
                                                         unsigned int sessionId, const unsigned char* applicationData,
                                                         unsigned char applicationDataSize,
                                                         unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType,
                                                               unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveCompletionResponseImpl(session::Session::ProcessType processType,
                                                              unsigned long long senderStationId)
{
    return 0;
}

void LanMatchJointSessionJob::CleanupImpl()
{
}

}
}
}
