#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcTextKeyboard.hpp"

#include <nn/erreula/erreula_cpp.h>
#include <nn/swkbd/swkbd_cpp.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <stdio.h>
#include <string.h>
#include <vpad/input.h>

namespace
{
nn::swkbd::State s_keyboardState = nn::swkbd::State::Hidden;
PcTextEntryResult s_keyboardResult = ePcTextEntry_Pending;
nn::erreula::State s_errorViewerState = nn::erreula::State::Hidden;
int32_t s_errorViewerCode = 0;
char16_t s_emptyText[1] = {0};

void PrintUtf16(const char16_t* text)
{
    if (text == 0)
        return;
    for (; *text != 0; ++text)
        putchar(*text < 0x80 ? static_cast<char>(*text) : '?');
}
}

void VPADInit()
{
}

int32_t VPADRead(VPADChan, VPADStatus* buffers, uint32_t count, VPADReadError* outError)
{
    if (PcInputActive() && buffers != 0 && count > 0)
    {
        PcInputFillGamePad(buffers);
        if (outError != 0)
            *outError = VPAD_READ_SUCCESS;
        return 1;
    }
    if (outError != 0)
        *outError = VPAD_READ_NO_SAMPLES;
    return 0;
}

void VPADGetTPCalibratedPoint(VPADChan, VPADTouchData* calibratedData, const VPADTouchData* uncalibratedData)
{
    *calibratedData = *uncalibratedData;
}

void KPADInit()
{
}

extern "C" void KPADReset()
{
}

uint32_t KPADReadEx(KPADChan channel, KPADStatus* buffers, uint32_t count, KPADError* error)
{
    KPADStatus status;
    bool connected = PcInputActive() && PcInputFillProController(static_cast<int>(channel), &status);
    if (buffers != 0 && count > 0)
        buffers[0] = status;
    if (error != 0)
        *error = connected ? KPAD_ERROR_OK : KPAD_ERROR_NO_SAMPLES;
    return connected && buffers != 0 && count > 0 ? 1 : 0;
}

void KPADEnableMpls(KPADChan, KPADMplsMode)
{
}

KPADConnectCallback KPADSetConnectCallback(KPADChan channel, KPADConnectCallback callback)
{
    return PcInputSetConnectCallback(static_cast<int>(channel), callback);
}

WPADError WPADControlDpd(WPADChan, WPADDpdFormat, WPADCallback)
{
    return WPAD_ERROR_NONE;
}

void WPADEnableURCC(BOOL)
{
}

namespace nn
{
namespace swkbd
{
uint32_t GetWorkMemorySize(uint32_t)
{
    return 16;
}

bool Create(const CreateArg&)
{
    return true;
}

bool AppearInputForm(const AppearArg& arg)
{
    int maxLength = arg.inputFormArg.maxTextLength;
    PcTextEntryBegin(arg.inputFormArg.initialText, maxLength > 0 ? static_cast<unsigned int>(maxLength) : 64u);
    PcTextKeyboardBegin();
    s_keyboardResult = ePcTextEntry_Pending;
    s_keyboardState = State::Visible;
    return true;
}

bool DisappearInputForm()
{
    PcTextEntryEnd();
    s_keyboardState = State::Hidden;
    return true;
}

void Calc(const ControllerInfo& info)
{
    uint32_t hold = 0;
    uint32_t trigger = 0;
    float stickX = 0.0f;
    float stickY = 0.0f;
    if (info.vpad != 0)
    {
        hold = info.vpad->hold;
        trigger = info.vpad->trigger;
        stickX = info.vpad->leftStick.x;
        stickY = info.vpad->leftStick.y;
    }
    static const uint32_t kProToGamePad[][2] = {
        {WPAD_PRO_BUTTON_A, VPAD_BUTTON_A},       {WPAD_PRO_BUTTON_B, VPAD_BUTTON_B},
        {WPAD_PRO_BUTTON_X, VPAD_BUTTON_X},       {WPAD_PRO_BUTTON_Y, VPAD_BUTTON_Y},
        {WPAD_PRO_BUTTON_L, VPAD_BUTTON_L},       {WPAD_PRO_BUTTON_R, VPAD_BUTTON_R},
        {WPAD_PRO_BUTTON_ZL, VPAD_BUTTON_ZL},     {WPAD_PRO_BUTTON_ZR, VPAD_BUTTON_ZR},
        {WPAD_PRO_BUTTON_PLUS, VPAD_BUTTON_PLUS}, {WPAD_PRO_BUTTON_MINUS, VPAD_BUTTON_MINUS},
        {WPAD_PRO_BUTTON_UP, VPAD_BUTTON_UP},     {WPAD_PRO_BUTTON_DOWN, VPAD_BUTTON_DOWN},
        {WPAD_PRO_BUTTON_LEFT, VPAD_BUTTON_LEFT}, {WPAD_PRO_BUTTON_RIGHT, VPAD_BUTTON_RIGHT}};
    for (int index = 0; index < 4; ++index)
    {
        const KPADStatus* pad = info.kpad[index];
        if (pad == 0 || pad->extensionType != WPAD_EXT_PRO_CONTROLLER)
            continue;
        for (unsigned int button = 0; button < sizeof(kProToGamePad) / sizeof(kProToGamePad[0]); ++button)
        {
            if ((pad->pro.hold & kProToGamePad[button][0]) != 0)
                hold |= kProToGamePad[button][1];
            if ((pad->pro.trigger & kProToGamePad[button][0]) != 0)
                trigger |= kProToGamePad[button][1];
        }
        if (pad->pro.leftStick.x * pad->pro.leftStick.x + pad->pro.leftStick.y * pad->pro.leftStick.y
            > stickX * stickX + stickY * stickY)
        {
            stickX = pad->pro.leftStick.x;
            stickY = pad->pro.leftStick.y;
        }
    }
    s_keyboardResult = PcTextKeyboardUpdate(hold, trigger, stickX, stickY);
}

void CalcSubThreadFont()
{
}

void CalcSubThreadPredict()
{
}

bool IsNeedCalcSubThreadFont()
{
    return false;
}

bool IsNeedCalcSubThreadPredict()
{
    return false;
}

bool IsDecideOkButton(bool*)
{
    return s_keyboardState == State::Visible && s_keyboardResult == ePcTextEntry_Accepted;
}

bool IsDecideCancelButton(bool*)
{
    return s_keyboardState == State::Visible && s_keyboardResult == ePcTextEntry_Cancelled;
}

const char16_t* GetInputFormString()
{
    return PcTextEntryActive() ? PcTextEntryText() : s_emptyText;
}

State GetStateKeyboard()
{
    return s_keyboardState;
}
}

namespace erreula
{
uint32_t GetWorkMemorySize()
{
    return 16;
}

bool Create(const CreateArg&)
{
    return true;
}

void AppearErrorViewer(const AppearArg& arg)
{
    s_errorViewerCode = arg.errorArg.errorCode;
    printf("[pc] error viewer: code %d ", static_cast<int>(arg.errorArg.errorCode));
    PrintUtf16(arg.errorArg.errorTitle);
    putchar(' ');
    PrintUtf16(arg.errorArg.errorMessage);
    putchar('\n');
    fflush(stdout);
    s_errorViewerState = State::Visible;
}

void DisappearErrorViewer()
{
    s_errorViewerState = State::Hidden;
}

void Calc(const ControllerInfo&)
{
}

bool IsDecideSelectButtonError()
{
    return s_errorViewerState == State::Visible;
}

State GetStateErrorViewer()
{
    return s_errorViewerState;
}

ResultType GetResultType()
{
    return ResultType::Exited;
}

int32_t GetResultCode()
{
    return s_errorViewerCode;
}

bool Jump(const char*, uint32_t)
{
    return false;
}
}
}
