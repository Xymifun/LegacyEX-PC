#include "nn/nex/Time.hpp"

#include "nn/nex/SystemClock.hpp"

namespace nn
{
namespace nex
{

uint64_t g_lastTime;

namespace
{

const uint32_t INFINITE_TIMEOUT = 0xffffffffu;

}

Time Time::GetTime()
{
    Time time = g_systemClockCorrectionEnabled ? SystemClock::GetTimeImpl(false)
                                               : SystemClock::GetTimeImplCorrectless();

    g_lastTime = time;
    return time;
}

Time Time::ConvertTimeoutToDeadline(uint32_t timeout)
{
    if (timeout == INFINITE_TIMEOUT)
        return Time(0);

    return GetTime() + static_cast<int32_t>(timeout);
}

int32_t Time::ConvertDeadlineToTimeout(Time deadline)
{
    if (deadline == 0)
        return -1;

    Time now(GetTime());
    if (now < deadline)
        return static_cast<int32_t>(deadline - now);

    return 0;
}

}
}
