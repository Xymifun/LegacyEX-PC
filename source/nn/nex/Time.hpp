#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIME_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class TimeProvider;

extern uint64_t g_lastTime;

class Time : public RootObject
{
public:
    Time() : m_value(0) {}
    explicit Time(uint64_t value) : m_value(value) {}

    Time& operator=(const Time& other)
    {
        m_value = other;
        return *this;
    }

    Time& operator=(uint64_t value)
    {
        m_value = value;
        return *this;
    }

    operator unsigned long long() const { return m_value; }

    Time operator+(int milliseconds) const
    {
        if (milliseconds < 0)
            return Time(m_value - static_cast<int64_t>(-milliseconds));

        return Time(m_value + static_cast<int64_t>(milliseconds));
    }

    Time operator-(const Time& other) const { return Time(m_value - other.m_value); }

    Time Multiply(float factor) const;
    Time Divide(float divisor) const;
    Time Scale(float factor) const;

    static Time GetTime();
    static void Reset();
    static void RegisterTimeProvider(TimeProvider* provider);
    static Time ConvertTimeoutToDeadline(uint32_t timeout);
    static int32_t ConvertDeadlineToTimeout(Time deadline);

private:
    uint64_t m_value;
};

}
}

#endif
