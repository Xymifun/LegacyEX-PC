#ifndef NN_PIA_SESSION_PROCESSDESTROYMESHJOB_HPP
#define NN_PIA_SESSION_PROCESSDESTROYMESHJOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class ProcessDestroyMeshJob : public common::StepSequenceJob
{
public:
    ProcessDestroyMeshJob();
    virtual ~ProcessDestroyMeshJob();

    bool Startup();
    ExecuteResult SendDestroyResponse();
    ExecuteResult CleanupMesh();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

    common::Time m_endTime;
    int m_timeoutMsec;
    bool m_started;
};

}
}
}

#endif
