#include "nn/pia/session/ProcessDestroyMeshJob.hpp"

#include "nn/pia/Common.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
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
    cStationIndexMax = 31,
    cDisconnectReasonMeshDestroyed = 3,
    cResultJoinFailed = 0x6439,
    cDefaultTimeoutMsec = 1000
};

typedef common::Job::ExecuteResult (ProcessDestroyMeshJob::*StepFunction)();

inline void SelectStep(common::StepSequenceJob::Step& slot, StepFunction function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

ProcessDestroyMeshJob::ProcessDestroyMeshJob()
    : common::StepSequenceJob(),
      m_endTime(),
      m_timeoutMsec(cDefaultTimeoutMsec),
      m_started(false)
{
}

ProcessDestroyMeshJob::~ProcessDestroyMeshJob()
{
}

bool ProcessDestroyMeshJob::Startup()
{
    if (m_started)
        return false;

    ProcessUpdateMeshJob* updateMeshJob = g_piaMesh->m_processUpdateMeshJob;
    if (updateMeshJob->m_started)
    {
        updateMeshJob->Cleanup();
        updateMeshJob->Reset(false);
    }

    ProcessHostMigrationJob* hostMigrationJob = g_piaMesh->m_processHostMigrationJob;
    if (hostMigrationJob != 0 && hostMigrationJob->m_started)
    {
        hostMigrationJob->Cleanup();
        hostMigrationJob->Reset(false);
    }

    LeaveMeshJob* leaveMeshJob = g_piaMesh->m_leaveMeshJob;
    if (leaveMeshJob->GetState() == cStateRunning || leaveMeshJob->GetState() == cStateWaiting)
        return false;

    JoinMeshJob* joinMeshJob = g_piaMesh->m_joinMeshJob;
    if (joinMeshJob->IsRunning())
    {
        const Result result(cResultJoinFailed);
        joinMeshJob->Cleanup(result);
        joinMeshJob->Reset(true);
        m_started = true;
        return false;
    }

    const common::TimeSpan timeout = common::TimeSpan::FromMSec(m_timeoutMsec);
    m_endTime = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_started = true;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReasonMeshDestroyed));
    Reset(true);
    SelectStep(m_currentStep, &ProcessDestroyMeshJob::SendDestroyResponse,
               "ProcessDestroyMeshJob::SendDestroyResponse");
    return true;
}

ProcessDestroyMeshJob::ExecuteResult ProcessDestroyMeshJob::SendDestroyResponse()
{
    const int hostStationIndex = g_piaMesh->GetHostStationIndex();
    if (hostStationIndex > cStationIndexMax
        || g_piaMesh->GetMeshProtocol()->SendDestroyResponse(static_cast<StationIndex>(hostStationIndex))
        || m_endTime <= common::g_piaJobScheduler->m_dispatchTime)
    {
        SelectStep(m_currentStep, &ProcessDestroyMeshJob::CleanupMesh, "ProcessDestroyMeshJob::CleanupMesh");
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessDestroyMeshJob::ExecuteResult ProcessDestroyMeshJob::CleanupMesh()
{
    for (transport::Station** station = transport::StationManager::s_pInstance->m_stations.begin();
         station != transport::StationManager::s_pInstance->m_stations.end(); ++station)
    {
        (*station)->m_disconnectStationJob->ProcessDisconnection(*station);
    }

    g_piaMesh->CleanupStationsJobs();
    g_piaMesh->m_disconnectReason = cDisconnectReasonMeshDestroyed;
    g_piaMesh->CleanupStatus();
    m_started = false;
    return MakeExecuteResult(cExecuteKindFinished);
}

void ProcessDestroyMeshJob::Cleanup()
{
    m_started = false;
}

void ProcessDestroyMeshJob::Trace(unsigned long long) const
{
}

}
}
}
