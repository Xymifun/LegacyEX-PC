#include "nn/pia/ModuleStackChecker.hpp"

namespace nn
{
namespace pia
{


namespace
{
int g_stackTop = 0;
ModuleType g_stack[24];
}

bool ModuleStackChecker::isInvalid(ModuleType moduleType)
{
    if (moduleType < 2 || moduleType > 22)
        return true;
    return false;
}

int ModuleStackChecker::getOrder(ModuleType moduleType)
{
    switch (moduleType)
    {
    case cModuleType_Common:
        return 0;
    case 3:
        return 2;
    case cModuleType_Transport:
        return 3;
    case 5:
        return 4;
    case cModuleType_Inet:
        return 2;
    case cModuleType_Session:
        return 5;
    case 15:
        return 1;
    case cModuleType_Lan:
        return 2;
    case 17:
        return 6;
    case 18:
        return 2;
    case 19:
        return 2;
    case 22:
        return 2;
    case cModuleType_Chat:
    case 12:
    case 13:
        return 4;
    default:
        return -1;
    }
}

ModuleType ModuleStackChecker::getTop()
{
    return g_stack[g_stackTop];
}

Result ModuleStackChecker::Register(ModuleType moduleType)
{
    if (isInvalid(moduleType))
        return 0x10c07;
    if (g_stackTop >= 23)
        return 0x10c08;
    if (g_stackTop == 0)
    {
        if (moduleType != 2)
            return 0x10c08;
    }
    else if (getOrder(getTop()) > getOrder(moduleType))
    {
        return 0x10c08;
    }
    ++g_stackTop;
    g_stack[g_stackTop] = moduleType;
    return 0;
}

Result ModuleStackChecker::Unregister(ModuleType moduleType)
{
    if (isInvalid(moduleType))
        return 0x10c07;
    if (g_stackTop <= 0)
        return 0x10c08;
    if (getTop() != moduleType)
        return 0x10c08;
    --g_stackTop;
    return 0;
}

}
}
