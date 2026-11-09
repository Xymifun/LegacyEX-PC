#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
typedef common::Job::ExecuteResult (common::StepSequenceJob::*StepFunction)();

enum
{
    cDefaultTimeoutMsec = 5000,
    cDisconnectReason2 = 2,
    cHostMigrationModeMulti = 2,
    cStationState5 = 5,
    cTraceMask1 = 0x10000000
};
}

LeaveWithHostMigrationJob::LeaveWithHostMigrationJob()
{
    m_timeoutMsec = cDefaultTimeoutMsec;
    m_callerContext = 0;
    m_started = false;
    m_newHost = static_cast<StationIndex>(ProcessHostMigrationJob::cStationIndexInvalid);
    OSBlockSet(m_stationResponses, 0, sizeof(m_stationResponses));
    m_migrationStarted = false;
}

LeaveWithHostMigrationJob::~LeaveWithHostMigrationJob()
{
}

bool LeaveWithHostMigrationJob::Startup(common::CallContext* context)
{
    if (m_started)
        return false;
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(m_timeoutMsec);
    m_deadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    if (context != 0)
    {
        m_callerContext = context;
        context->InitiateCall();
    }
    m_started = true;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReason2));
    Reset(true);
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CheckHostMigrationProcess),
                         "LeaveWithHostMigrationJob::CheckHostMigrationProcess");
    return true;
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::SendStartMigrationMessage()
{
    m_newHost = static_cast<StationIndex>(DecideNextHost());
    const StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    if (m_newHost > ProcessHostMigrationJob::cStationIndexMax || m_newHost == localIndex)
    {
        m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CleanupMesh),
                             "LeaveWithHostMigrationJob::CleanupMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (index != localIndex && g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
        {
            g_piaMesh->GetMeshProtocol()->SendMigrationStart(static_cast<StationIndex>(index), m_newHost);
            m_stationResponses[index] = true;
        }
        else
        {
            m_stationResponses[index] = false;
        }
    }
    m_migrationStarted = true;
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::WaitMigrationResponse),
                         "LeaveWithHostMigrationJob::WaitMigrationResponse");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::WaitMigrationResponse()
{
    int index;
    for (index = 0; index < cStationNumMax; ++index)
    {
        if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
        {
            m_stationResponses[index] = false;
        }
        else if (m_stationResponses[index])
        {
            break;
        }
    }
    if (index < cStationNumMax && !(m_deadline <= common::g_piaJobScheduler->m_dispatchTime))
        return MakeExecuteResult(cExecuteKindNextStep);
    m_migrationStarted = false;
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CleanupMesh),
                         "LeaveWithHostMigrationJob::CleanupMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::CleanupMesh()
{
    transport::Station** station = transport::StationManager::s_pInstance->m_stations.begin();
    while (station != transport::StationManager::s_pInstance->m_stations.end())
    {
        (*station)->m_disconnectStationJob->ProcessDisconnection(*station);
        ++station;
    }
    g_piaMesh->CleanupStationsJobs();
    g_piaMesh->m_disconnectReason = cDisconnectReason2;
    g_piaMesh->CleanupStatus();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    CleanupStatus();
    return MakeExecuteResult(cExecuteKindFinished);
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::SendStartMultiMigrationMessage()
{
    if (!g_piaMesh->GetMeshProtocol()->SendMultiMigrationStart(m_stationResponses))
    {
        m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CleanupMesh),
                             "LeaveWithHostMigrationJob::CleanupMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_migrationStarted = true;
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::WaitMigrationResponse),
                         "LeaveWithHostMigrationJob::WaitMigrationResponse");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::CheckHostMigrationProcess()
{
    ProcessHostMigrationJob* migrationJob = g_piaMesh->m_processHostMigrationJob;
    const bool isMultiMigration = g_piaMesh->m_hostMigrationMode == cHostMigrationModeMulti;
    ExecuteResult result;
    if (migrationJob->m_started)
    {
        if (isMultiMigration)
        {
            migrationJob->Cleanup();
            migrationJob->m_cancelRequested = true;
            m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::StartDisconnectStations),
                                 "LeaveWithHostMigrationJob::StartDisconnectStations");
            result = MakeExecuteResult(cExecuteKindContinue);
        }
        else
        {
            m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::WaitHostMigrationProcess),
                                 "LeaveWithHostMigrationJob::WaitHostMigrationProcess");
            result = MakeExecuteResult(cExecuteKindContinue);
        }
    }
    else if (isMultiMigration)
    {
        m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::SendStartMultiMigrationMessage),
                             "LeaveWithHostMigrationJob::SendStartMultiMigrationMessage");
        result = MakeExecuteResult(cExecuteKindContinue);
    }
    else
    {
        m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::SendStartMigrationMessage),
                             "LeaveWithHostMigrationJob::SendStartMigrationMessage");
        result = MakeExecuteResult(cExecuteKindContinue);
    }
    return result;
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::WaitHostMigrationProcess()
{
    if (g_piaMesh->m_processHostMigrationJob->m_started)
    {
        if (m_deadline <= common::g_piaJobScheduler->m_dispatchTime)
        {
            m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CleanupMesh),
                                 "LeaveWithHostMigrationJob::CleanupMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::SendStartMigrationMessage),
                         "LeaveWithHostMigrationJob::SendStartMigrationMessage");
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::StartDisconnectStations()
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
    {
        m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CleanupMesh),
                             "LeaveWithHostMigrationJob::CleanupMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (transport::Station** station = transport::StationManager::s_pInstance->m_stations.begin();
         station != transport::StationManager::s_pInstance->m_stations.end(); ++station)
    {
        if (*station != localStation)
        {
            if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>((*station)->m_field10));
            transport::DisconnectStationJob* disconnectJob = (*station)->m_disconnectStationJob;
            if (disconnectJob != 0)
            {
                if (disconnectJob->Startup(*station) != 0)
                    (*station)->Trace(cTraceMask1);
                else
                    disconnectJob->Ready(false);
            }
            else
            {
                (*station)->Trace(cTraceMask1);
            }
        }
    }
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::WaitDisconnectStations),
                         "LeaveWithHostMigrationJob::WaitDisconnectStations");
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveWithHostMigrationJob::ExecuteResult LeaveWithHostMigrationJob::WaitDisconnectStations()
{
    transport::Station** station;
    for (station = transport::StationManager::s_pInstance->m_stations.begin();
         station != transport::StationManager::s_pInstance->m_stations.end(); ++station)
    {
        if ((*station)->m_field20 == cStationState5
            && *station != transport::StationManager::s_pInstance->m_localStation)
            break;
    }
    if (station != transport::StationManager::s_pInstance->m_stations.end()
        && !(m_deadline <= common::g_piaJobScheduler->m_dispatchTime))
        return MakeExecuteResult(cExecuteKindNextStep);
    m_currentStep = Step(static_cast<StepFunction>(&LeaveWithHostMigrationJob::CleanupMesh),
                         "LeaveWithHostMigrationJob::CleanupMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

void LeaveWithHostMigrationJob::ReceiveMigrationResponse(StationIndex station)
{
    if (static_cast<int>(station) > ProcessHostMigrationJob::cStationIndexMax)
        return;
    m_stationResponses[station] = false;
}

void LeaveWithHostMigrationJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    CleanupStatus();
    m_deadline.m_tick = 0;
}

void LeaveWithHostMigrationJob::Trace(unsigned long long) const
{
}

void LeaveWithHostMigrationJob::CleanupStatus()
{
    m_callerContext = 0;
    m_started = false;
    m_newHost = static_cast<StationIndex>(ProcessHostMigrationJob::cStationIndexInvalid);
    OSBlockSet(m_stationResponses, 0, sizeof(m_stationResponses));
    m_migrationStarted = false;
}

}
}
}
