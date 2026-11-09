#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBWAITFORPROBES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBWAITFORPROBES_HPP

#include "nn/nex/Job.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class NATTraversalEngine;
class StationProbeList;

class JobWaitForProbes : public Job
{
public:
    JobWaitForProbes(unsigned int uiCallId, NATTraversalEngine* pEngine, StationProbeList* pProbeList,
                     unsigned int uiTimeout, bool bUseRelay, JobType jobType);
    virtual ~JobWaitForProbes();

    virtual void Execute();

    void ReportNATTraversalResults();

private:
    unsigned int m_uiCallId;
    NATTraversalEngine* m_pEngine;
    StationProbeList* m_pProbeList;
    Time m_deadline;
    bool m_bUseRelay;
};

}
}

#endif
