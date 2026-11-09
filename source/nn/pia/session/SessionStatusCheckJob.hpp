#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_STATUS_CHECK_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_STATUS_CHECK_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class SessionStatusCheckJob : public common::StepSequenceJob
{
public:
    SessionStatusCheckJob();
    virtual ~SessionStatusCheckJob();
    virtual void Trace(unsigned long long value) const;
    Result Startup();
    void Cleanup();
    ExecuteResult CheckSessionStatus();
    ExecuteResult CheckSessionStatus4JointSession();

    common::CallContext m_callContext;
    bool m_field54;
    bool m_field55;
};

}
}
}

#endif
