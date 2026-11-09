#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCHECKNATPROPERTIES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCHECKNATPROPERTIES_HPP

#include <stdint.h>

#include "nn/nex/JobNATCheckBase.hpp"

namespace nn
{
namespace nex
{

class JobCheckNATProperties : public JobNATCheckBase
{
public:
    JobCheckNATProperties(CallContext* pCallContext, const InetAddress& rAddress);

    virtual void StepInitializeImpl();
    virtual bool Finished();
    virtual qResult HandleResult();

    void StepPrepareSocket();
    void StepSend();

private:
    uint64_t m_sendTime;
    unsigned int m_uiRoundTripTime;
};

}
}

#endif
