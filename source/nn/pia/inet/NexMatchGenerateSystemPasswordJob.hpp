#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHGENERATESYSTEMPASSWORDJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHGENERATESYSTEMPASSWORDJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/GenerateMatchmakeSystemPasswordJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchGenerateSystemPasswordJob : public session::GenerateMatchmakeSystemPasswordJob
{
public:
    NexMatchGenerateSystemPasswordJob();
    virtual ~NexMatchGenerateSystemPasswordJob();
    virtual Result StartupImpl();
    ExecuteResult GenerateMatchmakeSystemPassword();
    ExecuteResult WaitGenerateMatchmakeSystemPassword();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

private:
    NexMatchmakeSession* m_matchmakeSession;
};

}
}
}

#endif
