#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCLEARSYSTEMPASSWORDJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCLEARSYSTEMPASSWORDJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/ClearMatchmakeSystemPasswordJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchClearSystemPasswordJob : public session::ClearMatchmakeSystemPasswordJob
{
public:
    NexMatchClearSystemPasswordJob();
    virtual ~NexMatchClearSystemPasswordJob();
    virtual Result StartupImpl();
    ExecuteResult ClearMatchmakeSystemPassword();
    ExecuteResult WaitClearMatchmakeSystemPassword();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    NexMatchmakeSession* m_matchmakeSession;
};

}
}
}

#endif
