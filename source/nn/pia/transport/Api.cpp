#include "nn/pia/transport/TransportSupport.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "nn/pia/Transport.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

unsigned char g_piaTransportInitialized = 0;
unsigned char g_piaTransportInSetupMode = 0;

Result Initialize()
{
    EmbedModuleInfo();

    if (g_piaTransportInitialized != 0)
        return ResultAlreadyInitialized();

    pead::SafeStringBase<char> heapName("pia transport heap");
    common::HeapManager::Setup(cModuleType_Transport, 0, heapName);
    g_piaTransportInitialized = 1;
    ModuleStackChecker::Register(cModuleType_Transport);
    return 0;
}

Result EndSetup()
{
    if (g_piaTransportInitialized == 0)
        return cResultNotInitialized;
    if (g_piaTransportInSetupMode == 0)
        return cResultInvalidState;

    common::HeapManager::GetHeap(cModuleType_Transport)->adjust();
    common::HeapManager::ClearCurrentHeap();
    g_piaTransportInSetupMode = 0;
    return 0;
}

void Finalize()
{
    if (g_piaTransportInitialized == 0)
        return;

    if (g_piaTransportInSetupMode != 0)
        EndSetup();
    if (g_piaTransport != 0)
        Transport::DestroyInstance();
    common::HeapManager::Cleanup(cModuleType_Transport);
    g_piaTransportInitialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Transport);
}

Result BeginSetup()
{
    if (g_piaTransportInitialized == 0)
        return ResultNotInitialized();
    if (g_piaTransportInSetupMode != 0)
        return ResultInvalidState();

    common::HeapManager::SetCurrentHeap(cModuleType_Transport);
    g_piaTransportInSetupMode = 1;
    return 0;
}

unsigned char IsInitialized()
{
    return g_piaTransportInitialized;
}

unsigned char IsInSetupMode()
{
    return g_piaTransportInSetupMode;
}

}
}
}
