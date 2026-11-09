#include "nn/pia/session/SessionStatusCheckJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionInfo.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum
{
    cResultConnectionError = 0x6c50,
    cDisconnectReasonLeave = 2,
    cDisconnectReasonNetwork = 8,
    cMatchmakeStatusJointSessionProcessing = 3,
    cMatchmakeStatusJointSessionProcessed = 4
};

enum MeshDisconnectReason
{
    cMeshDisconnectReasonNone = 0,
    cMeshDisconnectReasonNormal = 1,
    cMeshDisconnectReasonLeave = 2,
    cMeshDisconnectReasonKickedOut = 3,
    cMeshDisconnectReason4 = 4,
    cMeshDisconnectReason5 = 5,
    cMeshDisconnectReasonUpdateFailed = 6,
    cMeshDisconnectReason7 = 7,
    cMeshDisconnectReason8 = 8,
    cMeshDisconnectReason9 = 9
};
}

SessionStatusCheckJob::SessionStatusCheckJob()
    : common::StepSequenceJob(),
      m_callContext(),
      m_field54(false),
      m_field55(false)
{
}

SessionStatusCheckJob::~SessionStatusCheckJob()
{
}

void SessionStatusCheckJob::Trace(unsigned long long value) const
{
}

Result SessionStatusCheckJob::Startup()
{
    const Result result = g_piaSession->m_meshLayerController->CheckConnectedStatus();
    if (result != 0)
        return result;

    Reset(true);
    if (g_piaSession->m_jointSessionJob != 0)
    {
        m_currentStep = Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(
                                 &SessionStatusCheckJob::CheckSessionStatus4JointSession),
                             "SessionStatusCheckJob::CheckSessionStatus4JointSession");
    }
    else
    {
        m_currentStep = Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&SessionStatusCheckJob::CheckSessionStatus),
            "SessionStatusCheckJob::CheckSessionStatus");
    }
    return 0;
}

SessionStatusCheckJob::ExecuteResult SessionStatusCheckJob::CheckSessionStatus()
{
    const Result connectedStatus = g_piaSession->m_meshLayerController->CheckConnectedStatus();
    if (connectedStatus == Session::cResultStatusDisconnectedNetwork)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusDisconnectedNetwork;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == Session::cResultStatusUnknown6)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusUnknown4;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == Session::cResultStatusUnknown7)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusUnknown5;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == Session::cResultStatusUnknown8)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusUnknown6;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (connectedStatus == Session::cResultStatusDisconnectedSession)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
        if (g_piaMesh->CheckConnectionError() == cResultConnectionError)
        {
            const unsigned int disconnectReason = g_piaMesh->GetDisconnectReason();
            switch (disconnectReason)
            {
            case cMeshDisconnectReasonNone:
            case cMeshDisconnectReasonNormal:
            case cMeshDisconnectReasonKickedOut:
            case cMeshDisconnectReason4:
            case cMeshDisconnectReason5:
            case cMeshDisconnectReasonUpdateFailed:
            case cMeshDisconnectReason7:
            case cMeshDisconnectReason8:
            case cMeshDisconnectReason9:
                g_piaSession->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
                break;
            default:
                break;
            }
            if (g_piaSession->m_sessionDisconnectReason == 0)
                g_piaSession->m_sessionDisconnectReason = disconnectReason;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }

    if (transport::g_piaTransport->m_connectionError != 0)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusDisconnectedNetwork;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    if (g_piaSession->m_matchmakeStatus == Session::cMatchmakeStatusMatched
        && g_piaMesh->CheckConnectionError() == cResultConnectionError)
    {
        const unsigned int disconnectReason = g_piaMesh->GetDisconnectReason();
        switch (disconnectReason)
        {
        case cMeshDisconnectReasonNone:
        case cMeshDisconnectReasonNormal:
        case cMeshDisconnectReasonKickedOut:
        case cMeshDisconnectReason4:
        case cMeshDisconnectReason5:
        case cMeshDisconnectReasonUpdateFailed:
        case cMeshDisconnectReason7:
        case cMeshDisconnectReason8:
        case cMeshDisconnectReason9:
            g_piaSession->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
            break;
        default:
            break;
        }
        if (g_piaSession->m_sessionDisconnectReason == 0)
            g_piaSession->m_sessionDisconnectReason = disconnectReason;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    if (Session::s_debugSetting.m_matchmakeAnalysisEnabled != 0 && g_piaSession->IsHost())
    {
        IMatchmakeSession* matchmakeSession = g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex];
        if (!Session::SessionJoiningApprovalCallback(0))
            matchmakeSession->GetSessionInfo()->IsOpened();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

SessionStatusCheckJob::ExecuteResult SessionStatusCheckJob::CheckSessionStatus4JointSession()
{
    Session* session = g_piaSession;
    const unsigned int matchmakeStatus = session->m_matchmakeStatus;
    const Result connectedStatus = session->m_meshLayerController->CheckConnectedStatus();
    if (connectedStatus == Session::cResultStatusDisconnectedNetwork)
    {
        session->m_disconnectStatus = Session::cConnectionStatusDisconnectedNetwork;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindSuspend);
    }
    if (connectedStatus == Session::cResultStatusUnknown6)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusUnknown4;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindSuspend);
    }
    if (connectedStatus == Session::cResultStatusUnknown7)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusUnknown5;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindSuspend);
    }
    if (connectedStatus == Session::cResultStatusUnknown8)
    {
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusUnknown6;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindSuspend);
    }

    if (transport::g_piaTransport->m_connectionError != 0)
    {
        session->m_disconnectStatus = Session::cConnectionStatusDisconnectedNetwork;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
        return MakeExecuteResult(cExecuteKindSuspend);
    }

    if (session->m_disconnectStatus != Session::cConnectionStatusConnected)
        return MakeExecuteResult(cExecuteKindNextStep);

    ExecuteResult executeResult = MakeExecuteResult(cExecuteKindNextStep);
    const unsigned char sessionIndex = session->m_currentSessionIndex;
    const unsigned int currentSessionId = session->m_sessionId[sessionIndex];
    const unsigned int bufferSessionId = session->m_sessionId[sessionIndex == 0 ? 1 : 0];

    if (matchmakeStatus == Session::cMatchmakeStatusMatched || matchmakeStatus == cMatchmakeStatusJointSessionProcessed)
    {
        if (g_piaMesh->CheckConnectionError() == cResultConnectionError)
        {
            const unsigned int disconnectReason = g_piaMesh->GetDisconnectReason();
            switch (disconnectReason)
            {
            case cMeshDisconnectReasonNone:
            case cMeshDisconnectReasonNormal:
            case cMeshDisconnectReasonKickedOut:
            case cMeshDisconnectReason4:
            case cMeshDisconnectReason5:
            case cMeshDisconnectReasonUpdateFailed:
            case cMeshDisconnectReason7:
            case cMeshDisconnectReason8:
            case cMeshDisconnectReason9:
                session->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
                break;
            case cMeshDisconnectReasonLeave:
                break;
            default:
                session->m_disconnectStatus = Session::cConnectionStatusDisconnectedNetwork;
                executeResult = MakeExecuteResult(cExecuteKindSuspend);
                break;
            }
            if (g_piaSession->m_sessionDisconnectReason == 0)
                g_piaSession->m_sessionDisconnectReason = disconnectReason;
        }
        else if (matchmakeStatus == Session::cMatchmakeStatusMatched)
        {
            if (currentSessionId == 0 || session->m_meshLayerController->IsJoinedUnknownSession(currentSessionId, 0))
            {
                session->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
                if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
                    g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
            }
            if (Session::s_debugSetting.m_matchmakeAnalysisEnabled != 0 && g_piaSession->IsHost()
                && !Session::SessionJoiningApprovalCallback(g_piaSession->GetMatchmakeAnalysisToken()))
                session->m_matchmakeSession[session->m_currentSessionIndex]->GetSessionInfo()->IsOpened();
        }
        else if (matchmakeStatus == cMatchmakeStatusJointSessionProcessed)
        {
            if (currentSessionId == 0 || bufferSessionId == 0
                || session->m_meshLayerController->IsJoinedUnknownSession(currentSessionId, bufferSessionId))
            {
                session->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
                if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
                    g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
            }
            if (Session::s_debugSetting.m_matchmakeAnalysisEnabled != 0 && g_piaSession->IsJointSessionHost()
                && !Session::SessionJoiningApprovalCallback(g_piaSession->GetMatchmakeAnalysisToken()))
                session->m_matchmakeSession[session->m_currentSessionIndex == 0 ? 1 : 0]->GetSessionInfo()->IsOpened();
        }
    }
    else if (matchmakeStatus == cMatchmakeStatusJointSessionProcessing)
    {
        if ((session->m_jointSessionJob == 0 || !session->m_jointSessionJob->m_sessionDisconnectDetected)
            && currentSessionId == 0 && bufferSessionId == 0)
        {
            g_piaSession->m_disconnectStatus = Session::cConnectionStatusDisconnectedNetwork;
            if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
                g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
            executeResult = MakeExecuteResult(cExecuteKindSuspend);
        }
    }
    else if (connectedStatus == Session::cResultStatusDisconnectedSession)
    {
        session->m_disconnectStatus = Session::cConnectionStatusDisconnectedSession;
        if (g_piaSession->m_sessionDisconnectReason != cDisconnectReasonLeave)
            g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
    }
    return executeResult;
}

void SessionStatusCheckJob::Cleanup()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_field54 = false;
    CancelCleanup();
    Reset(true);
}

}
}
}
