#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_SUPPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_SUPPORT_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{
unsigned char IsInitialized();
void EmbedModuleInfo();
}

namespace chat
{

unsigned char IsInitialized();
unsigned char IsInSetupMode();
void EmbedModuleInfo();

}
}
}

#endif
