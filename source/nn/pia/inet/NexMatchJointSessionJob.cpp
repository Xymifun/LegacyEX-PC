#include "nn/pia/inet/NexMatchJointSessionJob.hpp"

#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/SessionParticipantInfo.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinRandomSessionSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

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
    cResultInvalidArgument = 0x10c07,
    cResultNoInvitationTarget = 0x2c0a,
    cResultSendBufferFull = 0x4c0d,
    cResultCommunicationFailed = 0x6c57,
    cResultConnectionError = 0x6c50,
    cResultOwnerChanged = 0x6c59,
    cResultRequestFailed = 0xac5a,
    cResultRetryJoinMesh = 0x6c4b
};

enum
{
    cFailurePointCloseMatchmakeSession = 1,
    cFailurePointCloseJointSessionParticipation = 2,
    cFailurePointSessionStatus = 3,
    cFailurePointSendError = 3,
    cFailurePointConnectionError = 4,
    cFailurePointLeaderCommunication = 5,
    cFailurePointDisconnectDetected = 6,
    cFailurePointDestroyInvitationUndelivered = 7,
    cFailurePointNoInvitationTarget = 8,
    cFailurePointSendNextSessionId = 9,
    cFailurePointSendLeaveMeshInvitation = 0xa,
    cFailurePointDestroyInvitationRejected = 0xb,
    cFailurePointInvitationRejected = 0xc,
    cFailurePointInvitationAsCompanionTimeout = 0xe,
    cFailurePointMeshSessionMismatch = 0xf,
    cFailurePointRandomMatchmake = 0x10,
    cFailurePointNextSessionIdIsCurrent = 0x11,
    cFailurePointCreateMatchmakeSession = 0x12,
    cFailurePointJoinMatchmakeSession = 0x13,
    cFailurePointNoBufferSessionId = 0x14,
    cFailurePointNoOwnerEntry = 0x15,
    cFailurePointRequestSessionInfo = 0x16,
    cFailurePointUpdateHostFailed = 0x17,
    cFailurePointSessionIdMismatch = 0x18,
    cFailurePointMeshStartupFailed = 0x1a,
    cFailurePointCreateNextMesh = 0x1b,
    cFailurePointGetStationConnectionInfo = 0x1d,
    cFailurePointInvalidProcessType = 0x1f,
    cFailurePointInvitationTimeout = 0x20,
    cFailurePointCompletionForMigrateTimeout = 0x22,
    cFailurePointCompanionTimeout = 0x23,
    cFailurePointSameSessionId = 0x24,
    cFailurePointNoticeEndingEventFailed = 0x25,
    cFailurePointLeavePreviousMatchmakeSession = 0x26,
    cFailurePointAnswerToCompletionTimeout = 0x38,
    cFailurePointUnknown = 0x39
};

enum
{
    cResultInvalidState = 0x10c08,
    cResultJoinMesh6C05 = 0x6c05,
    cResultJoinMesh6439 = 0x6439,
    cResultJoinMesh643F = 0x643f,
    cResultJoinMesh6442 = 0x6442,
    cResultJoinMesh6443 = 0x6443,
    cResultJoinMesh6444 = 0x6444,
    cResultJoinMesh6445 = 0x6445,
    cResultJoinMesh644A = 0x644a,
    cResultJoinMesh6467 = 0x6467,
    cResultJoinMesh6468 = 0x6468,
    cResultJoinMesh6469 = 0x6469,
    cResultJoinMesh646A = 0x646a,
    cResultJoinMesh6838 = 0x6838,
    cResultJoinMesh6C3D = 0x6c3d,
    cResultJoinMesh6846 = 0x6846,
    cResultJoinMesh6847 = 0x6847,
    cResultJoinMesh6848 = 0x6848,
    cResultJoinMesh6849 = 0x6849,
    cResultJoinMesh10C3E = 0x10c3e,
    cResultJoinMesh10C25 = 0x10c25
};

enum
{
    cFailurePointJoinMesh6C05 = 0x27,
    cFailurePointInvalidState = 0x28,
    cFailurePointJoinMesh6439 = 0x29,
    cFailurePointJoinMesh643F = 0x2a,
    cFailurePointJoinMesh6442 = 0x2b,
    cFailurePointJoinMesh6445 = 0x2c,
    cFailurePointJoinMesh6443 = 0x2d,
    cFailurePointJoinMesh6444 = 0x2e,
    cFailurePointJoinMesh644A = 0x2f,
    cFailurePointJoinMesh6838 = 0x30,
    cFailurePointJoinMesh6C3D = 0x31,
    cFailurePointJoinMesh6846 = 0x32,
    cFailurePointJoinMesh6847 = 0x33,
    cFailurePointJoinMesh6848 = 0x34,
    cFailurePointJoinMesh6849 = 0x35,
    cFailurePointJoinMesh10C3E = 0x36,
    cFailurePointJoinMesh10C25 = 0x37,
    cFailurePointJoinMeshOther = 0x1c
};

enum
{
    cJointSessionStateStartup = 1,
    cJointSessionStateCallDestroySessionEvent = 2,
    cJointSessionStateDestroyInvitationReceived = 3,
    cJointSessionStateAnswerToInvitation = 4,
    cJointSessionStateCallSessionEvent = 5,
    cJointSessionStatePassiveProcess = 6,
    cJointSessionStateStartMatchmake = 7,
    cJointSessionStateSendNextSessionId = 8,
    cJointSessionStateWaitNextSessionId = 9,
    cJointSessionStateNextSessionIdReceived = 0xa,
    cJointSessionStateSendLeaveMeshCompanion = 0xb,
    cJointSessionStateWaitLeaderLeavesMesh = 0xc,
    cJointSessionStateLeaveMeshInvitationReceived = 0xd,
    cJointSessionStateLeavePreviousMesh = 0xe,
    cJointSessionStateCreateNextMesh = 0xf,
    cJointSessionStateJoinNextMesh = 0x10,
    cJointSessionStateWaitCompanionStation = 0x11,
    cJointSessionStateSendCompletion = 0x12,
    cJointSessionStateWaitCompletionInvitation = 0x13,
    cJointSessionStateSendCompletionInvitation = 0x14,
    cJointSessionStateWaitForAnswerToCompletionInvitation = 0x15,
    cJointSessionStateFinished = 0x16
};

enum
{
    cLeaveMeshTypeLeave = 1,
    cLeaveMeshTypeLeaveWithHostMigration = 2,
    cLeaveMeshTypeDestroy = 3
};

enum
{
    cProcessStepNone = 0,
    cProcessStepWaitNextSessionId = 3,
    cProcessStepAnswerToInvitation = 6,
    cProcessStepInvitationSent = 7,
    cProcessStepWaitAnswerToDestroyInvitation = 9,
    cProcessStepWaitLeaderLeavesMesh = 0xa,
    cProcessStepLeaveMeshCompanion = 0xb,
    cProcessStepWaitUntilCompanionCompletion = 0x13,
    cProcessStepWaitCompletionInvitation = 0x14,
    cProcessStepWaitForAnswerToCompletionInvitation = 0x15
};

enum
{
    cMonitoringUnset8 = 0xff,
    cMonitoringUnset16 = 0xffff,
    cMonitoringUnset32 = 0xffffffff
};

enum
{
    cReportReasonJointSession = 1
};

enum
{
    cHostStationUpdateIntervalMsec = 10000,
    cRetryJoinIntervalMsec = 3000,
    cRetryJoinIntervalHostMsec = 12000,
    cCompanionTimeoutMsec = 40000,
    cCompletionTimeoutMsec = 10000,
    cJoinMeshRetryMax = 2,
    cConnectionInfoRetryMax = 2,
    cInvitationAsCompanionTimeoutMsec = 20000,
    cDestroyInvitationTimeoutMsec = 20000,
    cAnswerToInvitationTimeoutMsec = 15000,
    cDestroyInvitationAnswerTimeoutMsec = 15000,
    cResendInvitationIntervalMsec = 2000,
    cLeaveMeshCompanionTimeoutMsec = 30000,
    cMeshRestartWaitMsec = 5000,
    cNotificationTimeoutMsec = 10000,
    cCompanionPrepareTimeoutMsec = 15000,
    cNextSessionIdResendIntervalMsec = 2000,
    cLeaveMeshAnswerTimeoutMsec = 10000,
    cLeaveMeshWaitIntervalMsec = 1000,
    cCompanionStationIdNumMax = NexMatchJointSessionJob::cStationNumMax - 1,
    cHostMigratedTimeoutMsec = 15000,
    cInvitationAsCompanionWaitTimeoutMsec = 15000,
    cAnswerToCompletionTimeoutMsec = 15000,
    cCompanionRewaitElapsedMsec = 36000,
    cCompanionStationNumMin = 2,
    cHostMigrationStationNumThreshold = 2
};

const unsigned long long cTraceJointSessionLevel = 0x0000000100000000ULL;

inline unsigned int bufferSessionIndex()
{
    return session::g_piaSession->m_currentSessionIndex == 0 ? 1 : 0;
}

inline void unregisterBufferSearchCriteria()
{
    static_cast<NexMatchmakeSession*>(session::g_piaSession->m_matchmakeSession[bufferSessionIndex()])
        ->UnregisterSearchCriteria();
}

inline bool isElapsed(const common::Time& start, int msec)
{
    return static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - start.m_tick)
           >= common::TimeSpan::GetTicksPerMSec().m_tick * msec;
}

inline void setDeadline(common::Time& deadline, int msec)
{
    deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                      + common::TimeSpan::GetTicksPerMSec().m_tick * msec;
}

inline common::Time deadlineAfter(int msec)
{
    common::Time deadline;
    deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                      + common::TimeSpan::GetTicksPerMSec().m_tick * msec;
    return deadline;
}

inline common::Time timeBefore(int msec)
{
    common::Time time;
    time.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick - common::TimeSpan::GetTicksPerMSec().m_tick * msec;
    return time;
}

inline void countRetry(unsigned char& counter)
{
    if (counter == cMonitoringUnset8)
        counter = 1;
    else
        ++counter;
}
}

#define NEX_MATCH_JOINT_SESSION_FAIL_BY_STATUS(result) \
    m_sessionDisconnectDetected = true; \
    m_fieldc8 = cFailurePointSessionStatus; \
    m_failureResult = result; \
    RecordMonitoringDataWhenProcessFailure(); \
    session::g_piaSession->m_meshLayerController->MeshCleanup(); \
    setStep(&NexMatchJointSessionJob::ProcessFailure, "NexMatchJointSessionJob::ProcessFailure"); \
    return MakeExecuteResult(cExecuteKindContinue)

#define NEX_MATCH_JOINT_SESSION_CHECK_STATUS() \
    { \
        const unsigned int status = session::g_piaSession->GetStatus(); \
        switch (status) \
        { \
        case session::Session::cStatusDisconnectedNetwork: \
            NEX_MATCH_JOINT_SESSION_FAIL_BY_STATUS(session::Session::cResultStatusDisconnectedNetwork); \
        case session::Session::cStatusUnknown6: \
            NEX_MATCH_JOINT_SESSION_FAIL_BY_STATUS(session::Session::cResultStatusUnknown6); \
        case session::Session::cStatusUnknown7: \
            NEX_MATCH_JOINT_SESSION_FAIL_BY_STATUS(session::Session::cResultStatusUnknown7); \
        case session::Session::cStatusUnknown8: \
            NEX_MATCH_JOINT_SESSION_FAIL_BY_STATUS(session::Session::cResultStatusUnknown8); \
        default: \
            break; \
        } \
    }

NexMatchJointSessionJob::NexMatchJointSessionJob()
    : session::JointSessionJob(),
      m_stationConnectionInfo(),
      m_failureResult(0),
      m_jointSessionState(0),
      m_field1ee(1),
      m_sessionIdNum(0)
{
    for (unsigned char i = 0; i < cStationNumMax; ++i)
    {
        m_sessionIds[i] = 0;
        m_sessionInfoRequested[i] = false;
    }
}

NexMatchJointSessionJob::~NexMatchJointSessionJob()
{
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CallSessionEvent()
{
    if (session::g_piaMesh->CheckConnectionError() != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedSession);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown6)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown7)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown8)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    NoticeStartingEventByProcessType();
    session::g_piaSession->m_postponeNotice = true;
    session::g_piaMesh->SetSkipReport(true);
    m_time158 = common::g_piaJobScheduler->m_dispatchTime;
    m_time168 = common::g_piaJobScheduler->m_dispatchTime;

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_field1ec != 0)
    {
        m_fieldc8 = cFailurePointMeshSessionMismatch;
        m_failureResult = cResultInvalidState;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_processType == session::Session::cProcessTypeDestroyJointSession)
    {
        m_field60 = cProcessStepAnswerToInvitation;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::SendAnswerToDestroyInvitation),
                            "NexMatchJointSessionJob::SendAnswerToDestroyInvitation"));
    }
    else if (m_field9c == 0)
    {
        m_field60 = cProcessStepAnswerToInvitation;
        m_jointSessionState = cJointSessionStateAnswerToInvitation;
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::SendAnswerToInvitation),
            "NexMatchJointSessionJob::SendAnswerToInvitation"));
    }
    else
    {
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::CloseMatchmakeSession),
            "NexMatchJointSessionJob::CloseMatchmakeSession"));
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CloseMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (session::g_piaSession->GetSessionOpenStatus() != session::Session::cSessionOpenStatusParticipationClosed)
    {
        unsigned int currentIndex = piaSession->m_currentSessionIndex;
        Result closeResult = piaSession->m_matchmakeSession[currentIndex]->CloseParticipate(
            &m_callContext, piaSession->m_sessionId[currentIndex]);
        if (closeResult != 0)
        {
            m_failureResult = closeResult;
            m_fieldc8 = cFailurePointCloseMatchmakeSession;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        setStep(&NexMatchJointSessionJob::WaitCloseMatchmakeSession,
                "NexMatchJointSessionJob::WaitCloseMatchmakeSession");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (IsMigrateJointSessionToSession() && piaSession->m_matchmakeSession[bufferSessionIndex()]->IsMatchmakeOwner())
    {
        setStep(&NexMatchJointSessionJob::CloseJointSessionParticipation,
                "NexMatchJointSessionJob::CloseJointSessionParticipation");
    }
    else
    {
        m_jointSessionState = cJointSessionStateCallSessionEvent;
        setDeadline(m_time188, cInvitationAsCompanionTimeoutMsec);
        setStep(&NexMatchJointSessionJob::SendInvitationAsCompanion,
                "NexMatchJointSessionJob::SendInvitationAsCompanion");
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCloseMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Session* piaSession = session::g_piaSession;
    if (!piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->WaitCloseParticipate())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result != 0)
    {
        m_fieldc8 = cFailurePointCloseMatchmakeSession;
        if (m_callContext.m_result == cResultInvalidState)
            m_failureResult = cResultOwnerChanged;
        else
            m_failureResult = m_callContext.m_result;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->SetCurrentSessionOpenStatus(session::Session::cSessionOpenStatusParticipationClosed);
    if (IsMigrateJointSessionToSession() && piaSession->m_matchmakeSession[bufferSessionIndex()]->IsMatchmakeOwner())
    {
        setStep(&NexMatchJointSessionJob::CloseJointSessionParticipation,
                "NexMatchJointSessionJob::CloseJointSessionParticipation");
    }
    else
    {
        m_jointSessionState = cJointSessionStateCallSessionEvent;
        setDeadline(m_time188, cInvitationAsCompanionTimeoutMsec);
        setStep(&NexMatchJointSessionJob::SendInvitationAsCompanion,
                "NexMatchJointSessionJob::SendInvitationAsCompanion");
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CloseJointSessionParticipation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (session::g_piaSession->GetJointSessionOpenStatus() != session::Session::cSessionOpenStatusParticipationClosed)
    {
        unsigned int bufferIndex = bufferSessionIndex();
        Result closeResult = piaSession->m_matchmakeSession[bufferIndex]->CloseParticipate(
            &m_callContext, piaSession->m_sessionId[bufferIndex]);
        if (closeResult != 0)
        {
            m_failureResult = closeResult;
            m_fieldc8 = cFailurePointCloseJointSessionParticipation;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::WaitCloseJointSessionParticipation),
                            "NexMatchJointSessionJob::WaitCloseJointSessionParticipation"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_processType == session::Session::cProcessTypeDestroyJointSession)
    {
        setDeadline(m_time188, cDestroyInvitationTimeoutMsec);
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::SendDestroyInvitation),
            "NexMatchJointSessionJob::SendDestroyInvitation"));
    }
    else if (session::g_piaSession->IsHostMigrationEnabled())
    {
        m_sessionIdNum = session::g_piaSession->m_stationIdStatusTable->GetSessionIdArray(m_sessionIds);
        for (unsigned char i = 0; i < m_sessionIdNum; ++i)
            m_sessionInfoRequested[i] = false;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::GetMatchmakeSessionOwners),
                            "NexMatchJointSessionJob::GetMatchmakeSessionOwners"));
    }
    else
    {
        m_jointSessionState = cJointSessionStateCallSessionEvent;
        setDeadline(m_time188, cInvitationAsCompanionTimeoutMsec);
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::SendInvitationAsCompanion),
                            "NexMatchJointSessionJob::SendInvitationAsCompanion"));
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCloseJointSessionParticipation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (!session::g_piaSession->m_matchmakeSession[bufferSessionIndex()]->WaitCloseParticipate())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result != 0)
    {
        m_fieldc8 = cFailurePointCloseJointSessionParticipation;
        if (m_callContext.m_result == cResultInvalidState)
            m_failureResult = cResultOwnerChanged;
        else
            m_failureResult = m_callContext.m_result;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->SetBufferSessionOpenStatus(session::Session::cSessionOpenStatusParticipationClosed);
    if (m_processType == session::Session::cProcessTypeDestroyJointSession)
    {
        setDeadline(m_time188, cDestroyInvitationTimeoutMsec);
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::SendDestroyInvitation),
            "NexMatchJointSessionJob::SendDestroyInvitation"));
    }
    else if (session::g_piaSession->IsHostMigrationEnabled())
    {
        m_sessionIdNum = session::g_piaSession->m_stationIdStatusTable->GetSessionIdArray(m_sessionIds);
        for (unsigned char i = 0; i < m_sessionIdNum; ++i)
            m_sessionInfoRequested[i] = false;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::GetMatchmakeSessionOwners),
                            "NexMatchJointSessionJob::GetMatchmakeSessionOwners"));
    }
    else
    {
        m_jointSessionState = cJointSessionStateCallSessionEvent;
        setDeadline(m_time188, cInvitationAsCompanionTimeoutMsec);
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::SendInvitationAsCompanion),
                            "NexMatchJointSessionJob::SendInvitationAsCompanion"));
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendInvitationAsCompanion()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (m_time188 < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_fieldc8 = cFailurePointInvitationTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    session::Session* piaSession = session::g_piaSession;
    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();

    unsigned long long stationIds[cStationNumMax - 1];
    unsigned short stationIdNum = 0;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(*StationIdList::ValueOf(node)))
        {
            stationIds[stationIdNum] = *StationIdList::ValueOf(node);
            ++stationIdNum;
        }
    }

    if (stationIdNum > 1)
    {
        Result sendResult = piaSession->m_sessionProtocol->SendInvitationAsCompanion(
            static_cast<unsigned char>(m_processType), stationIds, stationIdNum, piaSession->m_localStationId, 0, 0);
        if (sendResult != cResultSendBufferFull)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                unsigned short connectedStationNum = 0;
                for (unsigned short i = 0; i < stationIdNum; ++i)
                {
                    if (stationIds[i] != piaSession->m_localStationId
                        && piaSession->m_stationIdStatusTable->CheckConnectStatus(stationIds[i]))
                    {
                        ++connectedStationNum;
                    }
                }
                if (connectedStationNum != 0)
                {
                    m_fieldc8 = cFailurePointNoInvitationTarget;
                    m_failureResult = cResultCommunicationFailed;
                    unregisterBufferSearchCriteria();
                    RecordMonitoringDataWhenProcessFailure();
                    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
            }
            else if (sendResult != 0)
            {
                m_failureResult = sendResult;
                m_fieldc8 = cFailurePointSendError;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
    }

    m_field60 = cProcessStepInvitationSent;
    piaSession->m_stationIdStatusTable->ClearAllAnswerStatus();
    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();
    piaSession->m_stationIdStatusTable->SetAnswer(m_leaveStationId, true);
    piaSession->m_stationIdStatusTable->SetSendInvitationStatus(m_leaveStationId, true);
    m_time148 = common::g_piaJobScheduler->m_dispatchTime;
    setDeadline(m_time180, cAnswerToInvitationTimeoutMsec);
    setStep(&NexMatchJointSessionJob::WaitForAnswerToInvitation, "NexMatchJointSessionJob::WaitForAnswerToInvitation");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ResendInvitationAsCompanion()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    unsigned long long stationIds[cStationNumMax - 1];
    unsigned long long pendingStationIds[cStationNumMax - 1];
    unsigned short stationIdNum = 0;
    unsigned short pendingStationIdNum = 0;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(*StationIdList::ValueOf(node)))
        {
            stationIds[stationIdNum] = *StationIdList::ValueOf(node);
            ++stationIdNum;
            session::StationIdStatusTable::AnswerStatus answerStatus =
                piaSession->m_stationIdStatusTable->CheckAnswerStatus(*StationIdList::ValueOf(node));
            bool invitationSent = piaSession->m_stationIdStatusTable->CheckSendInvitationStatus(
                *StationIdList::ValueOf(node));
            if (answerStatus == session::StationIdStatusTable::cAnswerStatusNone && !invitationSent)
            {
                pendingStationIds[pendingStationIdNum] = *StationIdList::ValueOf(node);
                ++pendingStationIdNum;
            }
        }
    }

    if (pendingStationIdNum != 0)
    {
        Result sendResult = piaSession->m_sessionProtocol->SendInvitationAsCompanion(
            static_cast<unsigned char>(m_processType), stationIds, stationIdNum, piaSession->m_localStationId,
            pendingStationIds, pendingStationIdNum);
        if (sendResult != cResultSendBufferFull && sendResult != 0)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                m_fieldc8 = cFailurePointNoInvitationTarget;
                m_failureResult = cResultOwnerChanged;
            }
            else
            {
                m_failureResult = sendResult;
                m_fieldc8 = cFailurePointSendError;
            }
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }

    m_time148 = common::g_piaJobScheduler->m_dispatchTime;
    SetCurrentStep(Step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitForAnswerToInvitation),
        "NexMatchJointSessionJob::WaitForAnswerToInvitation"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForAnswerToInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    bool answerTimedOut = m_time180 < common::g_piaJobScheduler->m_dispatchTime;
    session::Session* piaSession = session::g_piaSession;
    bool allAnswered = true;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        session::StationIdStatusTable::AnswerStatus answerStatus =
            piaSession->m_stationIdStatusTable->CheckAnswerStatus(*StationIdList::ValueOf(node));
        if (answerStatus == session::StationIdStatusTable::cAnswerStatusNone)
        {
            if (piaSession->m_stationIdStatusTable->CheckConnectStatus(*StationIdList::ValueOf(node))
                && !answerTimedOut)
            {
                allAnswered = false;
            }
        }
        else if (answerStatus == session::StationIdStatusTable::cAnswerStatusFalse)
        {
            m_fieldc8 = cFailurePointInvitationRejected;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }

    if (!allAnswered)
    {
        if (isElapsed(m_time148, cResendInvitationIntervalMsec))
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &NexMatchJointSessionJob::ResendInvitationAsCompanion),
                                "NexMatchJointSessionJob::ResendInvitationAsCompanion"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    m_field60 = cProcessStepNone;
    m_fieldc0 = common::g_piaJobScheduler->m_dispatchTime.m_tick;
    switch (m_processType)
    {
    case session::Session::cProcessTypeCreateJointSession:
        m_jointSessionState = cJointSessionStateStartMatchmake;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::StartCreateMatchmakeSession),
                            "NexMatchJointSessionJob::StartCreateMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    case session::Session::cProcessTypeJoinJointSession:
        m_jointSessionState = cJointSessionStateStartMatchmake;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::StartJoinMatchmakeSession),
                            "NexMatchJointSessionJob::StartJoinMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    case session::Session::cProcessTypeJoinRandomJointSession:
        m_jointSessionState = cJointSessionStateStartMatchmake;
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartRandomMatchmake),
            "NexMatchJointSessionJob::StartRandomMatchmake"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    case session::Session::cProcessTypeDestroyJointSession:
    case session::Session::cProcessTypeLeaveJointSession:
        m_jointSessionState = cJointSessionStateSendLeaveMeshCompanion;
        setDeadline(m_time170, cLeaveMeshCompanionTimeoutMsec);
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::SendLeaveMeshCompanion),
            "NexMatchJointSessionJob::SendLeaveMeshCompanion"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    default:
        m_fieldc8 = cFailurePointInvalidProcessType;
        m_failureResult = cResultInvalidState;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartRandomMatchmake()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    Result result = session::g_piaMesh->CheckConnectionError();
    if (result != 0 || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[bufferSessionIndex()]);
    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    for (int i = 0; i < cCompanionStationIdNumMax; ++i)
    {
        companionStationIds[i] = 0;
    }
    unsigned int companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (stationId != m_leaveStationId && piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId))
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    result = matchmakeSession->RandomMatchmakeSessionWithTeam(
        &m_callContext, companionStationIds, static_cast<unsigned short>(companionStationIdNum),
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        m_failureResult = result;
        m_fieldc8 = cFailurePointRandomMatchmake;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitRandomMatchmake),
             "NexMatchJointSessionJob::WaitRandomMatchmake"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitRandomMatchmake()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    unsigned int sessionId = 0;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[bufferSessionIndex()]);
    if (!matchmakeSession->WaitRandomMatchmakeSessionWithTeam(&sessionId, &m_field9d))
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    matchmakeSession->UnregisterSearchCriteria();
    if (m_callContext.m_result != 0)
    {
        m_fieldc8 = cFailurePointRandomMatchmake;
        m_failureResult = m_callContext.m_result;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    m_field98 = sessionId;
    session::g_piaSession->m_sessionId[bufferSessionIndex()] = sessionId;
    if (m_field9d != 0)
    {
        session::g_piaSession->SetBufferSessionOpenStatus(
            m_field1ee != 0 ? session::Session::cSessionOpenStatusOpen
                            : session::Session::cSessionOpenStatusParticipationClosed);
    }
    else if (matchmakeSession->IsRandomMatchmakeScoreAvailable())
    {
        unsigned int randomMatchmakeScore = matchmakeSession->GetRandomMatchmakeScore();
        session::g_piaSession->m_field148 = 1;
        session::g_piaSession->m_field144 = randomMatchmakeScore;
    }
    if (m_processType == session::Session::cProcessTypeJoinRandomJointSession)
        common::g_sessionBeginMonitoringContent.m_metric75 = m_field98;

    m_time1a0 = deadlineAfter(cNotificationTimeoutMsec);
    setStep(&NexMatchJointSessionJob::WaitNotification, "NexMatchJointSessionJob::WaitNotification");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartCreateMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    Result result = session::g_piaMesh->CheckConnectionError();
    if (result != 0 || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[bufferSessionIndex()]);
    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    for (int i = 0; i < cCompanionStationIdNumMax; ++i)
    {
        companionStationIds[i] = 0;
    }
    unsigned int companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (stationId != m_leaveStationId && piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId))
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    result = matchmakeSession->CreateMatchmakeSessionWithTeam(
        &m_callContext, companionStationIds, static_cast<unsigned short>(companionStationIdNum),
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        m_failureResult = result;
        m_fieldc8 = cFailurePointCreateMatchmakeSession;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::WaitCreateMatchmakeSession,
            "NexMatchJointSessionJob::WaitCreateMatchmakeSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCreateMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    unsigned int sessionId = 0;
    if (!static_cast<NexMatchmakeSession*>(session::g_piaSession->m_matchmakeSession[bufferSessionIndex()])
             ->WaitCreateMatchmakeSessionWithTeam(&sessionId))
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result != 0)
    {
        m_fieldc8 = cFailurePointCreateMatchmakeSession;
        m_failureResult = m_callContext.m_result;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    m_field98 = sessionId;
    session::g_piaSession->m_sessionId[bufferSessionIndex()] = sessionId;
    session::g_piaSession->SetBufferSessionOpenStatus(m_field1ee != 0
                                                          ? session::Session::cSessionOpenStatusOpen
                                                          : session::Session::cSessionOpenStatusParticipationClosed);
    common::g_sessionBeginMonitoringContent.m_metric75 = m_field98;

    m_time1a0 = deadlineAfter(cNotificationTimeoutMsec);
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitNotification),
             "NexMatchJointSessionJob::WaitNotification"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartJoinMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    Result result = session::g_piaMesh->CheckConnectionError();
    if (result != 0 || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[bufferSessionIndex()]);
    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    for (int i = 0; i < cCompanionStationIdNumMax; ++i)
    {
        companionStationIds[i] = 0;
    }
    unsigned int companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (stationId != m_leaveStationId && piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId))
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    result = matchmakeSession->JoinMatchmakeSessionWithTeam(
        &m_callContext, m_field98, companionStationIds, static_cast<unsigned short>(companionStationIdNum),
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        m_failureResult = result;
        m_fieldc8 = cFailurePointJoinMatchmakeSession;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::WaitJoinMatchmakeSession, "NexMatchJointSessionJob::WaitJoinMatchmakeSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitJoinMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (!static_cast<NexMatchmakeSession*>(session::g_piaSession->m_matchmakeSession[bufferSessionIndex()])
             ->WaitJoinMatchmakeSessionWithTeam())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result != 0)
    {
        m_fieldc8 = cFailurePointJoinMatchmakeSession;
        m_failureResult = m_callContext.m_result;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_sessionId[bufferSessionIndex()] = m_field98;
    common::g_sessionBeginMonitoringContent.m_metric75 = m_field98;

    m_time1a0 = deadlineAfter(cNotificationTimeoutMsec);
    setStep(&NexMatchJointSessionJob::WaitNotification, "NexMatchJointSessionJob::WaitNotification");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitNotification()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_time1a0 < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_failureResult = cResultRequestFailed;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    nex::NgsBridgeInterface* ngsInterface = meshLayerController->GetNgsInterface(0);
    if (ngsInterface != 0 && !meshLayerController->IsParticipated(m_field98, ngsInterface->GetPrincipalID()))
    {
        meshLayerController->Trace(cTraceJointSessionLevel);
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    common::g_sessionBeginMonitoringContent.m_metric80 = static_cast<unsigned int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_fieldc0)
        / common::TimeSpan::GetTicksPerMSec().m_tick);
    setStep(&NexMatchJointSessionJob::SendNextSessionId, "NexMatchJointSessionJob::SendNextSessionId");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendNextSessionId()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    m_jointSessionState = cJointSessionStateSendNextSessionId;
    session::Session* piaSession = session::g_piaSession;
    if (m_field9c != 0 && m_field1b8 != 0 && m_field1c0 != piaSession->m_localStationId)
    {
        m_leaveStationId = m_field1c0;
        m_field9c = 0;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::SendPreparedForMigrateSession),
                            "NexMatchJointSessionJob::SendPreparedForMigrateSession"));
        m_field1b8 = 0;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();
    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    unsigned short companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId))
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    if (companionStationIdNum > 1)
    {
        NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
            piaSession->m_matchmakeSession[bufferSessionIndex()]);
        Result sendResult = piaSession->m_sessionProtocol->SendNextSessionId(
            static_cast<unsigned char>(m_processType), piaSession->m_sessionId[piaSession->m_currentSessionIndex],
            m_field98, matchmakeSession->GetMatchmakeOwnerUniqueId(), matchmakeSession->m_matchmakeSessionKey,
            sizeof(matchmakeSession->m_matchmakeSessionKey), companionStationIds, companionStationIdNum);
        if (sendResult != cResultSendBufferFull)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                unsigned short connectedStationNum = 0;
                for (unsigned short i = 0; i < companionStationIdNum; ++i)
                {
                    if (companionStationIds[i] != piaSession->m_localStationId
                        && piaSession->m_stationIdStatusTable->CheckConnectStatus(companionStationIds[i]))
                    {
                        ++connectedStationNum;
                    }
                }
                if (connectedStationNum != 0)
                {
                    m_fieldc8 = cFailurePointSendNextSessionId;
                    m_failureResult = cResultCommunicationFailed;
                    unregisterBufferSearchCriteria();
                    RecordMonitoringDataWhenProcessFailure();
                    SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                            &NexMatchJointSessionJob::StartLeaveMesh),
                                        "NexMatchJointSessionJob::StartLeaveMesh"));
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
            }
            else if (sendResult != 0)
            {
                m_failureResult = sendResult;
                m_fieldc8 = cFailurePointSendError;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                SetCurrentStep(Step(
                    static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                    "NexMatchJointSessionJob::StartLeaveMesh"));
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
    }

    piaSession->m_stationIdStatusTable->ClearAllPrepareStatus();
    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();
    piaSession->m_stationIdStatusTable->SetPrepareStatus(piaSession->m_localStationId, true);
    piaSession->m_stationIdStatusTable->SetSendInvitationStatus(piaSession->m_localStationId, true);
    m_time150 = common::g_piaJobScheduler->m_dispatchTime;
    m_time180 = deadlineAfter(cCompanionPrepareTimeoutMsec);
    SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                            &NexMatchJointSessionJob::WaitCompanionStationPrepared),
                        "NexMatchJointSessionJob::WaitCompanionStationPrepared"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ResendNextSessionId()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    if (m_field9c != 0 && m_field1b8 != 0 && m_field1c0 != piaSession->m_localStationId)
    {
        m_leaveStationId = m_field1c0;
        m_field9c = 0;
        m_field1b8 = 0;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::SendPreparedForMigrateSession),
                            "NexMatchJointSessionJob::SendPreparedForMigrateSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    unsigned int companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId)
            && piaSession->m_stationIdStatusTable->CheckPrepareStatus(stationId)
                   == session::StationIdStatusTable::cPrepareStatusNone
            && !piaSession->m_stationIdStatusTable->CheckSendInvitationStatus(stationId))
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    if (companionStationIdNum != 0)
    {
        NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
            piaSession->m_matchmakeSession[bufferSessionIndex()]);
        Result sendResult = piaSession->m_sessionProtocol->SendNextSessionId(
            static_cast<unsigned char>(m_processType), piaSession->m_sessionId[piaSession->m_currentSessionIndex],
            m_field98, matchmakeSession->GetMatchmakeOwnerUniqueId(), matchmakeSession->m_matchmakeSessionKey,
            sizeof(matchmakeSession->m_matchmakeSessionKey), companionStationIds,
            static_cast<unsigned short>(companionStationIdNum));
        if (sendResult != cResultSendBufferFull && sendResult != 0)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                m_fieldc8 = cFailurePointSendNextSessionId;
                m_failureResult = cResultCommunicationFailed;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                SetCurrentStep(Step(
                    static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                    "NexMatchJointSessionJob::StartLeaveMesh"));
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
            m_failureResult = sendResult;
            m_fieldc8 = cFailurePointSendError;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }

    m_time150 = common::g_piaJobScheduler->m_dispatchTime;
    SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                            &NexMatchJointSessionJob::WaitCompanionStationPrepared),
                        "NexMatchJointSessionJob::WaitCompanionStationPrepared"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCompanionStationPrepared()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    if (m_field9c != 0 && m_field1b8 != 0 && m_field1c0 != piaSession->m_localStationId)
    {
        m_leaveStationId = m_field1c0;
        m_field9c = 0;
        m_field1b8 = 0;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::SendPreparedForMigrateSession),
                            "NexMatchJointSessionJob::SendPreparedForMigrateSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    bool prepareTimedOut = false;
    if (m_time180 < common::g_piaJobScheduler->m_dispatchTime)
        prepareTimedOut = true;

    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    unsigned short companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId)
            && piaSession->m_stationIdStatusTable->CheckPrepareStatus(stationId)
                   != session::StationIdStatusTable::cPrepareStatusNotPrepared)
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    if (piaSession->m_stationIdStatusTable->CheckAllPrepareStatus(companionStationIds, companionStationIdNum)
            == session::StationIdStatusTable::cPrepareCheckPending
        && !prepareTimedOut)
    {
        if (isElapsed(m_time150, cNextSessionIdResendIntervalMsec))
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &NexMatchJointSessionJob::ResendNextSessionId),
                                "NexMatchJointSessionJob::ResendNextSessionId"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    SetCurrentStep(Step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::SendLeaveMeshCompanion),
        "NexMatchJointSessionJob::SendLeaveMeshCompanion"));
    m_jointSessionState = cJointSessionStateSendLeaveMeshCompanion;
    m_time170 = deadlineAfter(cLeaveMeshCompanionTimeoutMsec);
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendLeaveMeshCompanion()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    session::Session* piaSession = session::g_piaSession;
    if (m_processType == session::Session::cProcessTypeDestroyJointSession && piaSession->IsJointSessionHost())
    {
        bool otherSessionStationRemains = false;
        session::StationIdStatusTable* stationIdStatusTable = session::g_piaSession->m_stationIdStatusTable;
        const common::ListNode* entrySentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
        for (const common::ListNode* node = entrySentinel->next; node != entrySentinel; node = node->next)
        {
            const unsigned long long stationId = transport::StationIdTable::EntryList::ValueOf(node)->stationId;
            unsigned int stationSessionId;
            if (!stationIdStatusTable->CheckConnectStatus(stationId)
                && stationIdStatusTable->GetSessionId(stationId, &stationSessionId)
                && stationSessionId != piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            {
                otherSessionStationRemains = true;
                break;
            }
        }
        if (common::g_piaJobScheduler->m_dispatchTime <= m_time170 && otherSessionStationRemains)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();
    unsigned long long companionStationIds[cCompanionStationIdNumMax];
    unsigned short companionStationIdNum = 0;
    const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = *StationIdList::ValueOf(node);
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId))
        {
            companionStationIds[companionStationIdNum] = stationId;
            ++companionStationIdNum;
        }
    }

    if (companionStationIdNum > 1)
    {
        Result sendResult = piaSession->m_sessionProtocol->SendLeaveMeshInvitation(
            static_cast<unsigned char>(m_processType), companionStationIds, companionStationIdNum,
            piaSession->m_localStationId, 0, 0);
        if (sendResult != cResultSendBufferFull)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                unsigned short connectedStationNum = 0;
                for (unsigned short i = 0; i < companionStationIdNum; ++i)
                {
                    if (companionStationIds[i] != piaSession->m_localStationId
                        && piaSession->m_stationIdStatusTable->CheckConnectStatus(companionStationIds[i]))
                    {
                        ++connectedStationNum;
                    }
                }
                if (connectedStationNum != 0)
                {
                    m_fieldc8 = cFailurePointSendLeaveMeshInvitation;
                    m_failureResult = cResultCommunicationFailed;
                    unregisterBufferSearchCriteria();
                    RecordMonitoringDataWhenProcessFailure();
                    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
            }
            else if (sendResult != 0)
            {
                m_failureResult = sendResult;
                m_fieldc8 = cFailurePointSendError;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
    }

    session::g_piaSession->m_fieldc0 = 1;
    piaSession->m_stationIdStatusTable->ClearAllAnswerStatus();
    piaSession->m_stationIdStatusTable->SetAnswer(m_leaveStationId, true);
    piaSession->m_stationIdStatusTable->SetSendInvitationStatus(m_leaveStationId, true);
    m_field60 = cProcessStepLeaveMeshCompanion;
    m_time170 = deadlineAfter(cLeaveMeshAnswerTimeoutMsec);
    setStep(&NexMatchJointSessionJob::WaitUntilCompanionLeavesMesh,
            "NexMatchJointSessionJob::WaitUntilCompanionLeavesMesh");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUntilCompanionLeavesMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (common::g_piaJobScheduler->m_dispatchTime < m_time170)
    {
        const common::ListNode* sentinel = m_companionStationIdList.Sentinel();
        for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
        {
            const unsigned long long stationId = *StationIdList::ValueOf(node);
            if (piaSession->m_stationIdStatusTable->CheckAnswerStatus(stationId)
                    == session::StationIdStatusTable::cAnswerStatusNone
                && piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId))
            {
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
    }

    setStep(&NexMatchJointSessionJob::StartLeavePreviousMesh, "NexMatchJointSessionJob::StartLeavePreviousMesh");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindWait, cLeaveMeshWaitIntervalMsec);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendAnswerToInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    Result sendResult = piaSession->m_sessionProtocol->SendAnswerToInvitation(static_cast<unsigned char>(m_processType),
                                                                              m_leaveStationId, true);
    if (sendResult == cResultSendBufferFull)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (IsMigrateSessionToJointSession())
    {
        if (sendResult != 0)
        {
            m_fieldc8 = cFailurePointLeaderCommunication;
            m_failureResult = cResultCommunicationFailed;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        m_field60 = cProcessStepWaitNextSessionId;
        setStep(&NexMatchJointSessionJob::WaitNextSessionId, "NexMatchJointSessionJob::WaitNextSessionId");
        m_jointSessionState = cJointSessionStateWaitNextSessionId;
        session::g_piaSession->SetCurrentSessionOpenStatus(session::Session::cSessionOpenStatusParticipationClosed);
    }
    else
    {
        piaSession->m_stationIdStatusTable->SetPrepareStatus(piaSession->m_localStationId, true);
        piaSession->m_stationIdStatusTable->SetPrepareStatus(m_leaveStationId, true);
        session::g_piaSession->m_fieldc0 = 1;
        m_field9f = 0;
        m_field60 = cProcessStepWaitLeaderLeavesMesh;
        m_field98 = piaSession->m_sessionId[piaSession->m_currentSessionIndex];
        setStep(&NexMatchJointSessionJob::WaitUntilLeaderLeavesMesh,
                "NexMatchJointSessionJob::WaitUntilLeaderLeavesMesh");
        m_jointSessionState = cJointSessionStateWaitLeaderLeavesMesh;
        session::g_piaSession->SetCurrentSessionOpenStatus(session::Session::cSessionOpenStatusParticipationClosed);
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitNextSessionId()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (!piaSession->m_stationIdStatusTable->CheckConnectStatus(m_leaveStationId))
    {
        if (!piaSession->IsHostMigrationEnabled())
        {
            m_fieldc8 = cFailurePointLeaderCommunication;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        if (piaSession->m_sessionId[bufferSessionIndex()] == 0)
        {
            m_fieldc8 = cFailurePointNoBufferSessionId;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
        {
            m_fieldc8 = cFailurePointConnectionError;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        m_field98 = piaSession->m_sessionId[bufferSessionIndex()];
    }

    if (m_field98 == 0)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (!static_cast<NexMatchMeshLayerController*>(piaSession->m_meshLayerController)->CheckSessionId(m_field98))
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    m_jointSessionState = cJointSessionStateNextSessionIdReceived;
    if (session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] == m_field98)
    {
        m_fieldc8 = cFailurePointNextSessionIdIsCurrent;
        m_failureResult = cResultInvalidState;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_sessionId[bufferSessionIndex()] = m_field98;
    common::g_sessionBeginMonitoringContent.m_metric75 = m_field98;
    m_field60 = cProcessStepNone;
    if (piaSession->m_matchmakeSession[bufferSessionIndex()]->GetMatchmakeOwnerUniqueId() == 0)
    {
        setStep(&NexMatchJointSessionJob::GetNextMatchmakeSessionInfo,
                "NexMatchJointSessionJob::GetNextMatchmakeSessionInfo");
    }
    else
    {
        setStep(&NexMatchJointSessionJob::SendPreparedForMigrateSession,
                "NexMatchJointSessionJob::SendPreparedForMigrateSession");
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::GetNextMatchmakeSessionInfo()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    Result requestResult = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()]->RequestSessionInfo(
        &m_callContext, m_field98);
    if (requestResult != 0)
    {
        m_failureResult = requestResult;
        m_fieldc8 = cFailurePointRequestSessionInfo;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::WaitGetNextMatchmakeSessionInfo,
            "NexMatchJointSessionJob::WaitGetNextMatchmakeSessionInfo");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitGetNextMatchmakeSessionInfo()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (!matchmakeSession->WaitRequestSessionInfo())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result != 0)
    {
        m_fieldc8 = cFailurePointRequestSessionInfo;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_fieldc2[bufferSessionIndex()] = static_cast<unsigned char>(
        matchmakeSession->GetSessionInfo()->GetMaxParticipantNum());
    setStep(&NexMatchJointSessionJob::SendPreparedForMigrateSession,
            "NexMatchJointSessionJob::SendPreparedForMigrateSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

void NexMatchJointSessionJob::PrepareBecomingToLeader()
{
    session::Session* piaSession = session::g_piaSession;
    m_companionStationIdList.Clear();

    session::StationIdStatusTable* statusTable = piaSession->m_stationIdStatusTable;
    const common::ListNode* end = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    for (const common::ListNode* node = end->next; node != end; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        unsigned int sessionId;
        if (statusTable->CheckConnectStatus(entry->stationId) && statusTable->GetSessionId(entry->stationId, &sessionId)
            && sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = entry->stationId;
        }
    }

    m_field9c = 1;
    m_leaveStationId = piaSession->m_localStationId;
    m_field98 = piaSession->m_sessionId[piaSession->m_currentSessionIndex];
    piaSession->m_stationIdStatusTable->ClearAllAnswerStatus();
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendPreparedForMigrateSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_stationIdStatusTable->CheckConnectStatus(m_leaveStationId))
    {
        Result sendResult = piaSession->m_sessionProtocol->SendPreparedForMigrateSession(
            static_cast<unsigned char>(m_processType), m_leaveStationId, m_field98, true);
        if (sendResult == cResultSendBufferFull)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        if (sendResult != 0)
        {
            m_fieldc8 = cFailurePointLeaderCommunication;
            m_failureResult = cResultCommunicationFailed;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        piaSession->m_stationIdStatusTable->SetPrepareStatus(piaSession->m_localStationId, true);
        piaSession->m_stationIdStatusTable->SetPrepareStatus(m_leaveStationId, true);
        session::g_piaSession->m_fieldc0 = 1;
        m_jointSessionState = cJointSessionStateWaitLeaderLeavesMesh;
        m_field9f = 0;
        m_field60 = cProcessStepWaitLeaderLeavesMesh;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::WaitUntilLeaderLeavesMesh),
                            "NexMatchJointSessionJob::WaitUntilLeaderLeavesMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (!piaSession->IsHostMigrationEnabled())
    {
        m_fieldc8 = cFailurePointLeaderCommunication;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        m_fieldc8 = cFailurePointConnectionError;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_field1b8 == 0)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    m_field1b8 = 0;
    if (m_field1c0 == piaSession->m_localStationId)
    {
        PrepareBecomingToLeader();
        m_field98 = piaSession->m_sessionId[bufferSessionIndex()];
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::SendNextSessionId),
                 "NexMatchJointSessionJob::SendNextSessionId"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    m_leaveStationId = m_field1c0;
    m_field98 = 0;
    session::g_piaSession->m_sessionId[bufferSessionIndex()] = 0;
    m_field60 = cProcessStepWaitNextSessionId;
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitNextSessionId),
             "NexMatchJointSessionJob::WaitNextSessionId"));
    m_jointSessionState = cJointSessionStateWaitNextSessionId;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUntilLeaderLeavesMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (session::g_piaSession->m_stationIdStatusTable->CheckConnectStatus(m_leaveStationId))
    {
        if (IsMigrateJointSessionToSession() && m_field9f != 0)
        {
            setStep(&NexMatchJointSessionJob::StartLeavePreviousMesh,
                    "NexMatchJointSessionJob::StartLeavePreviousMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::StartLeavePreviousMesh, "NexMatchJointSessionJob::StartLeavePreviousMesh");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeavePreviousMesh()
{
    m_jointSessionState = cJointSessionStateLeavePreviousMesh;

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::g_piaSession->m_fieldc0 = 1;
    session::Mesh* mesh = session::g_piaMesh;
    if (mesh->CheckConnectionError() == cResultConnectionError)
    {
        if (IsMigrateJointSessionToSession())
        {
            setStep(&NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession,
                    "NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        m_time198 = deadlineAfter(cMeshRestartWaitMsec);
        setStep(&NexMatchJointSessionJob::WaitMeshRestart, "NexMatchJointSessionJob::WaitMeshRestart");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    Result leaveResult;
    if (!mesh->IsHost())
    {
        leaveResult = mesh->LeaveMeshAsync();
        m_field5c = cLeaveMeshTypeLeave;
    }
    else if (IsMigrateSessionToJointSession() || m_processType == session::Session::cProcessTypeDestroyJointSession)
    {
        leaveResult = mesh->DestroyMeshAsync();
        m_field5c = cLeaveMeshTypeDestroy;
    }
    else if (!session::g_piaSession->IsHostMigrationEnabled())
    {
        leaveResult = mesh->DestroyMeshAsync();
        m_field5c = cLeaveMeshTypeDestroy;
    }
    else
    {
        leaveResult = mesh->LeaveMeshWithHostMigrationAsync();
        m_field5c = cLeaveMeshTypeLeaveWithHostMigration;
    }

    if (leaveResult != 0)
    {
        if (IsMigrateJointSessionToSession())
        {
            setStep(&NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession,
                    "NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        m_time198 = deadlineAfter(cMeshRestartWaitMsec);
        setStep(&NexMatchJointSessionJob::WaitMeshRestart, "NexMatchJointSessionJob::WaitMeshRestart");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    setStep(&NexMatchJointSessionJob::WaitLeavePreviousMesh, "NexMatchJointSessionJob::WaitLeavePreviousMesh");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeavePreviousMesh()
{
    session::Mesh* mesh = session::g_piaMesh;

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    switch (m_field5c)
    {
    case cLeaveMeshTypeLeave:
        if (!mesh->IsLeaveMeshCompleted())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        mesh->GetLeaveMeshResult();
        break;
    case cLeaveMeshTypeLeaveWithHostMigration:
        if (!mesh->IsLeaveMeshWithHostMigrationCompleted())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        mesh->GetLeaveMeshWithHostMigrationResult();
        break;
    case cLeaveMeshTypeDestroy:
        if (!mesh->IsDestroyMeshCompleted())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        mesh->GetDestroyMeshResult();
        break;
    default:
        break;
    }

    session::g_piaMesh->Cleanup();
    if (IsMigrateJointSessionToSession())
    {
        m_fielda3 = 0;
        setStep(&NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession,
                "NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession");
    }
    else
    {
        m_time198 = deadlineAfter(cMeshRestartWaitMsec);
        setStep(&NexMatchJointSessionJob::WaitMeshRestart, "NexMatchJointSessionJob::WaitMeshRestart");
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CallDestroySessionEvent()
{
    if (session::g_piaMesh->CheckConnectionError() != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedSession);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown6)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown7)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }
    if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown8)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
            m_callerContext = 0;
        }
        m_failureResult = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    NoticeStartingEventByProcessType();
    session::g_piaSession->m_postponeNotice = true;
    session::g_piaMesh->SetSkipReport(true);
    m_time158 = common::g_piaJobScheduler->m_dispatchTime;
    m_time168 = common::g_piaJobScheduler->m_dispatchTime;

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_field1ec != 0)
    {
        m_fieldc8 = cFailurePointMeshSessionMismatch;
        m_failureResult = cResultInvalidState;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::CloseMatchmakeSession, "NexMatchJointSessionJob::CloseMatchmakeSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendDestroyInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (m_time188 < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_fieldc8 = cFailurePointInvitationTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();
    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_companionStationIdList.GetNodeNum() > 1)
    {
        unsigned long long stationIds[cStationNumMax - 1];
        unsigned short stationIdNum = 0;
        const common::ListNode* end = m_companionStationIdList.Sentinel();
        for (const common::ListNode* node = end->next; node != end; node = node->next)
            stationIds[stationIdNum++] = *StationIdList::ValueOf(node);

        Result sendResult = piaSession->m_sessionProtocol->SendDestroyInvitation(
            static_cast<unsigned char>(m_processType), stationIds, stationIdNum, piaSession->m_localStationId, 0, 0);
        if (sendResult != cResultSendBufferFull)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                unsigned short connectedNum = 0;
                for (unsigned short i = 0; i < stationIdNum; ++i)
                {
                    if (stationIds[i] != piaSession->m_localStationId
                        && piaSession->m_stationIdStatusTable->CheckConnectStatus(stationIds[i]))
                        ++connectedNum;
                }
                if (connectedNum != 0)
                {
                    m_fieldc8 = cFailurePointDestroyInvitationUndelivered;
                    m_failureResult = cResultCommunicationFailed;
                    unregisterBufferSearchCriteria();
                    RecordMonitoringDataWhenProcessFailure();
                    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
            }
            else if (sendResult != 0)
            {
                m_failureResult = sendResult;
                m_fieldc8 = cFailurePointSendError;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
    }

    m_field60 = cProcessStepWaitAnswerToDestroyInvitation;
    piaSession->m_stationIdStatusTable->ClearAllSendInvitationStatus();
    piaSession->m_stationIdStatusTable->ClearAllAnswerStatus();
    piaSession->m_stationIdStatusTable->SetAnswer(m_leaveStationId, true);
    piaSession->m_stationIdStatusTable->SetSendInvitationStatus(m_leaveStationId, true);
    m_time148 = common::g_piaJobScheduler->m_dispatchTime;
    m_time180 = deadlineAfter(cDestroyInvitationAnswerTimeoutMsec);
    setStep(&NexMatchJointSessionJob::WaitForAnswerToDestroyInvitation,
            "NexMatchJointSessionJob::WaitForAnswerToDestroyInvitation");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ResendDestroyInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    unsigned long long stationIds[cStationNumMax - 1];
    unsigned long long resendStationIds[cStationNumMax - 1];
    unsigned short stationIdNum = 0;
    unsigned short resendStationIdNum = 0;
    const common::ListNode* end = m_companionStationIdList.Sentinel();
    for (const common::ListNode* node = end->next; node != end; node = node->next)
    {
        unsigned long long stationId = *StationIdList::ValueOf(node);
        StationIndex stationIndex = cStationIndexInvalid;
        if (piaSession->m_stationIdStatusTable->GetStationIndex(stationId, &stationIndex)
            && session::g_piaMesh->IsValidStationIndex(stationIndex))
        {
            stationIds[stationIdNum++] = stationId;
            session::StationIdStatusTable::AnswerStatus answerStatus =
                piaSession->m_stationIdStatusTable->CheckAnswerStatus(stationId);
            bool invitationSent = piaSession->m_stationIdStatusTable->CheckSendInvitationStatus(stationId);
            if (answerStatus == session::StationIdStatusTable::cAnswerStatusNone && !invitationSent)
                resendStationIds[resendStationIdNum++] = stationId;
        }
    }

    if (resendStationIdNum != 0)
    {
        Result sendResult = piaSession->m_sessionProtocol->SendDestroyInvitation(
            static_cast<unsigned char>(m_processType), stationIds, stationIdNum, piaSession->m_localStationId,
            resendStationIds, resendStationIdNum);
        if (sendResult != cResultSendBufferFull && sendResult != 0)
        {
            if (sendResult == cResultNoInvitationTarget)
            {
                m_fieldc8 = cFailurePointDestroyInvitationUndelivered;
                m_failureResult = cResultCommunicationFailed;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
            m_failureResult = sendResult;
            m_fieldc8 = cFailurePointSendError;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }

    m_time148 = common::g_piaJobScheduler->m_dispatchTime;
    setStep(&NexMatchJointSessionJob::WaitForAnswerToDestroyInvitation,
            "NexMatchJointSessionJob::WaitForAnswerToDestroyInvitation");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForAnswerToDestroyInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    bool answerTimedOut = m_time180 < common::g_piaJobScheduler->m_dispatchTime;
    session::Session* piaSession = session::g_piaSession;
    bool allAnswered = true;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        unsigned long long stationId = *StationIdList::ValueOf(node);
        session::StationIdStatusTable::AnswerStatus answer = piaSession->m_stationIdStatusTable->CheckAnswerStatus(
            stationId);
        if (answer == session::StationIdStatusTable::cAnswerStatusNone)
        {
            if (piaSession->m_stationIdStatusTable->CheckConnectStatus(stationId) && !answerTimedOut)
                allAnswered = false;
        }
        else if (answer == session::StationIdStatusTable::cAnswerStatusFalse)
        {
            m_fieldc8 = cFailurePointDestroyInvitationRejected;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }

    if (!allAnswered)
    {
        if (isElapsed(m_time148, cResendInvitationIntervalMsec))
        {
            setStep(&NexMatchJointSessionJob::ResendDestroyInvitation,
                    "NexMatchJointSessionJob::ResendDestroyInvitation");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    PrepareBecomingToLeader();
    m_jointSessionState = cJointSessionStateCallSessionEvent;
    m_time188 = deadlineAfter(cInvitationAsCompanionTimeoutMsec);
    setStep(&NexMatchJointSessionJob::SendInvitationAsCompanion, "NexMatchJointSessionJob::SendInvitationAsCompanion");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendAnswerToDestroyInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    Result sendResult = piaSession->m_sessionProtocol->SendAnswerToDestroyInvitation(
        static_cast<unsigned char>(m_processType), piaSession->GetJointSessionHostStationId(), true);
    if (sendResult == cResultSendBufferFull)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (sendResult != 0)
    {
        m_fieldc8 = cFailurePointLeaderCommunication;
        m_failureResult = cResultCommunicationFailed;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
    {
        PrepareBecomingToLeader();
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::CloseMatchmakeSession),
            "NexMatchJointSessionJob::CloseMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    m_time160 = common::g_piaJobScheduler->m_dispatchTime;
    m_field60 = cProcessStepAnswerToInvitation;
    SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                            &NexMatchJointSessionJob::WaitForInvitationAsCompanion),
                        "NexMatchJointSessionJob::WaitForInvitationAsCompanion"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForInvitationAsCompanion()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    unsigned int hostSessionId;
    if (!piaSession->m_stationIdStatusTable->GetSessionId(piaSession->GetJointSessionHostStationId(), &hostSessionId))
        hostSessionId = 0;

    if (isElapsed(m_time160, cInvitationAsCompanionWaitTimeoutMsec))
    {
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] != hostSessionId)
        {
            m_leaveStationId =
                piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId();
            m_field60 = cProcessStepAnswerToInvitation;
            setStep(&NexMatchJointSessionJob::SendAnswerToInvitation,
                    "NexMatchJointSessionJob::SendAnswerToInvitation");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        m_fieldc8 = cFailurePointInvitationAsCompanionTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_field1b8 != 0)
    {
        m_field1b8 = 0;
        if (m_field1c0 == piaSession->m_localStationId)
        {
            PrepareBecomingToLeader();
            m_jointSessionState = cJointSessionStateCallSessionEvent;
            m_time188 = deadlineAfter(cInvitationAsCompanionTimeoutMsec);
            setStep(&NexMatchJointSessionJob::SendInvitationAsCompanion,
                    "NexMatchJointSessionJob::SendInvitationAsCompanion");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        if (session::g_piaSession->m_stationIdStatusTable->GetSessionId(m_field1c0, &hostSessionId)
            && hostSessionId != piaSession->m_sessionId[piaSession->m_currentSessionIndex]
            && piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
        {
            PrepareBecomingToLeader();
            m_jointSessionState = cJointSessionStateCallSessionEvent;
            m_time188 = deadlineAfter(cInvitationAsCompanionTimeoutMsec);
            setStep(&NexMatchJointSessionJob::SendInvitationAsCompanion,
                    "NexMatchJointSessionJob::SendInvitationAsCompanion");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
    }

    if (m_leaveStationId != 0)
    {
        m_field60 = cProcessStepAnswerToInvitation;
        setStep(&NexMatchJointSessionJob::SendAnswerToInvitation, "NexMatchJointSessionJob::SendAnswerToInvitation");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession()
{
    session::g_piaMesh->Cleanup();

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    unsigned int sessionId = session::g_piaSession->m_sessionId[bufferSessionIndex()];
    Result leaveResult;
    if (!session::g_piaSession->IsHostMigrationEnabled() && matchmakeSession->IsMatchmakeOwner())
    {
        m_field1eb = 1;
        leaveResult = matchmakeSession->UnregisterMatchmakeSession(&m_callContext, sessionId);
    }
    else
    {
        m_field1eb = 0;
        leaveResult = matchmakeSession->LeaveMatchmakeSession(&m_callContext, sessionId);
    }

    if (leaveResult == 0)
    {
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::WaitLeavePreviousMatchmakeSession),
                            "NexMatchJointSessionJob::WaitLeavePreviousMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    m_failureResult = leaveResult;
    m_fieldc8 = cFailurePointLeavePreviousMatchmakeSession;
    unregisterBufferSearchCriteria();
    RecordMonitoringDataWhenProcessFailure();
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
             "NexMatchJointSessionJob::StartLeaveMesh"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeavePreviousMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (m_field1eb == 0)
    {
        if (!matchmakeSession->WaitLeaveMatchmakeSession())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }
    else
    {
        if (!matchmakeSession->WaitUnregisterMatchmakeSession())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_sessionId[bufferSessionIndex()] = 0;
    session::g_piaSession->SetBufferSessionOpenStatus(session::Session::cSessionOpenStatusClosed);
    m_time198 = deadlineAfter(cMeshRestartWaitMsec);
    setStep(&NexMatchJointSessionJob::WaitMeshRestart, "NexMatchJointSessionJob::WaitMeshRestart");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitMeshRestart()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_time198 < common::g_piaJobScheduler->m_dispatchTime)
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::MeshRestart),
                 "NexMatchJointSessionJob::MeshRestart"));
    if (m_field9c != 0)
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::MeshRestart),
                 "NexMatchJointSessionJob::MeshRestart"));
    session::g_piaMesh->GetMonitoringDataSender()->SetSendSessionBeginReportRequested(false);
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

void NexMatchJointSessionJob::CheckJoinedOnlyCompanion()
{
    session::Session* piaSession = session::g_piaSession;
    const SessionParticipantInfo* participantInfo;
    if (IsMigrateSessionToJointSession())
    {
        participantInfo = static_cast<NexMatchMeshLayerController*>(piaSession->m_meshLayerController)
                              ->GetSessionParticipantInfo(piaSession->m_sessionId[bufferSessionIndex()]);
    }
    else
    {
        participantInfo = static_cast<NexMatchMeshLayerController*>(piaSession->m_meshLayerController)
                              ->GetSessionParticipantInfo(piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
    }

    bool otherStationJoined = false;
    if (participantInfo == 0)
    {
        otherStationJoined = true;
    }
    else
    {
        const common::ListNode* participantSentinel = participantInfo->m_participantList.Sentinel();
        for (const common::ListNode* participantNode = participantSentinel->next;
             participantNode != participantSentinel; participantNode = participantNode->next)
        {
            const SessionParticipantInfo::Participant* participant = SessionParticipantInfo::ParticipantList::ValueOf(
                participantNode);
            bool isCompanion = false;
            const common::ListNode* companionSentinel = m_companionStationIdList.Sentinel();
            for (const common::ListNode* companionNode = companionSentinel->next; companionNode != companionSentinel;
                 companionNode = companionNode->next)
            {
                if (participant->m_principalId == *StationIdList::ValueOf(companionNode)
                    && participant->m_participationCount > 0)
                {
                    isCompanion = true;
                    break;
                }
            }
            if (!isCompanion)
            {
                otherStationJoined = true;
            }
        }
    }
    m_field1ed = !otherStationJoined;
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::MeshRestart()
{
    session::g_piaMesh->Cleanup();

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    RemoveNotCompanionStationId();
    CheckJoinedOnlyCompanion();
    piaSession->m_stationIdStatusTable->ClearAllConnectStatus();
    piaSession->m_stationIdStatusTable->ClearAllMigrationCompleteStatus();
    session::g_piaSession->m_disconnectStatus = 0;
    m_time158 = common::g_piaJobScheduler->m_dispatchTime;

    session::IMatchmakeSession* matchmakeSession;
    if (IsMigrateSessionToJointSession())
    {
        NexFacade::GetInstance()->UpdateGatheringID(piaSession->m_sessionId[bufferSessionIndex()]);
        NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
            piaSession->m_meshLayerController);
        if (!meshLayerController->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            || !meshLayerController->CheckSessionId(piaSession->m_sessionId[bufferSessionIndex()]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }

        matchmakeSession = piaSession->m_matchmakeSession[bufferSessionIndex()];
        m_field9d = matchmakeSession->IsMatchmakeOwner()
                    && piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner();
        if (matchmakeSession->IsMatchmakeOwner())
        {
            m_field1d8 = 0;
            if (m_field1ed == 0
                || !piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
            {
                m_time178 = deadlineAfter(cRetryJoinIntervalHostMsec);
                if (++m_field1e9 > cConnectionInfoRetryMax)
                {
                    m_failureResult = cResultOwnerChanged;
                    unregisterBufferSearchCriteria();
                    RecordMonitoringDataWhenProcessFailure();
                    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
                setStep(&NexMatchJointSessionJob::WaitStartRetryJoinMesh,
                        "NexMatchJointSessionJob::WaitStartRetryJoinMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
            }
            if (!static_cast<NexMatchmakeSession*>(matchmakeSession)->IsMatchmakeHost())
                matchmakeSession->UpdateMatchmakeSessionHost(piaSession->m_sessionId[bufferSessionIndex()]);
        }
        else
        {
            unsigned long long ownerUniqueId = matchmakeSession->GetMatchmakeOwnerUniqueId();
            m_field1d8 = 0;
            unsigned int ownerSessionId;
            if (piaSession->m_stationIdStatusTable->GetSessionId(ownerUniqueId, &ownerSessionId)
                && piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner()
                && ownerSessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            {
                if (!piaSession->m_meshLayerController->IsParticipated(
                        piaSession->m_sessionId[piaSession->m_currentSessionIndex], ownerUniqueId))
                {
                    piaSession->m_meshLayerController->Trace(cTraceJointSessionLevel);
                    m_failureResult = cResultOwnerChanged;
                    unregisterBufferSearchCriteria();
                    RecordMonitoringDataWhenProcessFailure();
                    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
                if (m_field1ed == 0)
                {
                    m_time178 = deadlineAfter(cRetryJoinIntervalHostMsec);
                    if (++m_field1e9 > cConnectionInfoRetryMax)
                    {
                        m_failureResult = cResultOwnerChanged;
                        unregisterBufferSearchCriteria();
                        RecordMonitoringDataWhenProcessFailure();
                        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                    }
                    setStep(&NexMatchJointSessionJob::WaitStartRetryJoinMesh,
                            "NexMatchJointSessionJob::WaitStartRetryJoinMesh");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
                }
                matchmakeSession->UpdateMatchmakeSessionHost(piaSession->m_sessionId[bufferSessionIndex()]);
                m_field9d = 1;
            }
        }
    }
    else
    {
        NexFacade::GetInstance()->UpdateGatheringID(piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
        m_field1c8 = 0;
        if (!static_cast<NexMatchMeshLayerController*>(piaSession->m_meshLayerController)
                 ->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }

        matchmakeSession = piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex];
        m_field9d = matchmakeSession->IsMatchmakeOwner();
        if (!m_field9d)
        {
            m_field9c = 0;
            m_leaveStationId = 0;
        }
        else if (m_field9c == 0)
        {
            bool leaveStationIsCompanion = false;
            common::ListNode* sentinel = m_companionStationIdList.Sentinel();
            for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
            {
                if (m_leaveStationId == *StationIdList::ValueOf(node))
                {
                    leaveStationIsCompanion = true;
                    break;
                }
            }

            if (leaveStationIsCompanion)
            {
                m_field9d = 0;
            }
            else
            {
                m_field9c = 1;
                m_leaveStationId = piaSession->m_localStationId;
                if (!static_cast<NexMatchmakeSession*>(matchmakeSession)->IsMatchmakeHost())
                {
                    Result updateResult = matchmakeSession->UpdateMatchmakeSessionHost(
                        piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
                    if (updateResult != 0)
                    {
                        m_fieldc8 = cFailurePointUpdateHostFailed;
                        m_failureResult = cResultRequestFailed;
                        unregisterBufferSearchCriteria();
                        RecordMonitoringDataWhenProcessFailure();
                        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
                        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                    }
                }
            }
        }
    }

    Result startupResult = piaSession->m_meshLayerController->MeshStartup(matchmakeSession, true);
    if (startupResult != 0)
    {
        m_failureResult = startupResult;
        m_fieldc8 = cFailurePointMeshStartupFailed;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    m_field1ea = 0;
    m_retryReasonB = 0;
    m_sessionInfoRetry = 0;
    if (m_field9d == 0)
    {
        setStep(&NexMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo,
                "NexMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo");
    }
    else
    {
        m_jointSessionState = cJointSessionStateCreateNextMesh;
        setStep(&NexMatchJointSessionJob::StartCreateNextMesh, "NexMatchJointSessionJob::StartCreateNextMesh");
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartCreateNextMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_fieldc0 = 0;
    Result createResult = session::g_piaMesh->CreateMeshAsync();
    if (createResult != 0)
    {
        m_failureResult = createResult;
        m_fieldc8 = cFailurePointCreateNextMesh;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaMesh->SetSkipReport(true);
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitCreateNextMesh),
             "NexMatchJointSessionJob::WaitCreateNextMesh"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCreateNextMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (!session::g_piaMesh->IsCreateMeshCompleted())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    Result createResult = session::g_piaMesh->GetCreateMeshResult();
    if (createResult != 0)
    {
        m_failureResult = createResult;
        m_fieldc8 = cFailurePointCreateNextMesh;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (IsMigrateSessionToJointSession())
    {
        transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
            session::g_piaSession->m_fieldc2[bufferSessionIndex()]);
        m_fielda3 = 1;
    }
    else
    {
        transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
            session::g_piaSession->m_fieldc2[session::g_piaSession->m_currentSessionIndex]);
    }
    session::g_piaSession->m_stationIdStatusTable->SetMigrationCompleteStatus(session::g_piaSession->m_localStationId,
                                                                              true);
    session::g_piaSession->m_disconnectStatus = 1;
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitCompanionStation),
             "NexMatchJointSessionJob::WaitCompanionStation"));
    m_jointSessionState = cJointSessionStateWaitCompanionStation;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::IMatchmakeSession* matchmakeSession;
    if (IsMigrateSessionToJointSession())
        matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    else
        matchmakeSession = session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];

    transport::StationConnectionInfo emptyConnectionInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(emptyConnectionInfo);
    Result requestResult = matchmakeSession->GetStationConnectionInfo(&m_callContext, m_field98);
    if (requestResult == 0)
    {
        setStep(&NexMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo,
                "NexMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    m_failureResult = requestResult;
    m_fieldc8 = cFailurePointGetStationConnectionInfo;
    unregisterBufferSearchCriteria();
    RecordMonitoringDataWhenProcessFailure();
    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    session::IMatchmakeSession* matchmakeSession;
    if (IsMigrateSessionToJointSession())
        matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    else
        matchmakeSession = session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];

    if (!matchmakeSession->WaitGetStationConnectionInfo(&m_stationConnectionInfo))
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callContext.m_result == 0)
    {
        m_stationConnectionInfo.Trace(cTraceJointSessionLevel);
        unsigned long long hostPrincipalId = m_stationConnectionInfo.m_stationLocation.m_principalId;
        if (hostPrincipalId == 0)
        {
            hostPrincipalId = m_stationConnectionInfo.m_localStationLocation.m_principalId;
            if (hostPrincipalId == 0)
            {
                m_fieldc8 = cFailurePointGetStationConnectionInfo;
                m_failureResult = cResultOwnerChanged;
                unregisterBufferSearchCriteria();
                RecordMonitoringDataWhenProcessFailure();
                SetCurrentStep(Step(
                    static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                    "NexMatchJointSessionJob::StartLeaveMesh"));
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }

        if (matchmakeSession->GetMatchmakeOwnerUniqueId() != hostPrincipalId)
        {
            if (IsMigrateSessionToJointSession())
                matchmakeSession->SetMatchmakeOwnerUniqueId(hostPrincipalId);
            else
                matchmakeSession->SetMatchmakeOwnerUniqueId(hostPrincipalId);
        }
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartJoinNextMesh),
                 "NexMatchJointSessionJob::StartJoinNextMesh"));
        m_jointSessionState = cJointSessionStateJoinNextMesh;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (m_callContext.m_result != cResultRetryJoinMesh)
    {
        m_fieldc8 = cFailurePointGetStationConnectionInfo;
        m_failureResult = m_callContext.m_result;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (++m_field1e9 > cConnectionInfoRetryMax)
    {
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (IsMigrateSessionToJointSession())
    {
        if (!meshLayerController->CheckSessionId(piaSession->m_sessionId[bufferSessionIndex()]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        if (m_field1d8 == 0)
        {
            m_time178 = deadlineAfter(cRetryJoinIntervalHostMsec);
        }
        else if (piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
        {
            SetCurrentStep(
                Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::MeshRestart),
                     "NexMatchJointSessionJob::MeshRestart"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        else
        {
            m_time178 = deadlineAfter(cRetryJoinIntervalMsec);
        }
    }
    else
    {
        if (!meshLayerController->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        if (m_field1c8 != 0)
            m_time178 = deadlineAfter(cRetryJoinIntervalMsec);
        else
            m_time178 = deadlineAfter(cRetryJoinIntervalHostMsec);
    }
    SetCurrentStep(Step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitStartRetryJoinMesh),
        "NexMatchJointSessionJob::WaitStartRetryJoinMesh"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartJoinNextMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_fieldc0 = 0;
    Result joinResult = session::g_piaMesh->JoinMeshAsync(m_stationConnectionInfo);
    if (joinResult != 0)
    {
        m_failureResult = joinResult;
        m_fieldc8 = cFailurePointJoinMeshOther;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaMesh->SetSkipReport(true);
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitJoinNextMesh),
             "NexMatchJointSessionJob::WaitJoinNextMesh"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitJoinNextMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* session = session::g_piaSession;
    if (!session::g_piaMesh->IsJoinMeshCompleted())
    {
        if (m_failurePending != 0)
        {
            session::g_piaMesh->CancelJoinMeshAsync();
        }
        else if (m_sessionInfoRetry != 0)
        {
            session::g_piaMesh->CancelJoinMeshAsync();
        }
        else if (m_retryReasonB != 0)
        {
            session::g_piaMesh->CancelJoinMeshAsync();
        }

        if ((IsMigrateSessionToJointSession() && m_field1d8 != 0)
            || (IsMigrateJointSessionToSession() && m_field1c8 != 0))
        {
            session::g_piaMesh->CancelJoinMeshAsync();
            if (m_field1e8 != cJoinMeshRetryMax)
                m_field1e8 = 0;
        }
    }

    if (!session::g_piaMesh->IsJoinMeshCompleted())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    Result joinResult = session::g_piaMesh->GetJoinMeshResult();
    if (joinResult != 0)
    {
        if (joinResult == cResultJoinMesh643F || joinResult == cResultJoinMesh6445 || joinResult == cResultJoinMesh6444
            || joinResult == cResultJoinMesh6443 || joinResult == cResultJoinMesh6439
            || joinResult == cResultJoinMesh644A || (joinResult == cResultJoinMesh6C05 && m_failurePending == 0))
        {
            countRetry(common::g_sessionBeginMonitoringContent.m_joinRetryCount);
            ++m_field1e8;
            if (m_field1e8 < cJoinMeshRetryMax)
            {
                session::g_piaSession->m_fieldc0 = 1;
                session::g_piaSession->m_disconnectStatus = 0;
                session::g_piaMesh->Cleanup();
                if (IsMigrateSessionToJointSession() && m_field1d8 != 0
                    && session->m_matchmakeSession[session->m_currentSessionIndex]->IsMatchmakeOwner())
                {
                    countRetry(common::g_sessionBeginMonitoringContent.m_ownerChangeRetryCount);
                    setStep(&NexMatchJointSessionJob::MeshRestart, "NexMatchJointSessionJob::MeshRestart");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
                if (m_sessionInfoRetry == 0)
                {
                    m_time178 = deadlineAfter(cRetryJoinIntervalMsec);
                    if (joinResult == cResultJoinMesh6439 || joinResult == cResultJoinMesh643F
                        || joinResult == cResultJoinMesh6442 || joinResult == cResultJoinMesh6445
                        || joinResult == cResultJoinMesh6443 || joinResult == cResultJoinMesh6444
                        || joinResult == cResultJoinMesh6467 || joinResult == cResultJoinMesh646A
                        || joinResult == cResultJoinMesh6468 || joinResult == cResultJoinMesh6469
                        || joinResult == cResultJoinMesh644A)
                    {
                        countRetry(common::g_sessionBeginMonitoringContent.m_retryReasonACount);
                    }
                    else if (m_retryReasonB != 0)
                    {
                        countRetry(common::g_sessionBeginMonitoringContent.m_retryReasonBCount);
                    }
                }
                else
                {
                    m_time178 = deadlineAfter(cRetryJoinIntervalHostMsec);
                    countRetry(common::g_sessionBeginMonitoringContent.m_sessionInfoRetryCount);
                }
                setStep(&NexMatchJointSessionJob::WaitStartRetryJoinMesh,
                        "NexMatchJointSessionJob::WaitStartRetryJoinMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }

        if (m_failurePending == 0)
        {
            if (joinResult == cResultJoinMesh6C05)
                m_fieldc8 = cFailurePointJoinMesh6C05;
            else if (joinResult == cResultInvalidState)
                m_fieldc8 = cFailurePointInvalidState;
            else if (joinResult == cResultJoinMesh6439)
                m_fieldc8 = cFailurePointJoinMesh6439;
            else if (joinResult == cResultJoinMesh643F)
                m_fieldc8 = cFailurePointJoinMesh643F;
            else if (joinResult == cResultJoinMesh6442 || joinResult == cResultJoinMesh6467)
                m_fieldc8 = cFailurePointJoinMesh6442;
            else if (joinResult == cResultJoinMesh6445 || joinResult == cResultJoinMesh646A)
                m_fieldc8 = cFailurePointJoinMesh6445;
            else if (joinResult == cResultJoinMesh6443 || joinResult == cResultJoinMesh6468)
                m_fieldc8 = cFailurePointJoinMesh6443;
            else if (joinResult == cResultJoinMesh6444 || joinResult == cResultJoinMesh6469)
                m_fieldc8 = cFailurePointJoinMesh6444;
            else if (joinResult == cResultJoinMesh644A)
                m_fieldc8 = cFailurePointJoinMesh644A;
            else if (joinResult == cResultJoinMesh6838)
                m_fieldc8 = cFailurePointJoinMesh6838;
            else if (joinResult == cResultJoinMesh6C3D)
                m_fieldc8 = cFailurePointJoinMesh6C3D;
            else if (joinResult == cResultJoinMesh6846)
                m_fieldc8 = cFailurePointJoinMesh6846;
            else if (joinResult == cResultJoinMesh6847)
                m_fieldc8 = cFailurePointJoinMesh6847;
            else if (joinResult == cResultJoinMesh6848)
                m_fieldc8 = cFailurePointJoinMesh6848;
            else if (joinResult == cResultJoinMesh6849)
                m_fieldc8 = cFailurePointJoinMesh6849;
            else if (joinResult == cResultJoinMesh10C3E)
                m_fieldc8 = cFailurePointJoinMesh10C3E;
            else if (joinResult == cResultJoinMesh10C25)
                m_fieldc8 = cFailurePointJoinMesh10C25;
            else
                m_fieldc8 = cFailurePointJoinMeshOther;
        }
        else
        {
            m_fieldc8 = cFailurePointDisconnectDetected;
        }
        if (m_failurePending != 0)
            joinResult = m_pendingFailureResult;
        m_failureResult = joinResult;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (IsMigrateSessionToJointSession())
        m_fielda3 = 1;
    session->m_stationIdStatusTable->SetMigrationCompleteStatus(session->m_localStationId, true);
    session::g_piaSession->m_disconnectStatus = 1;
    setStep(&NexMatchJointSessionJob::WaitCompanionStation, "NexMatchJointSessionJob::WaitCompanionStation");
    m_jointSessionState = cJointSessionStateWaitCompanionStation;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitStartRetryJoinMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Session* piaSession = session::g_piaSession;
    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (IsMigrateSessionToJointSession())
    {
        if (m_field1d8 != 0)
            m_field1e8 = 0;
        else if (common::g_piaJobScheduler->m_dispatchTime <= m_time178)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

        countRetry(common::g_sessionBeginMonitoringContent.m_ownerChangeRetryCount);
        if (piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
        {
            setStep(&NexMatchJointSessionJob::MeshRestart, "NexMatchJointSessionJob::MeshRestart");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        setStep(&NexMatchJointSessionJob::MeshRestart, "NexMatchJointSessionJob::MeshRestart");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (m_field1c8 != 0)
        m_field1e8 = 0;
    else if (common::g_piaJobScheduler->m_dispatchTime <= m_time178)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    countRetry(common::g_sessionBeginMonitoringContent.m_ownerChangeRetryCount);
    setStep(&NexMatchJointSessionJob::MeshRestart, "NexMatchJointSessionJob::MeshRestart");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCompanionStation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (m_companionStationIdList.GetNodeNum() < cCompanionStationNumMin)
    {
        setStep(&NexMatchJointSessionJob::WaitHostStationId, "NexMatchJointSessionJob::WaitHostStationId");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    session::Session* piaSession = session::g_piaSession;
    if (isElapsed(m_time158, cCompanionTimeoutMsec))
    {
        m_fieldc8 = cFailurePointCompanionTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    NexMatchMeshLayerController* controller = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (!controller->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex]))
    {
        m_fieldc8 = cFailurePointSessionIdMismatch;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (IsMigrateSessionToJointSession())
    {
        if (!controller->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            || !controller->CheckSessionId(piaSession->m_sessionId[bufferSessionIndex()]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }
    else if (!controller->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex]))
    {
        m_fieldc8 = cFailurePointSessionIdMismatch;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (!piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner()
        && piaSession->m_localStationId == piaSession->m_hostStationId)
    {
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    unsigned long long stationIds[cStationNumMax];
    unsigned short stationIdNum = 0;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        stationIds[stationIdNum] = *StationIdList::ValueOf(node);
        ++stationIdNum;
    }
    if (!piaSession->m_stationIdStatusTable->CheckAllConnectStatus(stationIds, stationIdNum))
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    transport::StationIdTable::Entry ownerEntry;
    if (transport::g_piaTransport->m_stationIdTable->FindByUniqueId(
            &ownerEntry, piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId())
        != 0)
    {
        m_fieldc8 = cFailurePointNoOwnerEntry;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
    {
        m_field60 = cProcessStepWaitUntilCompanionCompletion;
        m_time158 = common::g_piaJobScheduler->m_dispatchTime;
        setStep(&NexMatchJointSessionJob::WaitUntilCompanionCompletion,
                "NexMatchJointSessionJob::WaitUntilCompanionCompletion");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    m_leaveStationId = ownerEntry.stationId;
    m_time190 = deadlineAfter(cCompletionTimeoutMsec);
    setStep(&NexMatchJointSessionJob::SendCompletionForMigrateSession,
            "NexMatchJointSessionJob::SendCompletionForMigrateSession");
    m_jointSessionState = cJointSessionStateSendCompletion;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUntilCompanionCompletion()
{
    session::Session* piaSession = session::g_piaSession;
    if (isElapsed(m_time158, cCompanionTimeoutMsec))
    {
        m_fieldc8 = cFailurePointCompanionTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    unsigned long long stationIds[cStationNumMax];
    unsigned short stationIdNum = 0;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        stationIds[stationIdNum] = *StationIdList::ValueOf(node);
        ++stationIdNum;
    }
    if (!piaSession->m_stationIdStatusTable->CheckAllConnectStatus(stationIds, stationIdNum))
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (!piaSession->m_stationIdStatusTable->CheckMigrationCompleteStatus(*StationIdList::ValueOf(node)))
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::SendCompletionInvitation, "NexMatchJointSessionJob::SendCompletionInvitation");
    m_jointSessionState = cJointSessionStateSendCompletionInvitation;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendCompletionForMigrateSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (m_time190 < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_fieldc8 = cFailurePointCompletionForMigrateTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (piaSession->m_stationIdStatusTable->CheckConnectStatus(m_leaveStationId))
    {
        if (piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId()
            != m_leaveStationId)
        {
            if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
            {
                m_time158 = common::g_piaJobScheduler->m_dispatchTime;
                SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                        &NexMatchJointSessionJob::WaitCompanionStation),
                                    "NexMatchJointSessionJob::WaitCompanionStation"));
                m_jointSessionState = cJointSessionStateWaitCompanionStation;
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
            }
            m_time158 = timeBefore(cCompanionRewaitElapsedMsec);
            m_jointSessionState = cJointSessionStateWaitCompanionStation;
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &NexMatchJointSessionJob::WaitCompanionStation),
                                "NexMatchJointSessionJob::WaitCompanionStation"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }

        Result sendResult = piaSession->m_sessionProtocol->SendCompletionForMigrateSession(
            static_cast<unsigned char>(m_processType), m_leaveStationId, true);
        if (sendResult == cResultSendBufferFull)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

        if (sendResult != 0)
        {
            m_fieldc8 = cFailurePointLeaderCommunication;
            m_failureResult = cResultCommunicationFailed;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            SetCurrentStep(Step(
                static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                "NexMatchJointSessionJob::StartLeaveMesh"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }

        m_fielda0 = 0;
        m_field60 = cProcessStepWaitCompletionInvitation;
        m_completionDeadline = deadlineAfter(cCompletionTimeoutMsec);
        m_jointSessionState = cJointSessionStateWaitCompletionInvitation;
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::WaitCompletionInvitation),
                            "NexMatchJointSessionJob::WaitCompletionInvitation"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (!piaSession->IsHostMigrationEnabled())
    {
        m_fieldc8 = cFailurePointLeaderCommunication;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        m_fieldc8 = cFailurePointConnectionError;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::StartLeaveMesh),
                 "NexMatchJointSessionJob::StartLeaveMesh"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_field1c8 == 0)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    m_field1c8 = 0;
    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
    {
        m_time158 = common::g_piaJobScheduler->m_dispatchTime;
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitCompanionStation),
            "NexMatchJointSessionJob::WaitCompanionStation"));
        m_jointSessionState = cJointSessionStateWaitCompanionStation;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }
    m_time158 = timeBefore(cCompanionRewaitElapsedMsec);
    m_jointSessionState = cJointSessionStateWaitCompanionStation;
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitCompanionStation),
             "NexMatchJointSessionJob::WaitCompanionStation"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendCompletionInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* piaSession = session::g_piaSession;
    unsigned long long stationIds[cStationNumMax];
    unsigned short stationIdNum = 0;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (piaSession->m_stationIdStatusTable->CheckConnectStatus(*StationIdList::ValueOf(node)))
        {
            stationIds[stationIdNum] = *StationIdList::ValueOf(node);
            ++stationIdNum;
        }
    }

    Result sendResult = piaSession->m_sessionProtocol->SendCompletionInvitation(
        static_cast<unsigned char>(m_processType), stationIds, stationIdNum, piaSession->m_localStationId, 0, 0);
    if (sendResult != 0 && sendResult == session::Session::cResultStatusDisconnectedNetwork)
    {
        m_fieldc8 = cFailurePointSessionStatus;
        m_failureResult = session::Session::cResultStatusDisconnectedNetwork;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    m_field60 = cProcessStepWaitForAnswerToCompletionInvitation;
    piaSession->m_stationIdStatusTable->ClearAllAnswerStatus();
    piaSession->m_stationIdStatusTable->SetAnswer(piaSession->m_localStationId, true);
    m_jointSessionState = cJointSessionStateWaitForAnswerToCompletionInvitation;
    m_time180 = deadlineAfter(cAnswerToCompletionTimeoutMsec);
    setStep(&NexMatchJointSessionJob::WaitForAnswerToCompletionInvitation,
            "NexMatchJointSessionJob::WaitForAnswerToCompletionInvitation");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForAnswerToCompletionInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess
            || m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateFinished;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    if (m_time180 < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_fieldc8 = cFailurePointAnswerToCompletionTimeout;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    bool allAnswered = true;
    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (piaSession->m_stationIdStatusTable->CheckAnswerStatus(*StationIdList::ValueOf(node))
                == session::StationIdStatusTable::cAnswerStatusNone
            && piaSession->m_stationIdStatusTable->CheckConnectStatus(*StationIdList::ValueOf(node)))
        {
            allAnswered = false;
        }
    }
    if (!allAnswered)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    setStep(&NexMatchJointSessionJob::WaitHostStationId, "NexMatchJointSessionJob::WaitHostStationId");
    m_fielda0 = 1;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCompletionInvitation()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_stationIdStatusTable->CheckConnectStatus(m_leaveStationId))
    {
        if (piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId()
            == m_leaveStationId)
        {
            if (m_fielda0 == 0)
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            setStep(&NexMatchJointSessionJob::WaitHostStationId, "NexMatchJointSessionJob::WaitHostStationId");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }

        if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        {
            m_time158 = common::g_piaJobScheduler->m_dispatchTime;
            setStep(&NexMatchJointSessionJob::WaitCompanionStation, "NexMatchJointSessionJob::WaitCompanionStation");
            m_jointSessionState = cJointSessionStateWaitCompanionStation;
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        m_time158 = timeBefore(cCompanionRewaitElapsedMsec);
        m_jointSessionState = cJointSessionStateWaitCompanionStation;
        setStep(&NexMatchJointSessionJob::WaitCompanionStation, "NexMatchJointSessionJob::WaitCompanionStation");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (!piaSession->IsHostMigrationEnabled())
    {
        m_fieldc8 = cFailurePointLeaderCommunication;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (session::g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        m_fieldc8 = cFailurePointConnectionError;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_field1c8 == 0)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    m_field1c8 = 0;
    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
    {
        m_time158 = common::g_piaJobScheduler->m_dispatchTime;
        setStep(&NexMatchJointSessionJob::WaitCompanionStation, "NexMatchJointSessionJob::WaitCompanionStation");
        m_jointSessionState = cJointSessionStateWaitCompanionStation;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }
    m_time158 = timeBefore(cCompanionRewaitElapsedMsec);
    setStep(&NexMatchJointSessionJob::WaitCompanionStation, "NexMatchJointSessionJob::WaitCompanionStation");
    m_jointSessionState = cJointSessionStateWaitCompanionStation;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitHostStationId()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    session::Session* piaSession = session::g_piaSession;
    StationIndex hostStationIndex = session::g_piaMesh->GetHostStationIndex();
    session::IMatchmakeSession* currentSession = piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex];
    session::IMatchmakeSession* bufferSession = piaSession->m_matchmakeSession[bufferSessionIndex()];
    NexMatchMeshLayerController* controller = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    transport::StationIdTable::Entry entry;
    unsigned long long principalId;
    unsigned long long ownerStationId;

    if (!IsMigrateSessionToJointSession())
    {
        if (!controller->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }

        transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(hostStationIndex);
        if (hostStation == 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        if (hostStation->GetPrincipalId(&principalId) != 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        if (transport::g_piaTransport->m_stationIdTable->FindByUniqueId(&entry, principalId) != 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        ownerStationId = entry.stationId;
        if (!piaSession->m_stationIdStatusTable->CheckConnectStatus(ownerStationId))
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

        if (ownerStationId == piaSession->m_localStationId)
        {
            if (!currentSession->IsMatchmakeOwner())
            {
                if (isElapsed(m_time168, cHostStationUpdateIntervalMsec))
                {
                    m_time168 = common::g_piaJobScheduler->m_dispatchTime;
                    currentSession->UpdateMatchmakeSessionHost(
                        piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
                }
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
        else if (currentSession->GetMatchmakeOwnerUniqueId() != ownerStationId)
        {
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }
    else
    {
        if (!controller->CheckSessionId(piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            || !controller->CheckSessionId(piaSession->m_sessionId[bufferSessionIndex()]))
        {
            m_fieldc8 = cFailurePointSessionIdMismatch;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }

        transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(hostStationIndex);
        if (hostStation == 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        if (hostStation->GetPrincipalId(&principalId) != 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        if (transport::g_piaTransport->m_stationIdTable->FindByUniqueId(&entry, principalId) != 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        unsigned long long hostStationId = entry.stationId;
        if (!piaSession->m_stationIdStatusTable->CheckConnectStatus(hostStationId))
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

        if (transport::g_piaTransport->m_stationIdTable->FindByUniqueId(&entry,
                                                                        currentSession->GetMatchmakeOwnerUniqueId())
            != 0)
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        ownerStationId = entry.stationId;
        if (!piaSession->m_stationIdStatusTable->CheckConnectStatus(ownerStationId))
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

        if (piaSession->m_localStationId == hostStationId)
        {
            if (!bufferSession->IsMatchmakeOwner())
            {
                if (isElapsed(m_time168, cHostStationUpdateIntervalMsec))
                {
                    m_time168 = common::g_piaJobScheduler->m_dispatchTime;
                    bufferSession->UpdateMatchmakeSessionHost(piaSession->m_sessionId[bufferSessionIndex()]);
                }
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }

            if (!currentSession->IsMatchmakeOwner())
            {
                if (isElapsed(m_time168, cHostStationUpdateIntervalMsec))
                {
                    m_time168 = common::g_piaJobScheduler->m_dispatchTime;
                    currentSession->UpdateMatchmakeSessionHost(
                        piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
                }
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }

            if (m_field1c8 != 0)
            {
                if (isElapsed(m_time168, cHostStationUpdateIntervalMsec))
                {
                    m_field1c8 = 0;
                    m_time168 = common::g_piaJobScheduler->m_dispatchTime;
                    currentSession->UpdateMatchmakeSessionHost(
                        piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
                }
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }

            if (piaSession->m_sessionOpenStatus[bufferSessionIndex()] == session::Session::cSessionOpenStatusClosed)
            {
                setStep(&NexMatchJointSessionJob::UpdateJointSessionOpenStatus,
                        "NexMatchJointSessionJob::UpdateJointSessionOpenStatus");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
            }
        }
        else
        {
            if (bufferSession->GetMatchmakeOwnerUniqueId() != hostStationId)
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

            unsigned int hostSessionId;
            piaSession->m_stationIdStatusTable->GetSessionId(hostStationId, &hostSessionId);
            if (hostSessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            {
                if (currentSession->GetMatchmakeOwnerUniqueId() != hostStationId)
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
            else if (ownerStationId == piaSession->m_localStationId
                     && !static_cast<NexMatchmakeSession*>(currentSession)->IsMatchmakeHost())
            {
                currentSession->UpdateMatchmakeSessionHost(piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
            }
        }
    }

    if (ownerStationId != piaSession->m_hostStationId
        && piaSession->m_stationIdStatusTable->CheckConnectStatus(piaSession->m_hostStationId))
    {
        unsigned int meshHostSessionId = 0;
        unsigned int ownerSessionId = 0;
        if (!piaSession->m_stationIdStatusTable->GetSessionId(piaSession->m_hostStationId, &meshHostSessionId)
            || !piaSession->m_stationIdStatusTable->GetSessionId(ownerStationId, &ownerSessionId)
            || meshHostSessionId == ownerSessionId)
        {
            m_fieldc8 = cFailurePointSameSessionId;
            m_failureResult = cResultOwnerChanged;
            unregisterBufferSearchCriteria();
            RecordMonitoringDataWhenProcessFailure();
            setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
    }

    setStep(&NexMatchJointSessionJob::ProcessComplete, "NexMatchJointSessionJob::ProcessComplete");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::UpdateJointSessionOpenStatus()
{
    session::IMatchmakeSession* bufferSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (bufferSession->RequestSessionInfo(&m_callContext, m_field98) != 0)
    {
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    setStep(&NexMatchJointSessionJob::WaitUpdateJointSessionOpenStatus,
            "NexMatchJointSessionJob::WaitUpdateJointSessionOpenStatus");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUpdateJointSessionOpenStatus()
{
    session::IMatchmakeSession* bufferSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (!bufferSession->WaitRequestSessionInfo())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result == 0)
    {
        session::Session::SessionOpenStatus openStatus = session::Session::cSessionOpenStatusParticipationClosed;
        if (bufferSession->GetSessionInfo()->IsOpened())
            openStatus = session::Session::cSessionOpenStatusOpen;
        session::g_piaSession->SetBufferSessionOpenStatus(openStatus);
        setStep(&NexMatchJointSessionJob::ProcessComplete, "NexMatchJointSessionJob::ProcessComplete");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    m_failureResult = cResultOwnerChanged;
    unregisterBufferSearchCriteria();
    RecordMonitoringDataWhenProcessFailure();
    setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ProcessComplete()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    session::g_piaSession->m_postponeNotice = false;
    if (!ClearProcessTypeAndNoticeEndingEvent())
    {
        session::g_piaSession->m_postponeNotice = true;
        m_fieldc8 = cFailurePointNoticeEndingEventFailed;
        m_failureResult = cResultOwnerChanged;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    m_jointSessionState = cJointSessionStateFinished;
    session::g_piaMesh->GetMonitoringDataSender()->SetSendSessionBeginReportRequested(true);
    session::g_piaMesh->SetSkipReport(false);
    session::g_piaMesh->SendReport(cReportReasonJointSession);
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::GetMatchmakeSessionOwners()
{
    session::Session* piaSession = session::g_piaSession;

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (session::g_piaMesh->CheckConnectionError() != 0
        || session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedSession)
    {
        m_failureResult = cResultOwnerChanged;
        m_fieldc8 = cFailurePointConnectionError;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_failurePending != 0)
    {
        m_fieldc8 = cFailurePointDisconnectDetected;
        m_failureResult = m_pendingFailureResult;
        RecordMonitoringDataWhenProcessFailure();
        setStep(&NexMatchJointSessionJob::StartLeaveMesh, "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    for (unsigned char i = 0; i < m_sessionIdNum; ++i)
    {
        if (m_sessionIds[i] != piaSession->m_sessionId[piaSession->m_currentSessionIndex] && !m_sessionInfoRequested[i])
        {
            m_sessionInfoRequested[i] = true;
            Result requestResult = static_cast<NexMatchmakeSession*>(
                                       piaSession->m_matchmakeSession[bufferSessionIndex()])
                                       ->RequestSessionInfoWithoutPermission(&m_callContext, m_sessionIds[i]);
            if (requestResult == 0)
            {
                setStep(&NexMatchJointSessionJob::WaitGetMatchmakeSessionOwners,
                        "NexMatchJointSessionJob::WaitGetMatchmakeSessionOwners");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }
    }

    m_jointSessionState = cJointSessionStateCallSessionEvent;
    setDeadline(m_time188, cInvitationAsCompanionTimeoutMsec);
    setStep(&NexMatchJointSessionJob::SendInvitationAsCompanion, "NexMatchJointSessionJob::SendInvitationAsCompanion");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitGetMatchmakeSessionOwners()
{
    session::Session* piaSession = session::g_piaSession;
    NexMatchmakeSession* matchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[bufferSessionIndex()]);

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    if (!matchmakeSession->WaitRequestSessionInfoWithoutPermission())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    if (m_callContext.m_result != 0)
    {
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::GetMatchmakeSessionOwners),
                            "NexMatchJointSessionJob::GetMatchmakeSessionOwners"));
    }
    piaSession->m_stationIdStatusTable->SetSessionHost(matchmakeSession->GetMatchmakeOwnerUniqueId(), true);
    SetCurrentStep(Step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::GetMatchmakeSessionOwners),
        "NexMatchJointSessionJob::GetMatchmakeSessionOwners"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeaveMesh()
{
    m_sessionDisconnectDetected = true;
    session::g_piaSession->m_fieldc0 = 1;

    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Mesh* mesh = session::g_piaMesh;
    if (mesh->CheckConnectionError() == cResultConnectionError)
    {
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession),
                            "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    Result leaveResult;
    if (!mesh->IsHost())
    {
        leaveResult = mesh->LeaveMeshAsync();
        m_field5c = cLeaveMeshTypeLeave;
    }
    else if (!session::g_piaSession->IsHostMigrationEnabled())
    {
        leaveResult = mesh->DestroyMeshAsync();
        m_field5c = cLeaveMeshTypeDestroy;
    }
    else
    {
        leaveResult = mesh->LeaveMeshWithHostMigrationAsync();
        m_field5c = cLeaveMeshTypeLeaveWithHostMigration;
    }

    if (leaveResult != 0)
    {
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession),
                            "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::WaitLeaveMesh),
             "NexMatchJointSessionJob::WaitLeaveMesh"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeaveMesh()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Mesh* mesh = session::g_piaMesh;
    Result leaveResult;
    switch (m_field5c)
    {
    case cLeaveMeshTypeLeave:
        if (!mesh->IsLeaveMeshCompleted())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        leaveResult = mesh->GetLeaveMeshResult();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        break;
    case cLeaveMeshTypeLeaveWithHostMigration:
        if (!mesh->IsLeaveMeshWithHostMigrationCompleted())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        leaveResult = mesh->GetLeaveMeshWithHostMigrationResult();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        break;
    case cLeaveMeshTypeDestroy:
        if (!mesh->IsDestroyMeshCompleted())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        leaveResult = mesh->GetDestroyMeshResult();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        break;
    default:
        leaveResult = cResultInvalidState;
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        break;
    }

    if (m_field5c == cLeaveMeshTypeLeaveWithHostMigration && leaveResult == 0)
    {
        setDeadline(m_time1a8, cHostMigratedTimeoutMsec);
        setStep(&NexMatchJointSessionJob::WaitHostMigrated, "NexMatchJointSessionJob::WaitHostMigrated");
    }
    else
    {
        setStep(&NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession,
                "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession");
    }
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitHostMigrated()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Session* piaSession = session::g_piaSession;
    if (m_time1a8 <= common::g_piaJobScheduler->m_dispatchTime)
    {
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession),
                            "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (m_jointSessionState >= cJointSessionStateLeavePreviousMesh
        && m_jointSessionState <= cJointSessionStateJoinNextMesh)
    {
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                &NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession),
                            "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession"));
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    }

    if (static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
            ->GetMatchmakeSessionTotalStations(
                session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
        > cHostMigrationStationNumThreshold)
    {
        if (!piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]->IsMatchmakeOwner())
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession),
                                "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
        }
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                            &NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession),
                        "NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Session* piaSession = session::g_piaSession;
    unsigned int sessionId = piaSession->m_sessionId[piaSession->m_currentSessionIndex];
    if (sessionId != 0)
    {
        session::IMatchmakeSession* matchmakeSession =
            piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex];
        m_callContext.Reset();
        Result leaveResult;
        if (!session::g_piaSession->IsHostMigrationEnabled() && matchmakeSession->IsMatchmakeOwner())
        {
            m_field1eb = 1;
            leaveResult = matchmakeSession->UnregisterMatchmakeSession(&m_callContext, sessionId);
        }
        else
        {
            m_field1eb = 0;
            leaveResult = matchmakeSession->LeaveMatchmakeSession(&m_callContext, sessionId);
        }
        if (leaveResult == 0)
        {
            setStep(&NexMatchJointSessionJob::WaitLeaveCurrentMatchmakeSession,
                    "NexMatchJointSessionJob::WaitLeaveCurrentMatchmakeSession");
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        if (leaveResult == cResultRequestFailed)
            m_failureResult = leaveResult;
    }

    setStep(&NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession,
            "NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeaveCurrentMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Session* piaSession = session::g_piaSession;
    session::IMatchmakeSession* matchmakeSession = piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex];
    if (m_field1eb == 0)
    {
        if (!matchmakeSession->WaitLeaveMatchmakeSession())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }
    else
    {
        if (!matchmakeSession->WaitUnregisterMatchmakeSession())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callContext.m_state == common::CallContext::cStateFailure && m_callContext.m_result == cResultRequestFailed)
    {
        m_failureResult = m_callContext.m_result;
    }
    piaSession->m_sessionId[piaSession->m_currentSessionIndex] = 0;
    setStep(&NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession,
            "NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::Session* piaSession = session::g_piaSession;
    unsigned int bufferSessionId = piaSession->m_sessionId[bufferSessionIndex()];
    if (bufferSessionId != 0)
    {
        session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
        m_callContext.Reset();
        Result leaveResult;
        if (!session::g_piaSession->IsHostMigrationEnabled() && matchmakeSession->IsMatchmakeOwner())
        {
            m_field1eb = 1;
            leaveResult = matchmakeSession->UnregisterMatchmakeSession(&m_callContext, bufferSessionId);
        }
        else
        {
            m_field1eb = 0;
            leaveResult = matchmakeSession->LeaveMatchmakeSession(&m_callContext, bufferSessionId);
        }
        if (leaveResult == 0)
        {
            SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                    &NexMatchJointSessionJob::WaitLeaveBufferMatchmakeSession),
                                "NexMatchJointSessionJob::WaitLeaveBufferMatchmakeSession"));
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
        }
        if (leaveResult == cResultRequestFailed)
            m_failureResult = leaveResult;
    }

    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            unsigned int sessionId = meshLayerController->GetSessionId(i);
            if (sessionId == 0)
                continue;
            if (!meshLayerController->IsParticipated(sessionId, NexFacade::GetInstance()->GetLocalPrincipalId()))
            {
                meshLayerController->ClearSessionId(sessionId);
                continue;
            }
            if (sessionId != piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            {
                session::g_piaSession->m_sessionId[bufferSessionIndex()] = sessionId;
                session::IMatchmakeSession* matchmakeSession = piaSession->m_matchmakeSession[bufferSessionIndex()];
                m_callContext.Reset();
                Result leaveResult;
                if (!session::g_piaSession->IsHostMigrationEnabled() && matchmakeSession->IsMatchmakeOwner())
                {
                    m_field1eb = 1;
                    leaveResult = matchmakeSession->UnregisterMatchmakeSession(&m_callContext, sessionId);
                }
                else
                {
                    m_field1eb = 0;
                    leaveResult = matchmakeSession->LeaveMatchmakeSession(&m_callContext, sessionId);
                }
                if (leaveResult == 0)
                {
                    SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                            &NexMatchJointSessionJob::WaitLeaveBufferMatchmakeSession),
                                        "NexMatchJointSessionJob::WaitLeaveBufferMatchmakeSession"));
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
            }
        }
    }

    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&NexMatchJointSessionJob::ProcessFailure),
             "NexMatchJointSessionJob::ProcessFailure"));
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeaveBufferMatchmakeSession()
{
    NEX_MATCH_JOINT_SESSION_CHECK_STATUS()

    session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (m_field1eb == 0)
    {
        if (!matchmakeSession->WaitLeaveMatchmakeSession())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }
    else
    {
        if (!matchmakeSession->WaitUnregisterMatchmakeSession())
            return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (m_callContext.m_state == common::CallContext::cStateFailure && m_callContext.m_result == cResultRequestFailed)
    {
        m_failureResult = m_callContext.m_result;
    }
    session::g_piaSession->m_sessionId[bufferSessionIndex()] = 0;
    setStep(&NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession,
            "NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession");
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ProcessFailure()
{
    if (m_fieldc8 == 0)
        m_fieldc8 = cFailurePointUnknown;
    session::g_piaSession->m_fieldc0 = 0;
    session::g_piaSession->m_postponeNotice = false;
    ClearProcessTypeAndNoticeFailureEvent();
    if (m_callerContext != 0)
    {
        if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
            m_failureResult = session::Session::cResultStatusDisconnectedNetwork;
        else if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown6)
            m_failureResult = session::Session::cResultStatusUnknown6;
        else if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown7)
            m_failureResult = session::Session::cResultStatusUnknown7;
        else if (session::g_piaSession->GetStatus() == session::Session::cStatusUnknown8)
            m_failureResult = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(m_failureResult);
        m_callerContext = 0;
    }
    m_failureResult = 0;
    m_jointSessionState = cJointSessionStateFinished;
    session::g_piaMesh->GetMonitoringDataSender()->SetSendSessionBeginReportRequested(true);
    session::g_piaMesh->SetSkipReport(false);
    session::g_piaMesh->SendReport(cReportReasonJointSession);
    return MakeExecuteResult(cExecuteKindFinished);
}

unsigned int NexMatchJointSessionJob::GetJointSessionStateForDebug() const
{
    return m_jointSessionState;
}

void NexMatchJointSessionJob::Trace(unsigned long long p0) const
{
}

void NexMatchJointSessionJob::CleanupMonitoringData()
{
    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_joinResult = cMonitoringUnset32;
    monitoring.m_metric47 = cMonitoringUnset8;
    monitoring.m_metric50 = cMonitoringUnset8;
    for (int i = 0; i < 15; ++i)
        monitoring.m_metric51[i] = cMonitoringUnset32;
    monitoring.m_metric52 = cMonitoringUnset8;
    for (int i = 0; i < 8; ++i)
        monitoring.m_metric53[i] = cMonitoringUnset32;
    monitoring.m_metric75 = cMonitoringUnset32;
    monitoring.m_metric76 = cMonitoringUnset32;
    monitoring.m_metric158 = cMonitoringUnset8;
    monitoring.m_metric78 = cMonitoringUnset8;
    monitoring.m_metric79 = cMonitoringUnset8;
    monitoring.m_metric77 = cMonitoringUnset32;
    monitoring.m_metric80 = cMonitoringUnset32;
    monitoring.m_metric81 = cMonitoringUnset32;
    for (int i = 0; i < 6; ++i)
    {
        monitoring.m_metric82[i] = cMonitoringUnset32;
        monitoring.m_metric89[i] = cMonitoringUnset8;
        monitoring.m_metric90[i] = cMonitoringUnset8;
        monitoring.m_metric91[i] = cMonitoringUnset8;
        monitoring.m_metric100[i] = cMonitoringUnset8;
        monitoring.m_metric101[i] = cMonitoringUnset8;
        monitoring.m_metric102[i] = cMonitoringUnset8;
    }
    monitoring.m_metric84 = cMonitoringUnset32;
    monitoring.m_metric108 = cMonitoringUnset8;
    monitoring.m_metric85 = cMonitoringUnset8;
    monitoring.m_metric83 = cMonitoringUnset16;
    monitoring.m_metric86 = cMonitoringUnset8;
    monitoring.m_metric87 = cMonitoringUnset8;
    monitoring.m_metric88 = cMonitoringUnset8;
    monitoring.m_metric92 = cMonitoringUnset8;
    monitoring.m_metric93 = cMonitoringUnset8;
    monitoring.m_metric94 = cMonitoringUnset8;
    monitoring.m_metric96 = cMonitoringUnset8;
    monitoring.m_metric97 = cMonitoringUnset8;
    monitoring.m_metric98 = cMonitoringUnset8;
    monitoring.m_metric99 = cMonitoringUnset8;
    monitoring.m_metric112 = cMonitoringUnset8;
    monitoring.m_metric109 = cMonitoringUnset32;
    monitoring.m_metric110 = cMonitoringUnset32;
    monitoring.m_metric111 = cMonitoringUnset32;
    monitoring.m_metric114 = cMonitoringUnset32;
    monitoring.m_metric115 = cMonitoringUnset32;
    monitoring.m_metric113 = cMonitoringUnset8;
    monitoring.m_metric103 = cMonitoringUnset8;
    monitoring.m_metric104 = cMonitoringUnset8;
    monitoring.m_metric105 = cMonitoringUnset8;
    monitoring.m_metric117 = cMonitoringUnset8;
    monitoring.m_metric118 = cMonitoringUnset8;
    monitoring.m_joinRetryCount = cMonitoringUnset8;
    monitoring.m_metric107 = cMonitoringUnset8;
    monitoring.m_metric119 = cMonitoringUnset8;
    monitoring.m_retryReasonACount = cMonitoringUnset8;
    monitoring.m_connectionInfoRetryCount = cMonitoringUnset8;
    monitoring.m_sessionInfoRetryCount = cMonitoringUnset8;
    monitoring.m_ownerChangeRetryCount = cMonitoringUnset8;
    monitoring.m_retryReasonBCount = cMonitoringUnset8;
    monitoring.m_joinSessionState = cMonitoringUnset8;
    monitoring.m_metric130 = cMonitoringUnset8;
    monitoring.m_joinMeshState = cMonitoringUnset8;
    monitoring.m_metric137 = cMonitoringUnset8;
    monitoring.m_metric128 = cMonitoringUnset8;
    monitoring.m_metric95 = cMonitoringUnset32;
    monitoring.m_metric138 = cMonitoringUnset8;
    monitoring.m_metric127 = cMonitoringUnset8;
    monitoring.m_metric116 = cMonitoringUnset32;
    monitoring.m_joinTargetIdHash = cMonitoringUnset32;
    monitoring.m_metric126 = cMonitoringUnset32;
    for (int i = 0; i < 15; ++i)
    {
        monitoring.m_metric139[i] = cMonitoringUnset32;
        monitoring.m_metric140[i] = cMonitoringUnset32;
        monitoring.m_metric141[i] = cMonitoringUnset32;
        monitoring.m_metric142[i] = cMonitoringUnset32;
        monitoring.m_metric143[i] = cMonitoringUnset32;
        monitoring.m_metric144[i] = cMonitoringUnset8;
        monitoring.m_metric145[i] = cMonitoringUnset8;
        monitoring.m_metric146[i] = cMonitoringUnset8;
        monitoring.m_metric147[i] = cMonitoringUnset8;
        monitoring.m_metric155[i] = cMonitoringUnset8;
    }
    monitoring.m_metric150 = cMonitoringUnset16;
    monitoring.m_metric151 = cMonitoringUnset16;
    monitoring.m_metric148 = cMonitoringUnset8;
    monitoring.m_metric152 = cMonitoringUnset16;
    monitoring.m_metric149 = cMonitoringUnset8;
    common::g_sessionStateMonitoringContent.Cleanup();
}

Result NexMatchJointSessionJob::StartupJoinRandomJointSessionImpl(const session::JoinRandomSessionSetting* setting)
{
    m_jointSessionState = cJointSessionStateStartup;
    session::Session* piaSession = session::g_piaSession;
    session::IMatchmakeSession* matchmakeSession = piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (piaSession->m_sessionId[bufferSessionIndex()] != 0)
        return cResultInvalidState;

    m_field1ec = 0;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            if (meshLayerController->GetSessionId(i) != 0
                && piaSession->m_sessionId[piaSession->m_currentSessionIndex] != meshLayerController->GetSessionId(i))
                m_field1ec = 1;
        }
    }
    CleanupMonitoringData();

    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(
        setting->GetCreateSessionSetting());
    if (createSetting->GetMaxParticipantNum() == 0)
        return cResultInvalidArgument;
    const unsigned short maxParticipantNum = createSetting->GetMaxParticipantNum();
    const unsigned short extraParticipantNum = createSetting->GetExtraParticipantNum();
    if (transport::g_piaTransport->m_stationNum < static_cast<unsigned short>(maxParticipantNum + extraParticipantNum))
        return cResultInvalidArgument;

    m_field1ee = createSetting->IsOpenSession();
    if (static_cast<NexMatchmakeSession*>(matchmakeSession)
            ->ReflectJoinRandomSetting(setting, piaSession->IsHostMigrationEnabled())
        != 0)
        return cResultInvalidArgument;

    session::g_piaSession->m_fieldc2[bufferSessionIndex()] = static_cast<unsigned char>(maxParticipantNum
                                                                                        + extraParticipantNum);
    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();
    const common::ListNode* sentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    session::StationIdStatusTable* stationIdStatusTable = session::g_piaSession->m_stationIdStatusTable;
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        if (stationIdStatusTable->CheckConnectStatus(entry->stationId))
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = entry->stationId;
        }
    }
    m_field60 = 0;
    m_field98 = 0;
    m_field1e8 = 0;
    m_field1e9 = 0;
    m_field1d8 = 0;
    m_field1b8 = 0;
    m_field1c8 = 0;
    m_jointSessionState = cJointSessionStateCallSessionEvent;
    setStep(&NexMatchJointSessionJob::CallSessionEvent, "NexMatchJointSessionJob::CallSessionEvent");
    return 0;
}

Result NexMatchJointSessionJob::StartupCreateJointSessionImpl(const session::CreateSessionSetting* setting)
{
    m_jointSessionState = cJointSessionStateStartup;
    session::Session* piaSession = session::g_piaSession;
    session::IMatchmakeSession* matchmakeSession = piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (piaSession->m_sessionId[bufferSessionIndex()] != 0)
        return cResultInvalidState;

    m_field1ec = 0;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            if (meshLayerController->GetSessionId(i) != 0
                && piaSession->m_sessionId[piaSession->m_currentSessionIndex] != meshLayerController->GetSessionId(i))
                m_field1ec = 1;
        }
    }
    CleanupMonitoringData();

    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(setting);
    if (createSetting->GetMaxParticipantNum() == 0)
        return cResultInvalidArgument;
    const unsigned short maxParticipantNum = createSetting->GetMaxParticipantNum();
    const unsigned short extraParticipantNum = createSetting->GetExtraParticipantNum();
    if (transport::g_piaTransport->m_stationNum < static_cast<unsigned short>(maxParticipantNum + extraParticipantNum))
        return cResultInvalidArgument;

    m_field1ee = createSetting->IsOpenSession();
    static_cast<NexMatchmakeSession*>(matchmakeSession)
        ->ReflectCreateSetting(setting, piaSession->IsHostMigrationEnabled());

    session::g_piaSession->m_fieldc2[bufferSessionIndex()] = static_cast<unsigned char>(maxParticipantNum
                                                                                        + extraParticipantNum);
    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();
    const common::ListNode* sentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    session::StationIdStatusTable* stationIdStatusTable = session::g_piaSession->m_stationIdStatusTable;
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        if (stationIdStatusTable->CheckConnectStatus(entry->stationId))
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = entry->stationId;
        }
    }
    m_field1c8 = 0;
    m_field1b8 = 0;
    m_field1e8 = 0;
    m_field60 = 0;
    m_field98 = 0;
    m_jointSessionState = cJointSessionStateCallSessionEvent;
    m_field1e9 = 0;
    m_field1d8 = 0;
    setStep(&NexMatchJointSessionJob::CallSessionEvent, "NexMatchJointSessionJob::CallSessionEvent");
    return 0;
}

Result NexMatchJointSessionJob::StartupJoinJointSessionImpl(const session::JoinSessionSetting* setting)
{
    m_jointSessionState = cJointSessionStateStartup;
    session::Session* piaSession = session::g_piaSession;
    session::IMatchmakeSession* matchmakeSession = piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (piaSession->m_sessionId[bufferSessionIndex()] != 0)
        return cResultInvalidState;

    m_field1ec = 0;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            if (meshLayerController->GetSessionId(i) != 0
                && piaSession->m_sessionId[piaSession->m_currentSessionIndex] != meshLayerController->GetSessionId(i))
                m_field1ec = 1;
        }
    }
    CleanupMonitoringData();
    static_cast<NexMatchmakeSession*>(matchmakeSession)->ReflectJoinSetting(setting);

    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();
    const common::ListNode* sentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    session::StationIdStatusTable* stationIdStatusTable = session::g_piaSession->m_stationIdStatusTable;
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        if (stationIdStatusTable->CheckConnectStatus(entry->stationId))
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = entry->stationId;
        }
    }
    m_field60 = 0;
    m_field1d8 = 0;
    m_field1e8 = 0;
    m_field1c8 = 0;
    m_field1e9 = 0;
    m_field1b8 = 0;
    m_jointSessionState = cJointSessionStateCallSessionEvent;
    setStep(&NexMatchJointSessionJob::CallSessionEvent, "NexMatchJointSessionJob::CallSessionEvent");
    return 0;
}

Result NexMatchJointSessionJob::StartupLeaveJointSessionImpl()
{
    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == 0)
        return cResultInvalidState;

    m_field1ec = 0;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        bool otherSessionFound = false;
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            if (meshLayerController->GetSessionId(i) != 0
                && piaSession->m_sessionId[bufferSessionIndex()] != meshLayerController->GetSessionId(i))
            {
                otherSessionFound = true;
                break;
            }
        }
        if (!otherSessionFound)
            m_field1ec = 1;
    }

    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();
    const common::ListNode* sentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    session::StationIdStatusTable* stationIdStatusTable = piaSession->m_stationIdStatusTable;
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        if (stationIdStatusTable->CheckConnectStatus(entry->stationId))
        {
            unsigned int sessionId;
            if (stationIdStatusTable->GetSessionId(entry->stationId, &sessionId)
                && sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
            {
                unsigned long long* companionStationId = m_companionStationIdList.Alloc();
                if (companionStationId != 0)
                    *companionStationId = entry->stationId;
            }
        }
    }
    CleanupMonitoringData();
    m_field60 = 0;
    m_jointSessionState = cJointSessionStateCallSessionEvent;
    m_field98 = piaSession->m_sessionId[piaSession->m_currentSessionIndex];
    m_field1d8 = 0;
    m_field1e9 = 0;
    m_field1b8 = 0;
    m_field1c8 = 0;
    m_field1e8 = 0;
    setStep(&NexMatchJointSessionJob::CallSessionEvent, "NexMatchJointSessionJob::CallSessionEvent");
    return 0;
}

Result NexMatchJointSessionJob::StartupDestroyJointSessionImpl()
{
    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == 0)
        return cResultInvalidState;

    m_field1ec = 0;
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        bool otherSessionFound = false;
        for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
        {
            if (meshLayerController->GetSessionId(i) != 0
                && piaSession->m_sessionId[bufferSessionIndex()] != meshLayerController->GetSessionId(i))
            {
                otherSessionFound = true;
                break;
            }
        }
        if (!otherSessionFound)
            m_field1ec = 1;
    }

    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();
    const common::ListNode* sentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
    session::StationIdStatusTable* stationIdStatusTable = piaSession->m_stationIdStatusTable;
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
        if (stationIdStatusTable->CheckConnectStatus(entry->stationId))
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = entry->stationId;
        }
    }
    CleanupMonitoringData();
    m_jointSessionState = cJointSessionStateCallDestroySessionEvent;
    m_field1b8 = 0;
    m_field98 = piaSession->m_sessionId[piaSession->m_currentSessionIndex];
    m_field60 = 0;
    m_field1d8 = 0;
    m_field1e9 = 0;
    m_field1e8 = 0;
    m_field1c8 = 0;
    setStep(&NexMatchJointSessionJob::CallDestroySessionEvent, "NexMatchJointSessionJob::CallDestroySessionEvent");
    return 0;
}

Result NexMatchJointSessionJob::StartupPassiveProcessImpl(session::Session::ProcessType processType,
                                                          session::SessionProtocol::MessageType messageType,
                                                          const unsigned long long* stationIds,
                                                          unsigned short stationIdNum,
                                                          unsigned long long senderStationId)
{
    session::Session* piaSession = session::g_piaSession;
    bool senderIncluded = false;
    bool localIncluded = false;
    for (unsigned short i = 0; i < stationIdNum; ++i)
    {
        if (stationIds[i] == senderStationId)
            senderIncluded = true;
        if (stationIds[i] == session::g_piaSession->m_localStationId)
            localIncluded = true;
    }
    if (!senderIncluded)
        return cResultInvalidState;
    if (!localIncluded)
        return cResultInvalidState;

    if (m_companionStationIdList.GetNodeNum() != 0)
        m_companionStationIdList.Clear();
    m_field98 = 0;

    if (processType == session::Session::cProcessTypeDestroyJointSession)
    {
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == 0)
            return cResultInvalidState;

        m_field1ec = 0;
        NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
            piaSession->m_meshLayerController);
        if (meshLayerController != 0)
        {
            bool otherSessionFound = false;
            for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
            {
                if (meshLayerController->GetSessionId(i) != 0
                    && piaSession->m_sessionId[bufferSessionIndex()] != meshLayerController->GetSessionId(i))
                {
                    otherSessionFound = true;
                    break;
                }
            }
            if (!otherSessionFound)
                m_field1ec = 1;
        }

        if (messageType == session::SessionProtocol::cMessageTypeDestroyInvitation)
        {
            m_jointSessionState = cJointSessionStateDestroyInvitationReceived;
            const common::ListNode* sentinel = transport::g_piaTransport->m_stationIdTable->m_entries.Sentinel();
            session::StationIdStatusTable* stationIdStatusTable = piaSession->m_stationIdStatusTable;
            for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
            {
                const transport::StationIdTable::Entry* entry = transport::StationIdTable::EntryList::ValueOf(node);
                if (stationIdStatusTable->CheckConnectStatus(entry->stationId))
                {
                    unsigned int sessionId;
                    if (stationIdStatusTable->GetSessionId(entry->stationId, &sessionId)
                        && sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
                    {
                        unsigned long long* companionStationId = m_companionStationIdList.Alloc();
                        if (companionStationId != 0)
                            *companionStationId = entry->stationId;
                    }
                }
            }
        }
        else
        {
            const Result result = ReceiveLeaveInvitationImpl(processType, stationIds, stationIdNum, senderStationId);
            if (result != 0)
                return result;
        }
    }
    else if (processType == session::Session::cProcessTypeLeaveJointSession)
    {
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == 0)
            return cResultInvalidState;

        m_field1ec = 0;
        NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
            piaSession->m_meshLayerController);
        if (meshLayerController != 0)
        {
            bool otherSessionFound = false;
            for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
            {
                if (meshLayerController->GetSessionId(i) != 0
                    && piaSession->m_sessionId[bufferSessionIndex()] != meshLayerController->GetSessionId(i))
                {
                    otherSessionFound = true;
                    break;
                }
            }
            if (!otherSessionFound)
                m_field1ec = 1;
        }
        m_jointSessionState = cJointSessionStatePassiveProcess;
        m_field60 = cProcessStepAnswerToInvitation;
        for (unsigned short i = 0; i < stationIdNum; ++i)
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = stationIds[i];
        }
    }
    else
    {
        if (piaSession->m_sessionId[bufferSessionIndex()] != 0)
            return cResultInvalidState;

        m_field1ec = 0;
        NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
            piaSession->m_meshLayerController);
        if (meshLayerController != 0)
        {
            for (unsigned int i = 0; i < meshLayerController->GetSessionIdArraySize(); ++i)
            {
                if (meshLayerController->GetSessionId(i) != 0
                    && piaSession->m_sessionId[piaSession->m_currentSessionIndex]
                           != meshLayerController->GetSessionId(i))
                    m_field1ec = 1;
            }
        }
        m_jointSessionState = cJointSessionStatePassiveProcess;
        m_field60 = cProcessStepAnswerToInvitation;
        for (unsigned short i = 0; i < stationIdNum; ++i)
        {
            unsigned long long* companionStationId = m_companionStationIdList.Alloc();
            if (companionStationId != 0)
                *companionStationId = stationIds[i];
        }
    }
    CleanupMonitoringData();
    m_field1c8 = 0;
    m_field1e9 = 0;
    m_field1b8 = 0;
    m_field1d8 = 0;
    m_field1e8 = 0;
    setStep(&NexMatchJointSessionJob::CallSessionEvent, "NexMatchJointSessionJob::CallSessionEvent");
    return 0;
}

Result NexMatchJointSessionJob::ReceiveNextSessionIdImpl(session::Session::ProcessType processType,
                                                         unsigned int sessionId, const unsigned char* applicationData,
                                                         unsigned char applicationDataSize,
                                                         unsigned long long senderStationId)
{
    session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[bufferSessionIndex()];
    if (matchmakeSession != 0)
    {
        static_cast<NexMatchmakeSession*>(matchmakeSession)
            ->SetMatchmakeSessionKey(applicationData, applicationDataSize);
    }
    return 0;
}

Result NexMatchJointSessionJob::ReceiveLeaveInvitationImpl(session::Session::ProcessType processType,
                                                           const unsigned long long* stationIds,
                                                           unsigned short stationIdNum,
                                                           unsigned long long senderStationId)
{
    m_jointSessionState = cJointSessionStatePassiveProcess;
    session::Session* piaSession = session::g_piaSession;
    if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == 0)
        return cResultInvalidState;

    m_companionStationIdList.Clear();
    const unsigned long long localStationId = piaSession->m_localStationId;
    bool senderIncluded = false;
    bool localIncluded = false;
    for (unsigned short i = 0; i < stationIdNum; ++i)
    {
        unsigned long long* companionStationId = m_companionStationIdList.Alloc();
        if (companionStationId != 0)
        {
            *companionStationId = stationIds[i];
            if (*companionStationId == senderStationId)
                senderIncluded = true;
            if (*companionStationId == localStationId)
                localIncluded = true;
        }
    }
    if (!senderIncluded)
        return cResultInvalidState;
    if (!localIncluded)
        return cResultInvalidState;
    return 0;
}

Result NexMatchJointSessionJob::ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType,
                                                               unsigned long long senderStationId)
{
    m_jointSessionState = cJointSessionStateLeaveMeshInvitationReceived;
    m_field9f = 1;
    return 0;
}

Result NexMatchJointSessionJob::ReceiveCompletionResponseImpl(session::Session::ProcessType processType,
                                                              unsigned long long senderStationId)
{
    m_fielda0 = 1;
    return 0;
}

void NexMatchJointSessionJob::NoticeHostChangeEventImpl(unsigned long long hostStationId)
{
    m_field1c0 = hostStationId;
    m_field1b8 = 1;
    if (!session::g_piaSession->IsHostMigrationEnabled())
    {
        m_pendingFailureResult = cResultOwnerChanged;
        m_failurePending = 1;
    }
}

void NexMatchJointSessionJob::NoticeOwnerChangeEventImpl(unsigned int sessionId, unsigned long long ownerStationId)
{
    if (sessionId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
    {
        m_field1d0 = ownerStationId;
        m_field1c8 = 1;
    }
    if (sessionId == session::g_piaSession->m_sessionId[bufferSessionIndex()])
    {
        m_field1e0 = ownerStationId;
        m_field1d8 = 1;
    }
    if (!session::g_piaSession->IsHostMigrationEnabled())
    {
        m_pendingFailureResult = cResultOwnerChanged;
        m_failurePending = 1;
    }
}

void NexMatchJointSessionJob::NoticeLeaveEventImpl(unsigned int sessionId, unsigned long long stationId)
{
    if (sessionId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
    {
        StationIdList& companionStationIdList = m_companionStationIdList;
        common::ListNode* sentinel = companionStationIdList.Sentinel();
        for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
        {
            if (*StationIdList::ValueOf(node) == stationId)
            {
                companionStationIdList.Free(static_cast<StationIdList::Node*>(node));
                CheckJoinedOnlyCompanion();
                return;
            }
        }
    }
    CheckJoinedOnlyCompanion();
}

void NexMatchJointSessionJob::NoticeUnregisterEventImpl(unsigned int sessionId)
{
    if ((IsMigrateSessionToJointSession() && sessionId == session::g_piaSession->m_sessionId[bufferSessionIndex()])
        || sessionId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
    {
        m_pendingFailureResult = cResultOwnerChanged;
        m_failurePending = 1;
    }
}

void NexMatchJointSessionJob::CleanupImpl()
{
    m_jointSessionState = 0;
    m_field1ec = 0;
    m_field1ea = 0;
    m_field1e8 = 0;
    m_field1eb = 0;
    m_failureResult = 0;
    m_field1b8 = 0;
    m_field1c0 = 0;
    m_field1c8 = 0;
    m_field1d0 = 0;
    m_field1d8 = 0;
    m_field1e0 = 0;
    m_field1e9 = 0;
    m_stationConnectionInfo.SetStationConnectionInfo(transport::StationConnectionInfo());
    m_field1ee = 1;
    m_sessionIdNum = 0;
    for (unsigned char i = 0; i < cStationNumMax; ++i)
    {
        m_sessionIds[i] = 0;
        m_sessionInfoRequested[i] = false;
    }
}

}
}
}
