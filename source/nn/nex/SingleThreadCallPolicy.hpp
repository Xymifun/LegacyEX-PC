#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SINGLETHREADCALLPOLICY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SINGLETHREADCALLPOLICY_HPP

#include <stdint.h>

#include "nn/nex/CallPolicy.hpp"

namespace nn
{
namespace nex
{

class SingleThreadCallPolicy : public CallPolicy
{
public:
    SingleThreadCallPolicy();
    virtual ~SingleThreadCallPolicy() {}

    void AcceptOneThread(uint32_t threadId);
    void AcceptAnyThread();
    void SetMaximumRecursionLevel(uint32_t maximumRecursionLevel);
    bool CurrentThreadIsDispatchingJobs() const;

    virtual void BeginCallImpl();
    virtual void EndCallImpl();

private:
    uint32_t m_callDepth;
    uint32_t m_threadId;
    uint32_t m_maximumRecursionLevel;
    uint32_t m_recursionLevel;
};

}
}

#endif
