#include "nn/pia/inet/InetJoinMeshJob.hpp"

#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult (InetJoinMeshJob::*Step)();

const unsigned int cMonitoringValueUnset = 0xffffffffu;

enum
{
    cMonitoringStationNum = 15,
    cMonitoringByteUnset = 0xff,
    cBandwidthCheckJoinState = 3,
    cConnectedStationState = 5
};

enum
{
    cTraceSession = 0x08000000
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

InetJoinMeshJob::InetJoinMeshJob()
{
}

InetJoinMeshJob::~InetJoinMeshJob()
{
}

unsigned int InetJoinMeshJob::StartupImpl()
{
    Reset(true);
    SelectStep(m_currentStep, &JoinMeshJob::SetupLocalPlayerInfo, "InetJoinMeshJob::SetupLocalPlayerInfo");
    common::g_sessionBeginMonitoringContent.m_metric71 = cMonitoringValueUnset;
    for (int index = 0; index < cMonitoringStationNum; ++index)
    {
        common::g_sessionBeginMonitoringContent.m_metric139[index] = cMonitoringValueUnset;
        common::g_sessionBeginMonitoringContent.m_metric140[index] = cMonitoringValueUnset;
        common::g_sessionBeginMonitoringContent.m_metric141[index] = cMonitoringValueUnset;
        common::g_sessionBeginMonitoringContent.m_metric142[index] = cMonitoringValueUnset;
        common::g_sessionBeginMonitoringContent.m_metric143[index] = cMonitoringValueUnset;
        common::g_sessionBeginMonitoringContent.m_metric144[index] = cMonitoringByteUnset;
        common::g_sessionBeginMonitoringContent.m_metric145[index] = cMonitoringByteUnset;
        common::g_sessionBeginMonitoringContent.m_metric146[index] = cMonitoringByteUnset;
        common::g_sessionBeginMonitoringContent.m_metric147[index] = cMonitoringByteUnset;
        common::g_sessionBeginMonitoringContent.m_metric155[index] = cMonitoringByteUnset;
    }
    if (NexFacade::GetInstance())
        common::g_sessionBeginMonitoringContent.m_metric28 = NexFacade::GetInstance()->m_gatheringId;
    return 1;
}

InetJoinMeshJob::ExecuteResult InetJoinMeshJob::StartMonitoringServerAddressResolution()
{
    session::g_piaMesh->SetupSendReport();
    SelectStep(m_currentStep, &JoinMeshJob::SetupLocalStation, "InetJoinMeshJob::SetupLocalStation");
    return MakeExecuteResult(cExecuteKindContinue);
}

InetJoinMeshJob::ExecuteResult InetJoinMeshJob::StartBandwidthCheck()
{
    transport::BandwidthCheckProtocol* protocol = session::g_piaMesh->GetBandwidthCheckProtocol();
    if (protocol->m_isServer)
    {
        SelectStep(m_currentStep, &JoinMeshJob::CompleteProcess, "InetJoinMeshJob::CompleteProcess");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_joinState = cBandwidthCheckJoinState;
    if (m_stationCount < 2)
    {
        SelectStep(m_currentStep, &InetJoinMeshJob::WaitBandwidthCheck, "InetJoinMeshJob::WaitBandwidthCheck");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (!protocol->IsReady())
    {
        protocol->CancelSend();
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    transport::g_stationConnectionInfoTable->Trace(cTraceSession);
    transport::Station* station = 0;
    for (unsigned short index = 2; index <= m_stationCount; ++index)
    {
        station = transport::g_stationConnectionInfoTable->GetStation(m_stationInfos[m_stationCount - index]);
        if (station && station->GetStationIndex() != cStationIndexInvalid
            && station->GetStationIndex() != static_cast<unsigned int>(session::g_piaMesh->GetLocalStationIndex()))
            break;
    }
    if (station)
    {
        protocol->StartSend(station->GetStationIndex());
        SelectStep(m_currentStep, &InetJoinMeshJob::WaitBandwidthCheck, "InetJoinMeshJob::WaitBandwidthCheck");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &InetJoinMeshJob::WaitBandwidthCheck, "InetJoinMeshJob::WaitBandwidthCheck");
    return MakeExecuteResult(cExecuteKindContinue);
}

InetJoinMeshJob::ExecuteResult InetJoinMeshJob::WaitBandwidthCheck()
{
    transport::BandwidthCheckProtocol* protocol = session::g_piaMesh->GetBandwidthCheckProtocol();
    if (m_callerContext && m_callerContext->m_cancelRequested)
    {
        protocol->CancelSend();
        if (session::g_piaMesh->IsHostMigrationEnabled() && session::g_piaMesh->IsHost())
        {
            SelectStep(m_currentStep, &JoinMeshJob::LeaveMeshWithHostMigration,
                       "InetJoinMeshJob::LeaveMeshWithHostMigration");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        SelectStep(m_currentStep, &JoinMeshJob::LeaveMesh, "InetJoinMeshJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (CheckTransportConnectionStatus())
    {
        protocol->CancelSend();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (!session::g_piaMesh->IsHostMigrationEnabled() && CheckConnectionStateWithHostStation())
    {
        protocol->CancelSend();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (!protocol->IsCompleted())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (!session::g_piaMesh->IsHostMigrationEnabled())
    {
        SelectStep(m_currentStep, &InetJoinMeshJob::WaitCheckKickoutJob, "InetJoinMeshJob::WaitCheckKickoutJob");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, &InetJoinMeshJob::WaitCheckHostConnection, "InetJoinMeshJob::WaitCheckHostConnection");
    return MakeExecuteResult(cExecuteKindContinue);
}

InetJoinMeshJob::ExecuteResult InetJoinMeshJob::WaitCheckHostConnection()
{
    if (m_callerContext && m_callerContext->m_cancelRequested)
    {
        if (session::g_piaMesh->IsHostMigrationEnabled() && session::g_piaMesh->IsHost())
        {
            SelectStep(m_currentStep, &JoinMeshJob::LeaveMeshWithHostMigration,
                       "InetJoinMeshJob::LeaveMeshWithHostMigration");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        SelectStep(m_currentStep, &JoinMeshJob::LeaveMesh, "InetJoinMeshJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (CheckTransportConnectionStatus())
        return MakeExecuteResult(cExecuteKindFinished);
    if (session::g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        return MakeExecuteResult(cExecuteKindNextStep);
    if (session::g_piaMesh->GetKickoutManageJobPtr()->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    transport::Station* host = transport::StationManager::s_pInstance->GetStation(
        session::g_piaMesh->GetHostStationIndex());
    if (host && host->m_field20 == cConnectedStationState)
    {
        SelectStep(m_currentStep, &JoinMeshJob::CompleteProcess, "InetJoinMeshJob::CompleteProcess");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

InetJoinMeshJob::ExecuteResult InetJoinMeshJob::WaitCheckKickoutJob()
{
    if (m_callerContext && m_callerContext->m_cancelRequested)
    {
        if (session::g_piaMesh->IsHostMigrationEnabled() && session::g_piaMesh->IsHost())
        {
            SelectStep(m_currentStep, &JoinMeshJob::LeaveMeshWithHostMigration,
                       "InetJoinMeshJob::LeaveMeshWithHostMigration");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        SelectStep(m_currentStep, &JoinMeshJob::LeaveMesh, "InetJoinMeshJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (CheckTransportConnectionStatus())
        return MakeExecuteResult(cExecuteKindFinished);
    if (session::g_piaMesh->GetKickoutManageJobPtr()->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    SelectStep(m_currentStep, &JoinMeshJob::CompleteProcess, "InetJoinMeshJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

void InetJoinMeshJob::SetHostInfoToMonitoringData(const transport::StationConnectionInfo& info)
{
    common::g_sessionBeginMonitoringContent.m_metric71 = common::hashWithMd5(info.m_stationLocation.m_principalId);
}

void InetJoinMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl()
{
    SelectStep(m_currentStep, &InetJoinMeshJob::StartMonitoringServerAddressResolution,
               "InetJoinMeshJob::StartMonitoringServerAddressResolution");
}

InetJoinMeshJob::ExecuteResult InetJoinMeshJob::SetNextStepAfterWaitAllConnection()
{
    if (session::g_piaMesh->GetBitRateCheckMode())
    {
        SelectStep(m_currentStep, &InetJoinMeshJob::StartBandwidthCheck, "InetJoinMeshJob::StartBandwidthCheck");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, &JoinMeshJob::CompleteProcess, "InetJoinMeshJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

void InetJoinMeshJob::CleanupImpl()
{
}

}
}
}
