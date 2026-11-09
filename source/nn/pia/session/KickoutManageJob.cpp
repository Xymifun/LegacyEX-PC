#include "nn/pia/session/KickoutManageJob.hpp"

#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/transport/StationManager.hpp"

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
    cResultLeaveMeshNotNeeded = 0x6c50,
    cResultJoinCancelledRelayFailure = 0x6846,
    cResultJoinCancelledRelayTimeout = 0x6847,
    cResultJoinCancelledRelayRefused = 0x6848,
    cResultJoinCancelledKickedOut = 0x6849,
    cResultJoinCancelledUnknown = 0x6838,
    cKickoutReasonRelayFailureLast = 23,
    cDisconnectReasonKickedOut = 3,
    cStationIndexMax = 31,
    cResultKickoutNoSlot = 0x4c0d,
    cResultInvalidStationIndex = 0x10c07,
    cCallStateMin = 2,
    cCallStateMax = 4
};

inline unsigned int joinCancelResult(unsigned int kickoutReason)
{
    if (kickoutReason < KickoutManageJob::cKickoutReasonRelayFailureBase
        || kickoutReason > cKickoutReasonRelayFailureLast)
        return cResultJoinCancelledUnknown;
    switch (kickoutReason)
    {
    case 4:
        return cResultJoinCancelledRelayFailure;
    case 5:
        return cResultJoinCancelledRelayTimeout;
    case 6:
        return cResultJoinCancelledRelayRefused;
    default:
        return cResultJoinCancelledKickedOut;
    }
}

}

KickoutManageJob::KickoutManageJob() : m_joinCancelResult(0)
{
    m_kickoutReason = cKickoutReasonNone;
    m_kickoutCallContext = 0;
    m_joinCancelRequested = false;
}

KickoutManageJob::~KickoutManageJob()
{
}

void KickoutManageJob::Trace(unsigned long long value) const
{
}

void KickoutManageJob::SetKickoutFlag()
{
}

void KickoutManageJob::KickoutFromNetwork(const common::StationAddress& stationAddress)
{
}

bool KickoutManageJob::Startup()
{
    for (int i = 0; i < cKickoutStationNumMax; ++i)
        m_kickoutStations[i].m_stationIndex = cStationIndexInvalid;
    m_kickoutReason = cKickoutReasonNone;
    m_joinCancelRequested = false;
    return true;
}

void KickoutManageJob::Cleanup()
{
    for (int i = 0; i < cKickoutStationNumMax; ++i)
        m_kickoutStations[i].m_stationIndex = cStationIndexInvalid;

    if (m_callContext.m_state != common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();

    common::CallContext* kickoutCallContext = m_kickoutCallContext;
    if (kickoutCallContext != 0)
    {
        if (kickoutCallContext->m_state == common::CallContext::cStateInProgress)
            kickoutCallContext->SignalSuccess(0);
        m_kickoutCallContext = 0;
    }
    m_joinCancelRequested = false;
}

bool KickoutManageJob::StartupClient(KickoutReason kickoutReason)
{
    SetKickoutFlag();
    if (m_callContext.m_state == common::CallContext::cStateInProgress || IsRunning())
        return false;

    Reset(true);
    m_kickoutReason = kickoutReason;

    if (g_piaMesh->m_joinMeshJob->IsRunning())
    {
        m_joinCancelResult = Result(joinCancelResult(kickoutReason));
        m_joinCancelRequested = true;
        if (!g_piaMesh->m_processUpdateMeshJob->IsRunning())
        {
            if (g_piaMesh->GetJoinMeshState() == cDisconnectReasonKickedOut)
                setStep(&KickoutManageJob::ClientStartLeaveMesh, "KickoutManageJob::ClientStartLeaveMesh");
            else
                setStep(&KickoutManageJob::ClientFinalize, "KickoutManageJob::ClientFinalize");
            return true;
        }
    }

    setStep(&KickoutManageJob::ClientStartLeaveMesh, "KickoutManageJob::ClientStartLeaveMesh");
    if (g_piaMesh->m_leaveMeshJob->IsRunning())
        m_kickoutReason = cKickoutReasonNone;
    return true;
}

KickoutManageJob::ExecuteResult KickoutManageJob::ClientStartLeaveMesh()
{
    Mesh* mesh = g_piaMesh;
    if (mesh->GetLocalStationIndex() > cStationIndexMax
        || (mesh->CheckConnectionError() == Result(cResultLeaveMeshNotNeeded) && !m_joinCancelRequested))
    {
        setStep(&KickoutManageJob::ClientFinalize, "KickoutManageJob::ClientFinalize");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (mesh->m_leaveMeshJob->Startup(&m_callContext) == 0)
    {
        if (mesh->m_leaveMeshJob->RegisterExtraCallback(&m_callContext) == 0)
            return MakeExecuteResult(cExecuteKindNextStep);
    }
    else
    {
        mesh->m_leaveMeshJob->Ready(false);
    }
    setStep(&KickoutManageJob::ClientWaitLeaveMesh, "KickoutManageJob::ClientWaitLeaveMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

KickoutManageJob::ExecuteResult KickoutManageJob::ClientWaitLeaveMesh()
{
    const unsigned int state = static_cast<unsigned int>(m_callContext.m_state);
    if (state - cCallStateMin > cCallStateMax - cCallStateMin)
        return MakeExecuteResult(cExecuteKindNextStep);

    setStep(&KickoutManageJob::ClientFinalize, "KickoutManageJob::ClientFinalize");
    return MakeExecuteResult(cExecuteKindContinue);
}

KickoutManageJob::ExecuteResult KickoutManageJob::ClientFinalize()
{
    g_piaMesh->m_disconnectReason = cDisconnectReasonKickedOut;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReasonKickedOut));
    if (m_joinCancelRequested && g_piaMesh->m_joinMeshJob->IsRunning())
    {
        g_piaMesh->m_joinMeshJob->Cleanup(m_joinCancelResult);
        g_piaMesh->m_joinMeshJob->Reset(true);
    }
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

Result KickoutManageJob::StartKickout(StationIndex stationIndex, KickoutReason kickoutReason)
{
    if (static_cast<int>(stationIndex) > cStationIndexMax)
        return Result(cResultInvalidStationIndex);

    unsigned int freeSlot = cKickoutStationNumMax;
    for (int i = 0; i < cKickoutStationNumMax; ++i)
    {
        if (m_kickoutStations[i].m_stationIndex == stationIndex)
            return Result(0);
        if (static_cast<int>(m_kickoutStations[i].m_stationIndex) > cStationIndexMax
            && freeSlot == cKickoutStationNumMax)
            freeSlot = i;
    }

    if (freeSlot >= cKickoutStationNumMax)
        return Result(cResultKickoutNoSlot);

    if (!g_piaMesh->GetMeshProtocol()->SendKickoutNotice(stationIndex, static_cast<unsigned char>(kickoutReason)))
        return Result(cResultKickoutNoSlot);

    m_kickoutStations[freeSlot].m_stationIndex = stationIndex;
    m_kickoutStations[freeSlot].m_kickoutReason = kickoutReason;
    return Result(0);
}

void KickoutManageJob::SetLeaveEventStationIndex(StationIndex stationIndex)
{
    for (int i = 0; i < cKickoutStationNumMax; ++i)
    {
        if (m_kickoutStations[i].m_stationIndex == stationIndex)
        {
            m_kickoutStations[i].m_stationIndex = cStationIndexInvalid;
            common::StationAddress stationAddress;
            Result result = transport::StationManager::s_pInstance->GetStationAddress(&stationAddress, stationIndex);
            if (result == 0)
                KickoutFromNetwork(stationAddress);
            return;
        }
    }
}

bool KickoutManageJob::AssociateKickoutWith(common::CallContext* callContext)
{
    if (m_kickoutCallContext != 0)
        return false;

    m_kickoutReason = cKickoutReasonNone;
    if (callContext != 0)
    {
        m_kickoutCallContext = callContext;
        callContext->InitiateCall();
    }
    return true;
}

}
}
}
