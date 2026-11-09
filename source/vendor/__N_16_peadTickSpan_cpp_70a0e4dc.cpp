#include "vendor/__N_16_peadTickSpan_cpp_70a0e4dc.hpp"

#include "platform/wiiu/pead.hpp"

#include <coreinit/systeminfo.h>

int64_t __N_16_peadTickSpan_cpp_70a0e4dc::getFrequency_()
{
    return (int64_t)(OSGetSystemInfo()->busClockSpeed / 4);
}

const long long pead::TickSpan::cFrequency = __N_16_peadTickSpan_cpp_70a0e4dc().getFrequency_();
