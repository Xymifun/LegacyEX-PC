#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CLEAR_MATCHMAKE_SYSTEM_PASSWORD_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CLEAR_MATCHMAKE_SYSTEM_PASSWORD_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class IMatchmakeSession;

class ClearMatchmakeSystemPasswordJob : public common::StepSequenceJob
{
public:
    ClearMatchmakeSystemPasswordJob();
    virtual ~ClearMatchmakeSystemPasswordJob();
    Result Startup(common::CallContext* callerContext, unsigned int sessionId, IMatchmakeSession* matchmakeSession);
    ExecuteResult CompleteProcess();
    ExecuteResult FailureProcess();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual Result StartupImpl() = 0;

protected:
    unsigned int m_sessionId;
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
};

}
}
}

#endif
