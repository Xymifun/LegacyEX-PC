#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_OPEN_PARTICIPATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_OPEN_PARTICIPATION_JOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/CallContext.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class IMatchmakeSession;

class OpenParticipationJob : public common::StepSequenceJob
{
public:
    OpenParticipationJob();
    virtual ~OpenParticipationJob();

    Result Startup(common::CallContext* callerContext, unsigned int sessionId, IMatchmakeSession* matchmakeSession);
    ExecuteResult OpenParticipation();
    ExecuteResult WaitOpenParticipation();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    unsigned int m_sessionId;
    IMatchmakeSession* m_matchmakeSession;
};

}
}
}

#endif
