#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_MESH_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class CreateMeshJob : public common::StepSequenceJob
{
public:
    CreateMeshJob();
    virtual ~CreateMeshJob();

    void SetHostInfoToMonitoringData();
    Result Startup(common::CallContext* callerContext);
    ExecuteResult SetupLocalPlayerInfo();
    void Cleanup();
    ExecuteResult CompleteFailure();
    ExecuteResult SetupLocalStation();

    virtual void Trace(unsigned long long value) const;
    virtual Result StartupImpl();
    virtual void CleanupImpl();
    virtual void SetHostInfoToMonitoringDataImpl();
    virtual void SetNextStepAtSetupLocalPlayerInfoImpl();

    common::CallContext* m_callerContext;
};

}
}
}

#endif
