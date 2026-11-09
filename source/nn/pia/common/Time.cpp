#include "nn/pia/common/Time.hpp"

#include <coreinit/time.h>

namespace nn
{
namespace pia
{
namespace common
{

const Time Time::c_Max = Time::Create(0xffffffffffffffffULL);
const Time Time::c_Min;

void Time::SetNow()
{
    const unsigned long long currentTime = static_cast<unsigned long long>(OSGetSystemTime());
    m_tick = currentTime;
}

Time Time::Create(unsigned long long tick)
{
    Time time;
    time.m_tick = tick;
    return time;
}

}
}
}
