#include "nn/nex/RVConnectivityTester.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/JobWaitForProbes.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationProbeList.hpp"

namespace nn
{
namespace nex
{

RVConnectivityTester::RVConnectivityTester()
{
}

RVConnectivityTester::~RVConnectivityTester()
{
}

bool RVConnectivityTester::ProbeStations(CallContext* pCallContext, StationProbeList* pStationProbeList,
                                         unsigned int uiTimeout, bool bUseRelay, Job::JobType jobType)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    Network::GetInstance()->GetConnectivityManager().InitiateProbes(*pStationProbeList);
    if (!pCallContext->InitiateCall())
        return false;

    JobWaitForProbes* pJob = new JobWaitForProbes(pCallContext->GetCallID(),
                                                  Network::GetInstance()->GetNATTraversalEngine(), pStationProbeList,
                                                  uiTimeout, bUseRelay, jobType);
    Scheduler::GetInstance()->Queue(pJob, false);
    return true;
}

}
}
