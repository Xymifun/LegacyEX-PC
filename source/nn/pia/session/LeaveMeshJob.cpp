#include "nn/pia/session/LeaveMeshJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
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
enum
{
    cDisconnectReasonLeave = 2,
    cConnectedStationState = 5,
    cTraceStationLevel = 0x10000000
};

inline void SelectStep(common::StepSequenceJob::Step& slot, LeaveMeshJob::ExecuteResult (LeaveMeshJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<LeaveMeshJob::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

LeaveMeshJob::LeaveMeshJob()
    : common::StepSequenceJob(),
      m_deadline(),
      m_timeoutMsec(cLeaveResponseTimeoutMsec),
      m_callerContext(0),
      m_extraContext(0),
      m_disconnectStarted(false)
{
}

LeaveMeshJob::~LeaveMeshJob()
{
}

bool LeaveMeshJob::Startup(common::CallContext* callerContext)
{
    if (GetState() == cStateRunning || GetState() == cStateWaiting || GetState() == cStateReady)
        return false;
    if (callerContext != 0)
    {
        m_callerContext = callerContext;
        callerContext->InitiateCall();
    }
    const int timeoutMsec = m_timeoutMsec;
    const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
    m_deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick + ticksPerMSec.m_tick * timeoutMsec;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReasonLeave));
    Reset(true);
    SelectStep(m_currentStep, &LeaveMeshJob::SendLeaveRequest, "LeaveMeshJob::SendLeaveRequest");
    ProcessUpdateMeshJob* processUpdateMeshJob = g_piaMesh->m_processUpdateMeshJob;
    if (processUpdateMeshJob->m_started)
    {
        processUpdateMeshJob->Cleanup();
        processUpdateMeshJob->m_cancelRequested = true;
    }
    ProcessHostMigrationJob* processHostMigrationJob = g_piaMesh->m_processHostMigrationJob;
    if (processHostMigrationJob != 0 && processHostMigrationJob->m_started)
    {
        processHostMigrationJob->Cleanup();
        processHostMigrationJob->m_cancelRequested = true;
    }
    return true;
}

LeaveMeshJob::ExecuteResult LeaveMeshJob::SendLeaveRequest()
{
    g_piaMesh->GetMeshProtocol()->SendLeaveRequest();
    g_piaMesh->GetMeshProtocol()->SetWaitingLeaveResponseJob(this);
    m_disconnectStarted = true;
    SelectStep(m_currentStep, &LeaveMeshJob::WaitLeaveResponse, "LeaveMeshJob::WaitLeaveResponse");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveMeshJob::ExecuteResult LeaveMeshJob::WaitLeaveResponse()
{
    if (m_disconnectStarted)
    {
        if (m_deadline <= common::g_piaJobScheduler->m_dispatchTime)
        {
            const int timeoutMsec = m_timeoutMsec;
            const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
            m_deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick + ticksPerMSec.m_tick * timeoutMsec;
            m_disconnectStarted = false;
            SelectStep(m_currentStep, &LeaveMeshJob::StartDisconnectStations, "LeaveMeshJob::StartDisconnectStations");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (transport::StationManager::s_pInstance->m_localStation != 0)
            return MakeExecuteResult(cExecuteKindNextStep);
        m_disconnectStarted = false;
        SelectStep(m_currentStep, &LeaveMeshJob::StartDisconnectStations, "LeaveMeshJob::StartDisconnectStations");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, &LeaveMeshJob::StartDisconnectStations, "LeaveMeshJob::StartDisconnectStations");
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveMeshJob::ExecuteResult LeaveMeshJob::StartDisconnectStations()
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
    {
        SelectStep(m_currentStep, &LeaveMeshJob::LeaveSuccess, "LeaveMeshJob::LeaveSuccess");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
    if (station != station + transport::StationManager::s_pInstance->m_stations.size)
    {
        do
        {
            if (*station != localStation)
            {
                if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                    g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>((*station)->m_field10));
                transport::DisconnectStationJob* disconnectJob = (*station)->m_disconnectStationJob;
                if (disconnectJob != 0 && disconnectJob->Startup(*station) == 0)
                    disconnectJob->Ready(false);
                else
                    (*station)->Trace(cTraceStationLevel);
            }
            ++station;
        } while (station
                 != transport::StationManager::s_pInstance->m_stations.data
                        + transport::StationManager::s_pInstance->m_stations.size);
    }
    SelectStep(m_currentStep, &LeaveMeshJob::WaitLeavingProcess, "LeaveMeshJob::WaitLeavingProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveMeshJob::ExecuteResult LeaveMeshJob::WaitLeavingProcess()
{
    bool connected = false;
    for (transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
         station
         != transport::StationManager::s_pInstance->m_stations.data
                + transport::StationManager::s_pInstance->m_stations.size;
         ++station)
    {
        if ((*station)->m_field20 == cConnectedStationState
            && *station != transport::StationManager::s_pInstance->m_localStation)
        {
            connected = true;
            break;
        }
    }
    if (connected && !(m_deadline <= common::g_piaJobScheduler->m_dispatchTime))
        return MakeExecuteResult(cExecuteKindNextStep);
    SelectStep(m_currentStep, &LeaveMeshJob::LeaveSuccess, "LeaveMeshJob::LeaveSuccess");
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveMeshJob::ExecuteResult LeaveMeshJob::LeaveSuccess()
{
    for (transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
         station
         != transport::StationManager::s_pInstance->m_stations.data
                + transport::StationManager::s_pInstance->m_stations.size;
         ++station)
        (*station)->m_disconnectStationJob->ProcessDisconnection(*station);
    g_piaMesh->CleanupStationsJobs();
    g_piaMesh->m_disconnectReason = cDisconnectReasonLeave;
    g_piaMesh->CleanupStatus();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalSuccess(Result(0));
        m_callerContext = 0;
    }
    if (m_extraContext != 0)
    {
        if (m_extraContext->m_state == common::CallContext::cStateInProgress)
            m_extraContext->SignalSuccess(Result(0));
        m_extraContext = 0;
    }
    return MakeExecuteResult(cExecuteKindFinished);
}

void LeaveMeshJob::Cleanup()
{
    m_disconnectStarted = false;
    m_deadline = common::Time();
    common::CallContext* callerContext = m_callerContext;
    if (callerContext != 0)
    {
        if (callerContext->m_state == common::CallContext::cStateInProgress)
            callerContext->SignalSuccess(Result(0));
        m_callerContext = 0;
    }
    if (m_extraContext != 0)
    {
        if (m_extraContext->m_state == common::CallContext::cStateInProgress)
            m_extraContext->SignalSuccess(Result(0));
        m_extraContext = 0;
    }
}

unsigned int LeaveMeshJob::RegisterExtraCallback(common::CallContext* callContext)
{
    if (m_extraContext != 0)
        return 0;
    m_extraContext = callContext;
    callContext->InitiateCall();
    return 1;
}

void LeaveMeshJob::Trace(unsigned long long) const
{
}

}
}
}
