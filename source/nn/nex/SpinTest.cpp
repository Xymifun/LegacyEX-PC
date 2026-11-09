#include "nn/nex/SpinTest.hpp"

#include "nn/nex/Platform.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

SpinTest::SpinTest(uint32_t sleepInterval, uint32_t timeout) : m_sleepInterval(sleepInterval)
{
    m_timeout = timeout;
    m_started = Time::GetTime();
    m_timedOut = false;
    m_traceCount = 0;
    m_limit = 1000;
    m_elapsed = 0;
    m_trace = 8;
    m_state = 1;
}

void SpinTest::LeaveOnTimeout()
{
    m_state = 2;
}

int32_t SpinTest::GetRemainingTime()
{
    int32_t remaining = m_timeout;

    if (remaining != -1)
    {
        m_elapsed = Time::GetTime() - m_started;
        remaining = m_timeout > m_elapsed ? m_timeout - m_elapsed : 0;
    }

    return remaining;
}

uint32_t SpinTest::SpinOnce(wchar_t*, uint32_t, wchar_t*)
{
    Platform::Sleep(m_sleepInterval);

    m_elapsed = Time::GetTime() - m_started;

    if (m_elapsed > m_timeout && m_timeout != 0xffffffffu)
    {
        m_timedOut = true;

        if (m_state == 2)
            return 0;
    }

    return 1;
}

}
}
