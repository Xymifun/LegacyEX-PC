#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_KICKOUT_MANAGE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_KICKOUT_MANAGE_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class KickoutManageJob : public common::StepSequenceJob
{
public:
    enum
    {
        cKickoutStationNumMax = 32
    };

    enum KickoutReason
    {
        cKickoutReasonNone = 0,
        cKickoutReasonRequested = 2,
        cKickoutReasonRelayFailureBase = 3
    };

    class KickoutStation
    {
    public:
        StationIndex m_stationIndex;
        KickoutReason m_kickoutReason;
    };

    KickoutManageJob();
    virtual ~KickoutManageJob();
    virtual void Trace(unsigned long long value) const;
    virtual void SetKickoutFlag();
    virtual void KickoutFromNetwork(const common::StationAddress& stationAddress);
    bool Startup();
    void Cleanup();
    bool StartupClient(KickoutReason kickoutReason);
    Result StartKickout(StationIndex stationIndex, KickoutReason kickoutReason);
    ExecuteResult ClientStartLeaveMesh();
    ExecuteResult ClientWaitLeaveMesh();
    ExecuteResult ClientFinalize();
    void SetLeaveEventStationIndex(StationIndex stationIndex);
    bool AssociateKickoutWith(common::CallContext* callContext);

    void setStep(ExecuteResult (KickoutManageJob::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }

    KickoutStation m_kickoutStations[cKickoutStationNumMax];
    KickoutReason m_kickoutReason;
    common::CallContext m_callContext;
    common::CallContext* m_kickoutCallContext;
    bool m_joinCancelRequested;
    Result m_joinCancelResult;
};

}
}
}

#endif
