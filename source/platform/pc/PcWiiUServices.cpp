#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

#include <stdlib.h>


unsigned char s_altMainStopRequested = 0;

void altMain()
{
}

void altMainStop()
{
    s_altMainStopRequested = 0;
}

extern "C" void ProcUIShutdown()
{
}

extern "C" void SYSLaunchMenu()
{
    exit(0);
}
