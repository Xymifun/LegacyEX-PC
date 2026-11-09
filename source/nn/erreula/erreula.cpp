#include <nn/erreula/erreula_cpp.h>

#include <nn/middleware.h>

#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/memexpheap.h>

namespace nn
{
namespace erreula
{

struct ClosureA0
{
    typedef void (*Function)();

    void operator()() const { function(); }

    Function function;
};

namespace
{

const uint32_t sWorkMemorySize = 0x1600000;
const uint32_t sRplAcquireBufferSize = 0x380000;
const uint32_t sFrameworkBufferSize = 0x1280000;
const int sVersion = 3;

void (*sAppearError)(const AppearArg& arg);
void (*sCalc)(const ControllerInfo& controllerInfo);
void (*sChangeLang)(LangType languageType);
void (*sSetControllerRemo)(ControllerType controller);
void (*sAppearHomeNixSign)(HomeNixSignArg arg);
void (*sCreate)(unsigned char* buffer, RegionType region, LangType language, FSClient* fsClient);
void (*sDestroy)();
ClosureA0 sDisappearError;
void (*sDrawDRC)();
void (*sDrawTV)();
void (*sDisappearHomeNixSign)();
State (*sGetStateErrorViewer)();
int (*sGetResultCode)();
int (*sGetSelectButtonNumError)();
ResultType (*sGetResultType)();
bool (*sIsDecideSelectButtonError)();
bool (*sIsDecideSelectLeftButtonError)();
bool (*sIsDecideSelectRightButtonError)();
bool (*sIsAppearHomeNixSign)();
bool (*sIsSelectCursorActive)();
void (*sSetVersion)(int version);
void (*sPlayAppearSE)(bool playAppearSoundEffect);
bool (*sJump)(const char* buffer, uint32_t bufferSize);
MEMHeapHandle sHeapHandle;
OSDynLoad_Module sModule;

bool sIsHeapCreated = false;

}

OSDynLoad_Error allocForDynLoad(int size, int align, void** ptr)
{
    void* block = MEMAllocFromExpHeapEx(sHeapHandle, size, align);
    *ptr = block;
    return block != 0 ? OS_DYNLOAD_OK : static_cast<OSDynLoad_Error>(-1);
}

void freeForDynLoad(void* ptr)
{
    MEMFreeToExpHeap(sHeapHandle, ptr);
}

template <typename T>
void findExport(T* function, const char* name)
{
    OSDynLoad_FindExport(sModule, OS_DYNLOAD_EXPORT_FUNC, name, reinterpret_cast<void**>(function));
}

bool Create(const CreateArg& args)
{
    if (args.workMemory == 0)
    {
        OSReport("ErrEula Create Failed. CreateArg.mBuffer is NULL.");
        return false;
    }
    if (args.fsClient == 0)
    {
        OSReport("ErrEula Create Failed. CreateArg.mFSClient is NULL.");
        return false;
    }

    NN_DEFINE_MIDDLEWARE(moduleInfo, "Nintendo", "Error_2_4_0");
    NN_USING_MIDDLEWARE(moduleInfo);

    sHeapHandle = MEMCreateExpHeapEx(args.workMemory, sWorkMemorySize, 0);
    sIsHeapCreated = true;

    OSDynLoad_SetAllocator(allocForDynLoad, freeForDynLoad);
    int error = OSDynLoad_Acquire("erreula.rpl", &sModule);
    if (error != OS_DYNLOAD_OK)
    {
        OSReport("ErrEula Create Failed. OSDynLoad_Acquire() return error(%d).\n", error);
        return false;
    }

    if (sIsHeapCreated)
    {
        uint32_t useSize = sWorkMemorySize - MEMGetAllocatableSizeForExpHeapEx(sHeapHandle, 4);
        OSReport("OSDynLoad_Acquire() use [%d]\n", useSize);
        if (useSize > sRplAcquireBufferSize)
        {
            OSReport("ErrEula Create Failed. dynload_acquire_use_size > sRplAcquireBufferSize");
            return false;
        }
    }

    findExport(&sAppearError, "ErrEulaAppearError__3RplFRCQ3_2nn7erreula9AppearArg");
    findExport(&sCalc, "ErrEulaCalc__3RplFRCQ3_2nn7erreula14ControllerInfo");
    findExport(&sCreate, "ErrEulaCreate__3RplFPUcQ3_2nn7erreula10RegionTypeQ3_2nn7erreula8LangTypeP8FSClient");
    findExport(&sDestroy, "ErrEulaDestroy__3RplFv");
    findExport(&sDisappearError, "ErrEulaDisappearError__3RplFv");
    findExport(&sDrawDRC, "ErrEulaDrawDRC__3RplFv");
    findExport(&sDrawTV, "ErrEulaDrawTV__3RplFv");
    findExport(&sGetStateErrorViewer, "ErrEulaGetStateErrorViewer__3RplFv");
    findExport(&sIsDecideSelectButtonError, "ErrEulaIsDecideSelectButtonError__3RplFv");
    findExport(&sIsDecideSelectLeftButtonError, "ErrEulaIsDecideSelectLeftButtonError__3RplFv");
    findExport(&sIsDecideSelectRightButtonError, "ErrEulaIsDecideSelectRightButtonError__3RplFv");
    findExport(&sSetControllerRemo, "ErrEulaSetControllerRemo__3RplFQ3_2nn7erreula14ControllerType");
    findExport(&sAppearHomeNixSign, "ErrEulaAppearHomeNixSign__3RplFRCQ3_2nn7erreula14HomeNixSignArg");
    findExport(&sIsAppearHomeNixSign, "ErrEulaIsAppearHomeNixSign__3RplFv");
    findExport(&sDisappearHomeNixSign, "ErrEulaDisappearHomeNixSign__3RplFv");
    findExport(&sChangeLang, "ErrEulaChangeLang__3RplFQ3_2nn7erreula8LangType");
    findExport(&sIsSelectCursorActive, "ErrEulaIsSelectCursorActive__3RplFv");
    findExport(&sGetResultType, "ErrEulaGetResultType__3RplFv");
    findExport(&sGetResultCode, "ErrEulaGetResultCode__3RplFv");
    findExport(&sGetSelectButtonNumError, "ErrEulaGetSelectButtonNumError__3RplFv");
    findExport(&sSetVersion, "ErrEulaSetVersion__3RplFi");
    findExport(&sPlayAppearSE, "ErrEulaPlayAppearSE__3RplFb");
    findExport(&sJump, "ErrEulaJump__3RplFPCcUi");

    unsigned char* frameworkBuffer = 0;
    if (sIsHeapCreated)
    {
        frameworkBuffer = static_cast<unsigned char*>(MEMAllocFromExpHeapEx(sHeapHandle, sFrameworkBufferSize, 4));
        if (frameworkBuffer == 0)
        {
            OSReport("ErrEula Create Failed. framework_buffer == NULL");
            return false;
        }
    }

    sSetVersion(sVersion);
    sCreate(frameworkBuffer, args.region, args.language, args.fsClient);
    return true;
}

void Destroy()
{
    sDestroy();
    OSDynLoad_Release(sModule);
    if (sIsHeapCreated)
    {
        MEMDestroyExpHeap(sHeapHandle);
        sIsHeapCreated = false;
    }
}

State GetStateErrorViewer()
{
    return sGetStateErrorViewer();
}

bool IsAppearHomeNixSign()
{
    return sIsAppearHomeNixSign();
}

void Calc(const ControllerInfo& controllerInfo)
{
    if (GetStateErrorViewer() != State() || IsAppearHomeNixSign())
    {
        sCalc(controllerInfo);
    }
}

void DrawTV()
{
    if (GetStateErrorViewer() != State() || IsAppearHomeNixSign())
    {
        sDrawTV();
    }
}

void DrawDRC()
{
    if (GetStateErrorViewer() != State() || IsAppearHomeNixSign())
    {
        sDrawDRC();
    }
}

void AppearErrorViewer(const AppearArg& arg)
{
    sAppearError(arg);
}

void DisappearErrorViewer()
{
    sDisappearError();
}

bool IsDecideSelectButtonError()
{
    return sIsDecideSelectButtonError();
}

bool IsDecideSelectLeftButtonError()
{
    return sIsDecideSelectLeftButtonError();
}

bool IsDecideSelectRightButtonError()
{
    return sIsDecideSelectRightButtonError();
}

ResultType GetResultType()
{
    return sGetResultType();
}

int32_t GetResultCode()
{
    return sGetResultCode();
}

int32_t GetSelectButtonNumError()
{
    return sGetSelectButtonNumError();
}

void SetControllerRemo(ControllerType controller)
{
    sSetControllerRemo(controller);
}

void AppearHomeNixSign(const HomeNixSignArg& arg)
{
    sAppearHomeNixSign(arg);
}

void DisappearHomeNixSign()
{
    sDisappearHomeNixSign();
}

void ChangeLangError(LangType languageType)
{
    sChangeLang(languageType);
}

bool IsSelectCursorActive()
{
    return sIsSelectCursorActive();
}

void PlayAppearSE(bool playAppearSoundEffect)
{
    sPlayAppearSE(playAppearSoundEffect);
}

bool Jump(const char* buffer, uint32_t bufferSize)
{
    return sJump(buffer, bufferSize);
}

uint32_t GetWorkMemorySize()
{
    return sWorkMemorySize;
}

}
}
