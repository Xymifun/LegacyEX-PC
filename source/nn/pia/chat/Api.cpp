#include "nn/pia/ChatSupport.hpp"
#include "nn/pia/chat/CodecFacade.hpp"

namespace nn
{
namespace pia
{
namespace chat
{

namespace
{

unsigned char g_chatInitialized = 0;
unsigned char g_chatInSetup = 0;

void createSystemSingletons()
{
    CodecFacade::CreateInstance();
    g_codecFacade->Initialize();
}

void destroySystemSingletons()
{
    if (g_codecFacade != 0)
    {
        g_codecFacade->Finalize();
        CodecFacade::DestroyInstance();
    }
}

}

unsigned int ConvertID_Pia2VCT(unsigned int stationIndex)
{
    return stationIndex + 1;
}

unsigned int ConvertID_VCT2Pia(unsigned int connectionId)
{
    return connectionId - 1;
}

Result Initialize()
{
    EmbedModuleInfo();
    if (!common::IsInitialized())
        return ResultInvalidState();
    if (g_chatInitialized != 0)
        return ResultAlreadyInitialized();

    pead::SafeStringBase<char> heapName("pia chat heap");
    const ModuleType chatModule = cModuleType_Chat;
    common::HeapManager::Setup(chatModule, 0, heapName);
    g_chatInitialized = 1;
    ModuleStackChecker::Register(chatModule);
    return 0;
}

Result BeginSetup()
{
    if (g_chatInitialized == 0)
        return ResultNotInitialized();
    if (g_chatInSetup != 0)
        return ResultInvalidState();

    common::HeapManager::SetCurrentHeap(cModuleType_Chat);
    g_chatInSetup = 1;
    createSystemSingletons();
    return 0;
}

Result EndSetup()
{
    if (g_chatInitialized == 0)
        return 0x10c0c;
    if (g_chatInSetup == 0)
        return 0x10c08;

    common::HeapManager::GetHeap()->adjust();
    common::HeapManager::ClearCurrentHeap();
    g_chatInSetup = 0;
    return 0;
}

void Finalize()
{
    if (g_chatInitialized == 0)
        return;
    if (g_chatInSetup != 0)
        EndSetup();

    destroySystemSingletons();
    common::HeapManager::Cleanup(cModuleType_Chat);
    g_chatInitialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Chat);
}

unsigned char IsInitialized()
{
    return g_chatInitialized;
}

unsigned char IsInSetupMode()
{
    return g_chatInSetup;
}

}
}
}
