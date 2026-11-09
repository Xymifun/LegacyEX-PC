#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_MESH_JOB_HPP
#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
namespace nn
{
namespace pia
{
namespace session
{
class DestroyMeshJob : public common::StepSequenceJob
{
public:
    enum
    {
        cStationNumMax = 32,
        cStationIndexMax = cStationNumMax - 1,
        cResponseTimeoutMsec = 5000
    };

    DestroyMeshJob();
    virtual ~DestroyMeshJob();
    bool Startup(common::CallContext* callContext, bool systemDestroy);
    bool AssociateSystemWith(common::CallContext* callContext);
    ExecuteResult SendDestroyMesh();
    ExecuteResult WaitDestroyResponse();
    void CleanupStatus();
    ExecuteResult CleanupMesh();
    void ReceiveDestroyResponse(StationIndex stationIndex);
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

    void setStep(ExecuteResult (DestroyMeshJob::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }

    common::CallContext* m_callerContext;
    common::Time m_deadline;
    int m_timeoutMsec;
    unsigned char m_stationResponses[cStationNumMax];
    bool m_waitingResponses;
    bool m_started;
    bool m_systemDestroy;
};
}
}
}
#endif
