#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_MODULESTACKCHECKER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_MODULESTACKCHECKER_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{


enum ModuleType
{
    cModuleType_Invalid = 0,
    cModuleType_Common = 2,
    cModuleType_Transport = 4,
    cModuleType_Inet = 6,
    cModuleType_Session = 7,
    cModuleType_Chat = 11,
    cModuleType_Lan = 16
};

class ModuleStackChecker
{
public:
    static bool isInvalid(ModuleType moduleType);
    static bool isInclude(ModuleType moduleType);
    static int getOrder(ModuleType moduleType);
    static ModuleType getTop();
    static Result Register(ModuleType moduleType);
    static Result Unregister(ModuleType moduleType);
};

}
}

#endif
