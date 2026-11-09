#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CLOSE_PARTICIPATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CLOSE_PARTICIPATION_JOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class IMatchmakeSession;

class CloseParticipationJob : public common::StepSequenceJob
{
public:
    CloseParticipationJob();
    virtual ~CloseParticipationJob();

    Result Startup(common::CallContext* callerContext, unsigned int sessionId, IMatchmakeSession* matchmakeSession);
    ExecuteResult CloseParticipation();
    ExecuteResult WaitCloseParticipation();
    ExecuteResult WaitP2pStable();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    unsigned int m_sessionId;
    IMatchmakeSession* m_matchmakeSession;
    common::Time m_startTime;
};

}
}
}

#endif
