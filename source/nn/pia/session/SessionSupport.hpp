#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_SUPPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_SUPPORT_HPP

#include "nn/pia.h"

namespace nn
{
namespace pia
{
namespace session
{

unsigned char IsInitialized();
unsigned char IsDuringSetup();
void EmbedModuleInfo();
Result Initialize();
void Finalize();
Result BeginSetup();
Result EndSetup();

}
}
}

#endif
