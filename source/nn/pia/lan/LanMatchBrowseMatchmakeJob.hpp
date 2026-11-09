#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHBROWSEMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHBROWSEMATCHMAKEJOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/session/BrowseMatchmakeJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchmakeSession;

class LanMatchBrowseMatchmakeJob : public session::BrowseMatchmakeJob
{
public:
    LanMatchBrowseMatchmakeJob();
    virtual ~LanMatchBrowseMatchmakeJob();
    virtual void Trace(unsigned long long value) const;
    virtual void Cleanup();

    ExecuteResult BrowseMatchmake();
    ExecuteResult WaitBrowseMatchmake();
    ExecuteResult WaitRequestSessionInfo();

protected:
    virtual Result StartupImpl(session::IMatchmakeSession* matchmakeSession,
                               const session::SessionSearchCriteria* criteria);
    virtual Result StartupImpl(session::IMatchmakeSession* matchmakeSession, unsigned int sessionId);

private:
    common::CallContext m_callContext;
    LanMatchmakeSession* m_matchmakeSession;
};

}
}
}

#endif
