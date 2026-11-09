#include "nn/pia/session/SessionSupport.hpp"

#include <nn/middleware.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
unsigned char s_initialized = 0;
unsigned char s_duringSetup = 0;
}

unsigned char IsInitialized()
{
    return s_initialized;
}

unsigned char IsDuringSetup()
{
    return s_duringSetup;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_SESSION_5_9_0]");
}

Result Initialize()
{
    EmbedModuleInfo();

    if (s_initialized != 0)
        return ResultAlreadyInitialized();

    pead::SafeStringBase<char> heapName("pia session heap");
    common::HeapManager::Setup(cModuleType_Session, 0, heapName);
    s_initialized = 1;
    ModuleStackChecker::Register(cModuleType_Session);
    return 0;
}

Result BeginSetup()
{
    if (s_initialized == 0)
        return ResultNotInitialized();
    if (s_duringSetup != 0)
        return ResultInvalidState();

    common::HeapManager::SetCurrentHeap(cModuleType_Session);
    s_duringSetup = 1;
    return 0;
}

Result EndSetup()
{
    if (s_initialized == 0)
        return 0x10c0c;
    if (s_duringSetup == 0)
        return 0x10c08;

    common::HeapManager::GetHeap()->adjust();
    common::HeapManager::ClearCurrentHeap();
    s_duringSetup = 0;
    return 0;
}

void Finalize()
{
    if (s_initialized == 0)
        return;

    if (s_duringSetup != 0)
        EndSetup();

    if (g_piaMesh != 0)
    {
        g_piaMesh->Cleanup();
        Mesh::DestroyInstance();
    }

    common::HeapManager::Cleanup(cModuleType_Session);
    s_initialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Session);
}

}
}
}
