#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHDESTROYSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHDESTROYSESSIONJOB_HPP

#include "nn/pia/session/DestroySessionJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchDestroySessionJob : public session::DestroySessionJob
{
public:
    LanMatchDestroySessionJob();
    virtual ~LanMatchDestroySessionJob();
    ExecuteResult UnregisterCurrentMatchmakeSession();
    ExecuteResult WaitUnregisterCurrentMatchmakeSession();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    virtual Result StartupImpl();
    virtual void SetNextStepAtWaitDestroyMesh();
    virtual void SetNextStepAtMeshCleanup();
};

}
}
}

#endif
