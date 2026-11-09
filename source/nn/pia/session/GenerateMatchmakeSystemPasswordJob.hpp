#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_GENERATEMATCHMAKESYSTEMPASSWORDJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_GENERATEMATCHMAKESYSTEMPASSWORDJOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class GenerateMatchmakeSystemPasswordJob : public common::StepSequenceJob
{
public:
    GenerateMatchmakeSystemPasswordJob();
    virtual ~GenerateMatchmakeSystemPasswordJob();
    Result Startup(common::CallContext* callerContext, unsigned int sessionId, IMatchmakeSession* session);
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
