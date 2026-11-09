#include "nn/nex/JobWaitForProbes.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/StationProbeList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    WAIT_CONTEXT_PROBES = 50
};

enum ProbeResult
{
    PROBE_TIMED_OUT_RESULT = static_cast<int>(0x8001000Bu),
    PROBE_FAILED_RESULT = static_cast<int>(0x80050020u),
    PROBE_SUCCEEDED_RESULT = 0x10001
};

}

JobWaitForProbes::JobWaitForProbes(unsigned int uiCallId, NATTraversalEngine* pEngine, StationProbeList* pProbeList,
                                   unsigned int uiTimeout, bool bUseRelay, JobType jobType)
    : Job(jobType, DebugString(L"")),
      m_uiCallId(uiCallId),
      m_pEngine(pEngine),
      m_pProbeList(pProbeList),
      m_deadline(Time::ConvertTimeoutToDeadline(uiTimeout)),
      m_bUseRelay(bUseRelay)
{
}

JobWaitForProbes::~JobWaitForProbes()
{
}

void JobWaitForProbes::ReportNATTraversalResults()
{
    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    if (pEngine == 0)
        return;

    for (StationProbeList::iterator probe = m_pProbeList->begin(); probe != m_pProbeList->end(); ++probe)
    {
        unsigned int uiConnectionID = probe->m_contactInfo.GetConnectionID();
        if (uiConnectionID == 0)
            continue;

        if (probe->m_bFailed)
            pEngine->ReportNATTraversalResult(uiConnectionID, false, probe->m_iPingTime);
        else if (probe->m_iPingTime != -1)
            pEngine->ReportNATTraversalResult(uiConnectionID, true, probe->m_iPingTime);
        else
            pEngine->ReportNATTraversalResult(uiConnectionID, false, probe->m_iPingTime);
    }
}

void JobWaitForProbes::Execute()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetToComplete();
        return;
    }

    m_pProbeList->Update();
    if (m_pProbeList->EveryoneReached())
    {
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(PROBE_SUCCEEDED_RESULT), true);
        SetToComplete();
        return;
    }

    if (!m_bUseRelay && m_pProbeList->AnyoneFailed())
    {
        ReportNATTraversalResults();
        pCallContext->SignalFailure(qResult(PROBE_FAILED_RESULT));
        SetToComplete();
        return;
    }

    if (m_deadline < Time::GetTime())
    {
        ReportNATTraversalResults();
        pCallContext->SignalFailure(qResult(PROBE_TIMED_OUT_RESULT));
        SetToComplete();
        return;
    }

    SetToWaiting(WAIT_CONTEXT_PROBES);
}

}
}
