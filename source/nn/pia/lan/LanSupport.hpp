#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSUPPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSUPPORT_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

Result Initialize();
Result BeginSetup();
Result EndSetup();
void Finalize();
unsigned char IsInitialized();
unsigned char IsInSetupMode();
void EmbedModuleInfo();

}
}
}

#endif
