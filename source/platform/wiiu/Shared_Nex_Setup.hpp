#ifndef SOURCE_PLATFORM_WIIU_SHARED_NEX_SETUP_HPP
#define SOURCE_PLATFORM_WIIU_SHARED_NEX_SETUP_HPP

#include "nn/nex/Core.hpp"

namespace nn
{
namespace nex
{
class NgsFacade;
}
}

unsigned int Nex_InitializeNex(unsigned int runtimeMemorySize, unsigned int reservedMemorySize,
                               nn::nex::Core::ThreadMode threadMode);
void Nex_FinalizeNex();
nn::nex::NgsFacade* Nex_GetNgsFacadeInstance();
unsigned int Nex_GetInternalErrorCode();
unsigned int Nex_GetLastAPIErrorCode();
unsigned int Nex_GetPeakMemoryUsage();

#endif
