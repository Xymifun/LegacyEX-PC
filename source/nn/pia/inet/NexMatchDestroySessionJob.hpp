#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHDESTROYSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHDESTROYSESSIONJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/DestroySessionJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchDestroySessionJob : public session::DestroySessionJob
{
public:
    NexMatchDestroySessionJob();
    virtual ~NexMatchDestroySessionJob();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    ExecuteResult UnregisterCurrentMatchmakeSession();
    ExecuteResult WaitUnregisterCurrentMatchmakeSession();
    ExecuteResult UnregisterBufferMatchmakeSession();
    ExecuteResult WaitUnregisterBufferMatchmakeSession();

protected:
    virtual Result StartupImpl();
    virtual void SetNextStepAtWaitDestroyMesh();
    virtual void SetNextStepAtMeshCleanup();
};

}
}
}

#endif
