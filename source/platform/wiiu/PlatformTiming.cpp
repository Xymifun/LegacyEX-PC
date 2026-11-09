#include "platform/wiiu/PlatformTiming.hpp"

#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

unsigned int QueryPerformanceCounter(_LARGE_INTEGER* value)
{
    value->QuadPart = OSTicksToMicroseconds(OSGetTime());
    return 1;
}

unsigned int QueryPerformanceFrequency(_LARGE_INTEGER* value)
{
    value->QuadPart = 1000000;
    return 0;
}
