#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBPROTOCOLCALLCONTEXTCALLBACK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBPROTOCOLCALLCONTEXTCALLBACK_HPP

#include "nn/nex/JobCallContextCallback.hpp"
#include "nn/nex/ProtocolCallContext.hpp"

namespace nn
{
namespace nex
{

class JobProtocolCallContextCallback : public JobCallContextCallback
{
public:
    JobProtocolCallContextCallback(JobType jobType, const wchar_t* pName, CallContext* pCallContext)
        : JobCallContextCallback(jobType, pName, pCallContext)
    {
    }

    virtual ~JobProtocolCallContextCallback() {}

    ProtocolCallContext* GetProtocolCallContext();
    virtual CallContext* CreateCallContext();
};

}
}

#endif
