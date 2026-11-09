#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCLOCK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCLOCK_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class TimeProvider;

extern bool g_systemClockCorrectionEnabled;

class SystemClock : public RootObject
{
public:
    SystemClock();
    virtual ~SystemClock();

    static void ApplyCorrection(Time reference, Time corrected);
    static Time GetTimeImplCorrectless();
    static Time GetTimeImpl(bool allowRetry);
    static Time ProtectedGetTime();
    static void RegisterTimeProvider(TimeProvider* provider, bool applyCorrection);
    static void Reset();
};

}
}

#endif
