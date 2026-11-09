#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_PROCESS_JOIN_REQUEST_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_PROCESS_JOIN_REQUEST_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class ProcessJoinRequestJob : public common::StepSequenceJob
{
public:
    enum
    {
        cResponsePartMax = 3
    };

    ProcessJoinRequestJob();
    virtual ~ProcessJoinRequestJob();
    virtual void Trace(unsigned long long) const;
    bool Startup();
    ExecuteResult InitialStep();
    ExecuteResult CheckApprovalJoin();
    ExecuteResult SendDenyingJoinResponse();
    ExecuteResult SendJoinResponse();
    ExecuteResult WaitResponseAck();
    ExecuteResult JoinSucceeded();
    void SetJoiningStationData(const common::StationAddress&);
    void CancellationNotice(StationIndex);
    void Cleanup();

    void setStep(ExecuteResult (ProcessJoinRequestJob::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }

    common::Time m_deadline;
    int m_timeoutMsec;
    unsigned int m_unknown4c;
    unsigned int m_responseAckIds[cResponsePartMax];
    unsigned char* m_responseBuffers[cResponsePartMax];
    common::StationAddress m_joiningStationAddress;
    StationIndex m_stationIndex;
    bool m_unknown78;
    unsigned char m_denyReason;
    bool m_joinCancelRequested;
};

}
}
}

#endif
