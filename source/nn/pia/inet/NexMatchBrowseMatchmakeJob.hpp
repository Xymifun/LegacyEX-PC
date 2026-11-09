#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHBROWSEMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHBROWSEMATCHMAKEJOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/session/BrowseMatchmakeJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchBrowseMatchmakeJob : public session::BrowseMatchmakeJob
{
public:
    NexMatchBrowseMatchmakeJob();
    virtual ~NexMatchBrowseMatchmakeJob();
    virtual void Trace(unsigned long long value) const;
    virtual void Cleanup();

    ExecuteResult BrowseMatchmake();
    ExecuteResult WaitBrowseMatchmake();
    ExecuteResult WaitFindSessionByOwner();
    ExecuteResult WaitFindSessionByParticipant();
    ExecuteResult WaitFindSessionBySessionId();
    ExecuteResult WaitRequestSessionInfo();

protected:
    virtual Result StartupImpl(session::IMatchmakeSession* matchmakeSession,
                               const session::SessionSearchCriteria* criteria);
    virtual Result StartupImpl(session::IMatchmakeSession* matchmakeSession, unsigned int sessionId);

private:
    common::CallContext m_callContext;
    NexMatchmakeSession* m_matchmakeSession;
    Result m_startupResult;
};

}
}
}

#endif
