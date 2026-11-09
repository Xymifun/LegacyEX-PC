#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TIME_HPP

#include "nn/pia/common/TimeSpan.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Time
{
public:
    Time() : m_tick(0) {}

    bool operator<=(const Time& other) const { return m_tick <= other.m_tick; }
    bool operator<(const Time& other) const { return m_tick < other.m_tick; }

    Time operator+(const TimeSpan& span) const
    {
        Time time;
        time.m_tick = m_tick + span.m_tick;
        return time;
    }

    TimeSpan operator-(const Time& other) const { return TimeSpan(m_tick - other.m_tick); }

    void SetNow();
    static Time Create(unsigned long long tick);
    static const Time c_Max;
    static const Time c_Min;
    void Trace(unsigned long long value) const;

    unsigned long long m_tick;
};

}
}
}

#endif
