#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATSERVERADDRESSRESOLVEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATSERVERADDRESSRESOLVEJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexNatServerAddressResolveJob : public common::StepSequenceJob
{
public:
    NexNatServerAddressResolveJob();
    virtual ~NexNatServerAddressResolveJob();
    virtual void CancelCleanup();
    Result Startup(common::CallContext* callContext);
    void Cleanup();
    ExecuteResult StepResolveNatServerAddress();
    ExecuteResult StepRequestRelayServerSetting();
    ExecuteResult StepWaitRequestRelayServerSetting();
    ExecuteResult StepResolveRelayServerSetting();
    ExecuteResult StepComplete();

private:
    common::CallContext* m_callContext;
    common::Time m_deadline;
    Result m_result;
};

}
}
}

#endif
