#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCHECKCURRENTPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCHECKCURRENTPORT_HPP

#include "nn/nex/JobNATCheckBase.hpp"

namespace nn
{
namespace nex
{

class JobCheckCurrentPort : public JobNATCheckBase
{
public:
    JobCheckCurrentPort(CallContext* pCallContext, const InetAddress& rAddress, bool bRelayProbe);

    virtual void StepInitializeImpl();
    virtual bool Finished();
    virtual qResult HandleResult();

    void StepPrepareSocket();
    void StepSend();

private:
    bool m_bRelayProbe;
};

}
}

#endif
