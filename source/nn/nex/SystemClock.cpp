#include "nn/nex/SystemClock.hpp"

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HighResolutionClock.hpp"
#include "nn/nex/TimeProvider.hpp"

namespace nn
{
namespace nex
{
namespace
{

TimeProvider* s_timeProvider = 0;
int64_t s_correction = 0;
bool s_initialized = false;
CriticalSection s_criticalSection(1);

inline Time GetRawTime()
{
    if (s_timeProvider != 0)
        return s_timeProvider->GetTime();

    if (HighResolutionClock::s_defaultToHighResolution)
        return HighResolutionClock::GetTime();

    return HighResolutionClock::GetLowResolutionTime();
}

}

bool g_systemClockCorrectionEnabled = true;

SystemClock::~SystemClock()
{
}

void SystemClock::ApplyCorrection(Time reference, Time corrected)
{
    s_correction = static_cast<int64_t>(corrected - reference);
}

Time SystemClock::GetTimeImplCorrectless()
{
    if (s_initialized)
        return Time(static_cast<unsigned long long>(GetRawTime()) + s_correction);

    s_initialized = true;
    ApplyCorrection(GetRawTime(), Time(1));
    return Time(1);
}

Time SystemClock::GetTimeImpl(bool allowRetry)
{
    static Time s_lastTime;

    Time lastTime = s_lastTime;
    long long correction = s_correction;
    Time raw = GetRawTime();

    if (!s_initialized)
    {
        s_initialized = true;
        s_lastTime = Time(1);
        ApplyCorrection(raw, Time(1));
        return Time(1);
    }

    if (correction >= 0 || raw >= static_cast<unsigned long long>(-correction))
    {
        Time now(static_cast<unsigned long long>(raw) + correction);

        if (now >= lastTime)
        {
            s_lastTime = now;
            return now;
        }
    }

    if (allowRetry)
    {
        Time corrected(lastTime + 1);

        ApplyCorrection(raw, corrected);
        s_lastTime = corrected;
        return corrected;
    }

    return ProtectedGetTime();
}

void SystemClock::RegisterTimeProvider(TimeProvider* provider, bool applyCorrection)
{
    ScopedCS lock(&s_criticalSection);

    Time sample = g_systemClockCorrectionEnabled ? GetTimeImpl(false) : GetTimeImplCorrectless();

    g_lastTime = sample;
    s_timeProvider = provider;
    g_systemClockCorrectionEnabled = provider != 0 ? applyCorrection : true;

    ApplyCorrection(GetRawTime(), sample);

    g_lastTime = g_systemClockCorrectionEnabled ? GetTimeImpl(false) : GetTimeImplCorrectless();
}

Time SystemClock::ProtectedGetTime()
{
    ScopedCS lock(&s_criticalSection);

    if (g_systemClockCorrectionEnabled)
        return GetTimeImpl(true);

    return GetTimeImplCorrectless();
}

}
}
