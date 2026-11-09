#include "nn/pia/inet/Api.hpp"

#include <nn/middleware.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "nn/pia/Transport.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
unsigned char g_inetInitialized = 0;
unsigned char g_inetInSetup = 0;
}

Result Initialize()
{
    EmbedModuleInfo();

    if (g_inetInitialized != 0)
        return ResultAlreadyInitialized();

    pead::SafeStringBase<char> heapName("pia inet heap");
    common::HeapManager::Setup(cModuleType_Inet, 0, heapName);
    g_inetInitialized = 1;
    ModuleStackChecker::Register(cModuleType_Inet);
    return 0;
}

void Finalize()
{
    if (g_inetInitialized == 0)
        return;

    common::HeapManager::Cleanup(cModuleType_Inet);
    g_inetInitialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Inet);
}

Result BeginSetup()
{
    if (g_inetInitialized == 0)
        return ResultNotInitialized();
    if (g_inetInSetup != 0)
        return ResultInvalidState();

    common::HeapManager::SetCurrentHeap(cModuleType_Inet);
    g_inetInSetup = 1;
    return 0;
}

Result EndSetup()
{
    if (g_inetInitialized == 0)
        return transport::cResultNotInitialized;
    if (g_inetInSetup == 0)
        return transport::cResultInvalidState;

    common::HeapManager::GetHeap(cModuleType_Inet)->adjust();
    g_inetInSetup = 0;
    common::HeapManager::ClearCurrentHeap();
    return 0;
}

bool IsBehindNat(const transport::StationLocation& location)
{
    return (location.m_field25 & 1) != 0;
}

bool IsPublic(const transport::StationLocation& location)
{
    return ((location.m_field25 >> 1) & 1) != 0;
}

unsigned char IsGlobal(const transport::StationLocation& location)
{
    unsigned int global = 0;
    if (IsPublic(location) && !IsBehindNat(location))
        global = 1;
    return global;
}

bool IsEdmMapping(const transport::StationLocation& location)
{
    return location.m_field23 == 2;
}

bool IsEimMapping(const transport::StationLocation& location)
{
    return location.m_field23 == 1;
}

namespace
{
DebugSetting g_debugSetting;
}

unsigned char IsInitialized()
{
    return g_inetInitialized;
}

unsigned char IsInSetupMode()
{
    return g_inetInSetup;
}

DebugSetting* GetDebugSetting()
{
    return &g_debugSetting;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_INET_5_9_0]");
}

}
}
}
