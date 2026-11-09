#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_MESH_JOB_HPP
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
class LeaveMeshJob : public common::StepSequenceJob
{
public:
    enum
    {
        cLeaveResponseTimeoutMsec = 5000
    };

    LeaveMeshJob();
    virtual ~LeaveMeshJob();
    bool Startup(common::CallContext* callerContext);
    ExecuteResult SendLeaveRequest();
    ExecuteResult WaitLeaveResponse();
    ExecuteResult StartDisconnectStations();
    ExecuteResult WaitLeavingProcess();
    ExecuteResult LeaveSuccess();
    void Cleanup();
    unsigned int RegisterExtraCallback(common::CallContext* callContext);
    virtual void Trace(unsigned long long value) const;

    common::Time m_deadline;
    int m_timeoutMsec;
    common::CallContext* m_callerContext;
    common::CallContext* m_extraContext;
    bool m_disconnectStarted;
};
}
}
}
#endif
