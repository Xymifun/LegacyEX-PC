#include <nn/swkbd.h>

#include <cafe/mem.h>
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <nn/middleware.h>

namespace nn
{
namespace swkbd
{

namespace
{

const uint32_t sWorkMemorySize = 0x1980000;
const uint32_t sWorkMemorySizeReductionFlag0 = 0xA0000;
const uint32_t sWorkMemorySizeReductionFlag1 = 0x120000;
const uint32_t sRplAcquireBufferSize = 0x380000;
const int sSwkbdVersion = 3;

bool (*sAppearInputForm)(const AppearArg&);
bool (*sAppearKeyboard)(const KeyboardArg&);
void (*sCalcSubThreadFont)();
void (*sCalcSubThreadPredict)();
void (*sCalc)(const ControllerInfo&);
void (*sConfirmUnfixAll)();
void (*sCreate)(unsigned char*, RegionType, unsigned int, FSClient*);
void (*sDestroy)();
bool (*sDisappearInputForm)();
bool (*sDisappearKeyboard)();
void (*sDrawDRC)();
void (*sDrawTV)();
void (*sGetDrawStringInfo)(DrawStringInfo*);
const char16_t* (*sGetInputFormString)();
void (*sGetKeyboardCondition)(KeyboardCondition*);
bool (*sInitLearnDic)(void*);
State (*sGetStateInputForm)();
State (*sGetStateKeyboard)();
bool (*sIsCoveredWithSubWindow)();
bool (*sIsDecideCancelButton)(bool*);
bool (*sIsDecideOkButton)(bool*);
bool (*sIsKeyboardTarget)(IEventReceiver*);
bool (*sIsNeedCalcSubThreadFont)();
bool (*sIsNeedCalcSubThreadPredict)();
bool (*sIsSelectCursorActive)();
void (*sInactivateSelectCursor)();
void (*sSetControllerRemo)(ControllerType);
void (*sSetCursorPos)(int);
void (*sSetEnableOkButton)(bool);
void (*sSetReceiver)(const ReceiverArg&);
void (*sSetSelectFrom)(int);
void (*sSetUserControllerEventObj)(IControllerEventObj*);
void (*sSetUserSoundObj)(ISoundObj*);
void (*sSetInputFormString)(const char16_t*);
void (*sMuteAllSound)(bool);
void (*sSetVersion)(int);

MEMHeapHandle sHeapHandle;
OSDynLoad_Module sRplHandle;

}

template <typename T>
void findExport(T* function, const char* name);

int allocForDynLoad(int size, int align, void** ptr)
{
    *ptr = MEMAllocFromExpHeapEx(sHeapHandle, size, align);
    if (*ptr != NULL)
        return 0;
    return -1;
}

void freeForDynLoad(void* ptr)
{
    MEMFreeToExpHeap(sHeapHandle, ptr);
}

uint32_t GetWorkMemorySize(uint32_t unk)
{
    uint32_t reduction = 0;
    if (unk & 1)
        reduction = sWorkMemorySizeReductionFlag0;
    if (unk & 2)
        reduction += sWorkMemorySizeReductionFlag1;
    return sWorkMemorySize - reduction;
}

bool Create(const CreateArg& args)
{
    if (args.workMemory == NULL)
    {
        OSReport("SWKBD: Create failed. CreateArg.mBuffer is NULL.");
        return false;
    }
    if (args.fsClient == NULL)
    {
        OSReport("SWKBD: Create failed. CreateArg.mFSClient is NULL.");
        return false;
    }

    NN_DEFINE_MIDDLEWARE(moduleInfo, "Nintendo", "SWKBD_2_4_1");
    NN_USING_MIDDLEWARE(moduleInfo);

    sHeapHandle = MEMCreateExpHeap(args.workMemory, GetWorkMemorySize(args.unk_0x08));
    OSDynLoad_SetAllocator(reinterpret_cast<OSDynLoadAllocFn>(allocForDynLoad), freeForDynLoad);

    int error = OSDynLoad_Acquire("swkbd.rpl", &sRplHandle);
    if (error != 0)
    {
        OSReport("SWKBD: Create failed. OSDynLoad_Acquire() return error(%d).\n", error);
        return false;
    }

    uint32_t dynloadAcquireUseSize = GetWorkMemorySize(args.unk_0x08) - MEMGetAllocatableSizeForExpHeap(sHeapHandle);
    OSReport("SWKBD: OSDynLoad_Acquire() use [%d/%d]\n", dynloadAcquireUseSize, sRplAcquireBufferSize);
    if (dynloadAcquireUseSize > sRplAcquireBufferSize)
    {
        OSReport("SWKBD: Create failed. dynload_acquire_use_size > sRplAcquireBufferSize.");
        return false;
    }

    findExport(&sAppearInputForm, "SwkbdAppearInputForm__3RplFRCQ3_2nn5swkbd9AppearArg");
    findExport(&sAppearKeyboard, "SwkbdAppearKeyboard__3RplFRCQ3_2nn5swkbd11KeyboardArg");
    findExport(&sCalcSubThreadFont, "SwkbdCalcSubThreadFont__3RplFv");
    findExport(&sCalcSubThreadPredict, "SwkbdCalcSubThreadPredict__3RplFv");
    findExport(&sCalc, "SwkbdCalc__3RplFRCQ3_2nn5swkbd14ControllerInfo");
    findExport(&sConfirmUnfixAll, "SwkbdConfirmUnfixAll__3RplFv");
    findExport(&sCreate, "SwkbdCreate__3RplFPUcQ3_2nn5swkbd10RegionTypeUiP8FSClient");
    findExport(&sDestroy, "SwkbdDestroy__3RplFv");
    findExport(&sDisappearInputForm, "SwkbdDisappearInputForm__3RplFv");
    findExport(&sDisappearKeyboard, "SwkbdDisappearKeyboard__3RplFv");
    findExport(&sDrawDRC, "SwkbdDrawDRC__3RplFv");
    findExport(&sDrawTV, "SwkbdDrawTV__3RplFv");
    findExport(&sGetDrawStringInfo, "SwkbdGetDrawStringInfo__3RplFPQ3_2nn5swkbd14DrawStringInfo");
    findExport(&sGetInputFormString, "SwkbdGetInputFormString__3RplFv");
    findExport(&sGetKeyboardCondition, "SwkbdGetKeyboardCondition__3RplFPQ3_2nn5swkbd17KeyboardCondition");
    findExport(&sInitLearnDic, "SwkbdInitLearnDic__3RplFPv");
    findExport(&sGetStateInputForm, "SwkbdGetStateInputForm__3RplFv");
    findExport(&sGetStateKeyboard, "SwkbdGetStateKeyboard__3RplFv");
    findExport(&sIsCoveredWithSubWindow, "SwkbdIsCoveredWithSubWindow__3RplFv");
    findExport(&sIsDecideCancelButton, "SwkbdIsDecideCancelButton__3RplFPb");
    findExport(&sIsDecideOkButton, "SwkbdIsDecideOkButton__3RplFPb");
    findExport(&sIsKeyboardTarget, "SwkbdIsKeyboardTarget__3RplFPQ3_2nn5swkbd14IEventReceiver");
    findExport(&sIsNeedCalcSubThreadFont, "SwkbdIsNeedCalcSubThreadFont__3RplFv");
    findExport(&sIsNeedCalcSubThreadPredict, "SwkbdIsNeedCalcSubThreadPredict__3RplFv");
    findExport(&sIsSelectCursorActive, "SwkbdIsSelectCursorActive__3RplFv");
    findExport(&sInactivateSelectCursor, "SwkbdInactivateSelectCursor__3RplFv");
    findExport(&sSetControllerRemo, "SwkbdSetControllerRemo__3RplFQ3_2nn5swkbd14ControllerType");
    findExport(&sSetCursorPos, "SwkbdSetCursorPos__3RplFi");
    findExport(&sSetEnableOkButton, "SwkbdSetEnableOkButton__3RplFb");
    findExport(&sSetReceiver, "SwkbdSetReceiver__3RplFRCQ3_2nn5swkbd11ReceiverArg");
    findExport(&sSetSelectFrom, "SwkbdSetSelectFrom__3RplFi");
    findExport(&sSetUserControllerEventObj, "SwkbdSetUserControllerEventObj__3RplFPQ3_2nn5swkbd19IControllerEventObj");
    findExport(&sSetUserSoundObj, "SwkbdSetUserSoundObj__3RplFPQ3_2nn5swkbd9ISoundObj");
    findExport(&sSetInputFormString, "SwkbdSetInputFormString__3RplFPCw");
    findExport(&sMuteAllSound, "SwkbdMuteAllSound__3RplFb");
    findExport(&sSetVersion, "SwkbdSetVersion__3RplFi");

    unsigned char* frameworkBuffer = static_cast<unsigned char*>(
        MEMAllocFromExpHeap(sHeapHandle, GetWorkMemorySize(args.unk_0x08) - sRplAcquireBufferSize));
    if (frameworkBuffer == NULL)
    {
        OSReport("SWKBD: Create failed. framework_buffer == NULL");
        return false;
    }

    sSetVersion(sSwkbdVersion);
    sCreate(frameworkBuffer, args.regionType, args.unk_0x08, args.fsClient);
    return true;
}

State GetStateInputForm()
{
    return sGetStateInputForm();
}

State GetStateKeyboard()
{
    return sGetStateKeyboard();
}

void Calc(const ControllerInfo& controllerInfo)
{
    if (GetStateInputForm() != State() || GetStateKeyboard() != State())
        sCalc(controllerInfo);
}

bool IsNeedCalcSubThreadFont()
{
    return sIsNeedCalcSubThreadFont();
}

void CalcSubThreadFont()
{
    sCalcSubThreadFont();
}

bool IsNeedCalcSubThreadPredict()
{
    return sIsNeedCalcSubThreadPredict();
}

void CalcSubThreadPredict()
{
    sCalcSubThreadPredict();
}

void DrawTV()
{
    if (GetStateInputForm() != State() || GetStateKeyboard() != State())
        sDrawTV();
}

void DrawDRC()
{
    if (GetStateInputForm() != State() || GetStateKeyboard() != State())
        sDrawDRC();
}

bool AppearInputForm(const AppearArg& args)
{
    return sAppearInputForm(args);
}

bool DisappearInputForm()
{
    return sDisappearInputForm();
}

const char16_t* GetInputFormString()
{
    return sGetInputFormString();
}

bool IsDecideCancelButton(bool* outIsSelected)
{
    return sIsDecideCancelButton(outIsSelected);
}

bool IsDecideOkButton(bool* outIsSelected)
{
    return sIsDecideOkButton(outIsSelected);
}

template <typename T>
void findExport(T* function, const char* name)
{
    OSDynLoad_FindExport(sRplHandle, OS_DYNLOAD_EXPORT_FUNC, name, reinterpret_cast<void**>(function));
}

}
}
