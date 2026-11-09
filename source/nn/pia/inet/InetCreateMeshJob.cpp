#include "nn/pia/inet/InetCreateMeshJob.hpp"

#include "nn/nex/Credentials.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult (InetCreateMeshJob::*Step)();

const unsigned int cMonitoringValueUnset = 0xffffffffu;

enum
{
    cMonitoringStationNum = 15,
    cMonitoringByteUnset = 0xff
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

InetCreateMeshJob::InetCreateMeshJob()
{
}

InetCreateMeshJob::~InetCreateMeshJob()
{
}

Result InetCreateMeshJob::StartupImpl()
{
    Reset(true);
    SelectStep(m_currentStep, &CreateMeshJob::SetupLocalPlayerInfo, "InetCreateMeshJob::SetupLocalPlayerInfo");
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
    return 0;
}

void InetCreateMeshJob::SetHostInfoToMonitoringDataImpl()
{
    NexFacade* facade = NexFacade::GetInstance();
    if (facade && facade->m_matchmakeExtensionClient && facade->m_matchmakeExtensionClient->GetCredentials())
        common::g_sessionBeginMonitoringContent.m_metric71 = common::hashWithMd5(
            facade->m_matchmakeExtensionClient->GetCredentials()->GetPrincipalID());
}

void InetCreateMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl()
{
    SelectStep(m_currentStep, &InetCreateMeshJob::StartMonitoringServerAddressResolution,
               "InetCreateMeshJob::StartMonitoringServerAddressResolution");
}

InetCreateMeshJob::ExecuteResult InetCreateMeshJob::StartMonitoringServerAddressResolution()
{
    session::g_piaMesh->SetupSendReport();
    SelectStep(m_currentStep, &CreateMeshJob::SetupLocalStation, "InetCreateMeshJob::SetupLocalStation");
    return MakeExecuteResult(cExecuteKindContinue);
}

void InetCreateMeshJob::CleanupImpl()
{
}

}
}
}
