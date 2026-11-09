#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TIME_SPAN_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TIME_SPAN_HPP

namespace nn
{
namespace pia
{
namespace common
{

class TimeSpan
{
public:
    TimeSpan() : m_tick(0) {}
    explicit TimeSpan(long long tick) : m_tick(tick) {}

    bool operator<=(const TimeSpan& other) const { return m_tick <= other.m_tick; }
    bool operator<(const TimeSpan& other) const { return m_tick < other.m_tick; }

    void Trace(unsigned long long value) const;
    static const TimeSpan& GetTicksPerUSec();
    static const TimeSpan& GetTicksPerMSec();
    static const TimeSpan& GetTicksPerSec();

    static TimeSpan FromMSec(int msec)
    {
        TimeSpan span;
        span.m_tick = GetTicksPerMSec().m_tick * msec;
        return span;
    }

    long long m_tick;
};

}
}
}

#endif
