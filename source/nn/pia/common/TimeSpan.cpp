#include "nn/pia/common/TimeSpan.hpp"

#include <coreinit/systeminfo.h>

namespace
{
long long GetFrequency()
{
    const unsigned int busClockSpeed = OSGetSystemInfo()->busClockSpeed;
    return static_cast<long long>(busClockSpeed >> 2);
}
}

namespace nn
{
namespace pia
{
namespace common
{

void TimeSpan::Trace(unsigned long long) const
{
}

const TimeSpan& TimeSpan::GetTicksPerMSec()
{
    static const TimeSpan s_ticksPerMSec(GetFrequency() / 1000);
    return s_ticksPerMSec;
}

const TimeSpan& TimeSpan::GetTicksPerSec()
{
    static const TimeSpan s_ticksPerSec(GetTicksPerMSec().m_tick * 1000);
    return s_ticksPerSec;
}

}
}
}
