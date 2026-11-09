#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SPINTEST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SPINTEST_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class SpinTest : public RootObject
{
public:
    SpinTest(uint32_t sleepInterval, uint32_t timeout);
    virtual ~SpinTest() {}

    void SetTrace(uint32_t trace, uint64_t flags);
    void AssertOnTimeout();
    void LeaveOnTimeout();

    uint32_t GetElapsed();
    uint32_t GetTimeout();
    int32_t GetRemainingTime();
    uint32_t SpinOnce(wchar_t* file, uint32_t line, wchar_t* state);

private:
    uint16_t m_state;
    uint32_t m_timeout;
    Time m_started;
    uint32_t m_elapsed;
    uint32_t m_sleepInterval;
    uint32_t m_limit;
    uint32_t m_reserved;
    uint32_t m_trace;
    uint32_t m_traceCount;
    bool m_timedOut;
};

}
}

#endif
