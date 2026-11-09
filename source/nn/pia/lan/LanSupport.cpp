#include "nn/pia/lan/LanSupport.hpp"

#include <nn/middleware.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "platform/wiiu/pead.hpp"

namespace
{
unsigned char g_lanInitialized = 0;
unsigned char g_lanInSetup = 0;

enum
{
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c
};
}

namespace nn
{
namespace pia
{
namespace lan
{

Result Initialize()
{
    EmbedModuleInfo();

    if (g_lanInitialized != 0)
        return ResultAlreadyInitialized();

    pead::SafeStringBase<char> heapName("pia lan heap");
    common::HeapManager::Setup(cModuleType_Lan, 0, heapName);
    g_lanInitialized = 1;
    Result result = ModuleStackChecker::Register(cModuleType_Lan);
    if (result != 0)
        return result;
    return 0;
}

Result BeginSetup()
{
    if (g_lanInitialized == 0)
        return ResultNotInitialized();
    if (g_lanInSetup != 0)
        return ResultInvalidState();

    common::HeapManager::SetCurrentHeap(cModuleType_Lan);
    g_lanInSetup = 1;
    return 0;
}

Result EndSetup()
{
    if (g_lanInitialized == 0)
        return cResultNotInitialized;
    if (g_lanInSetup == 0)
        return cResultInvalidState;

    pead::Heap* heap = common::HeapManager::GetHeap(cModuleType_Lan);
    if (heap == 0)
        return cResultInvalidState;

    heap->adjust();
    g_lanInSetup = 0;
    common::HeapManager::ClearCurrentHeap();
    return 0;
}

void Finalize()
{
    if (g_lanInitialized == 0)
        return;

    common::HeapManager::Cleanup(cModuleType_Lan);
    g_lanInitialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Lan);
}

unsigned char IsInitialized()
{
    return g_lanInitialized;
}

unsigned char IsInSetupMode()
{
    return g_lanInSetup;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_LAN_5_9_0]");
}

}
}
}
