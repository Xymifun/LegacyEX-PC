#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCLOCK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HIGHRESOLUTIONCLOCK_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class HighResolutionClock : public RootObject
{
public:
    HighResolutionClock();
    virtual ~HighResolutionClock();

    static uint64_t GetTickCount();
    static uint64_t GetFrequency();
    static Time GetTime();
    static Time GetLowResolutionTime();
    static uint64_t GetHighResolutionTime();
    static void DefaultToHighResolution();

    static bool s_defaultToHighResolution;
};

}
}

#endif
