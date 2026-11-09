#include "platform/pc/PcDebug.hpp"

namespace
{
bool debugOn = false;
bool freeCraftingOn = false;
}

void PcSetDebugMode(bool debug)
{
    debugOn = debug;
}

bool PcDebugMode()
{
    return debugOn;
}

void PcSetDebugFreeCrafting(bool freeCrafting)
{
    freeCraftingOn = freeCrafting;
    if (freeCrafting)
        debugOn = true;
}

bool PcDebugFreeCrafting()
{
    return freeCraftingOn;
}
