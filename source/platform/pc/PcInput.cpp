#include "platform/pc/PcInput.hpp"

#include "platform/pc/PcDebugOverlay.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcKeyBindings.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>
#include <vpad/input.h>

namespace
{
const int kKeyCount = 256;
const int kKeyBackspace = 0x08;
const int kKeyTab = 0x09;
const int kKeyReturn = 0x0D;
const int kKeyShift = 0x10;
const int kKeyControl = 0x11;
const int kKeyEscape = 0x1B;
const int kKeyPageUp = 0x21;
const int kKeyPageDown = 0x22;
const int kKeyEnd = 0x23;
const int kKeyHome = 0x24;
const float kGlyphStickThreshold = 0.5f;
const int kKeySpace = 0x20;
const int kKeyLeft = 0x25;
const int kKeyUp = 0x26;
const int kKeyRight = 0x27;
const int kKeyDown = 0x28;
const int kKeyDelete = 0x2E;
const int kKeyF2 = 0x71;
const int kKeyF3 = 0x72;
const int kKeyF5 = 0x74;
const int kKeyF6 = 0x75;
const int kKeyF7 = 0x76;
const int kKeyF8 = 0x77;
const int kHotbarSlots = 9;
const float kLogicalWidth = 1280.0f;
const float kLogicalHeight = 720.0f;
const float kNavigationStick = 0.3f;
const unsigned long long kContainerLingerNanoseconds = 250000000ULL;
const float kMouseLookScale = 0.04f;
const float kMouseLookDecay = 0.5f;
const float kTriggerThreshold = 0.5f;
const int kPadSlots = 5;
const int kGamePadSlot = 0;

enum InputContext
{
    eInputContext_Game,
    eInputContext_Menu,
    eInputContext_Container,
    eInputContext_Count
};

struct KeyBinding
{
    int key;
    uint32_t buttons[eInputContext_Count];
};

const KeyBinding kKeyBindings[] = {
    {kKeySpace, {0, VPAD_BUTTON_A, 0}},
    {kKeyReturn, {0, VPAD_BUTTON_A, 0}},
    {kKeyBackspace, {0, VPAD_BUTTON_B, VPAD_BUTTON_B}},
    {kKeyEscape, {VPAD_BUTTON_PLUS, VPAD_BUTTON_B, VPAD_BUTTON_B}},
    {'E', {0, VPAD_BUTTON_X, 0}},
    {'C', {0, VPAD_BUTTON_Y, 0}},
    {kKeyTab, {0, VPAD_BUTTON_MINUS, 0}},
    {'Z', {0, VPAD_BUTTON_L, VPAD_BUTTON_L}},
    {'X', {0, VPAD_BUTTON_R, VPAD_BUTTON_R}},
    {'R', {0, VPAD_BUTTON_ZL, VPAD_BUTTON_ZL}},
    {'F', {0, VPAD_BUTTON_ZR, VPAD_BUTTON_ZR}},
    {kKeyPageUp, {0, VPAD_BUTTON_L, VPAD_BUTTON_L}},
    {kKeyPageDown, {0, VPAD_BUTTON_R, VPAD_BUTTON_R}},
    {kKeyHome, {0, VPAD_BUTTON_ZL, VPAD_BUTTON_ZL}},
    {kKeyEnd, {0, VPAD_BUTTON_ZR, VPAD_BUTTON_ZR}},
    {kKeyLeft, {0, VPAD_BUTTON_LEFT, VPAD_BUTTON_LEFT}},
    {kKeyRight, {0, VPAD_BUTTON_RIGHT, VPAD_BUTTON_RIGHT}},
    {kKeyUp, {0, VPAD_BUTTON_UP, VPAD_BUTTON_UP}},
    {kKeyDown, {0, VPAD_BUTTON_DOWN, VPAD_BUTTON_DOWN}},
};

const uint32_t kJoypadBits[] = {0x1,    0x2,    0x4,     0x8,     0x400000, 0x800000, 0x40,   0x80,
                                0x10,   0x20,   0x100,   0x200,   0x400,    0x800,    0x1000, 0x2000};
const uint32_t kJoypadVpad[] = {VPAD_BUTTON_B,       VPAD_BUTTON_A,       VPAD_BUTTON_Y,    VPAD_BUTTON_X,
                                VPAD_BUTTON_ZR,      VPAD_BUTTON_ZL,      VPAD_BUTTON_R,    VPAD_BUTTON_L,
                                VPAD_BUTTON_PLUS,    VPAD_BUTTON_MINUS,   VPAD_BUTTON_STICK_R, VPAD_BUTTON_STICK_L,
                                VPAD_BUTTON_UP,      VPAD_BUTTON_DOWN,    VPAD_BUTTON_LEFT, VPAD_BUTTON_RIGHT};

bool s_keys[kKeyCount];
bool s_mouseButtons[ePcOsMouseButton_Count];
bool s_keyTapped[kKeyCount];
bool s_mouseTapped[ePcOsMouseButton_Count];
bool s_capturing = false;
int s_captureResult = -2;
bool s_captureSwallowKey[kKeyCount];
bool s_captureSwallowMouse[ePcOsMouseButton_Count];
bool s_captured = false;
bool s_focused = true;
bool s_textEntry = false;
uint32_t s_heldAfterTextEntry = 0;
uint32_t s_proHeldAfterTextEntry[kPadSlots];
bool s_logInput = false;
unsigned long long s_inputFrame = 0;
std::u16string s_textEntryText;
unsigned int s_textEntryMax = 0;
PcTextEntryResult s_textEntryResult = ePcTextEntry_Pending;
unsigned int s_textEntryKeyboardTyping = 0;
std::u16string::size_type s_textEntryCursor = 0;
bool s_textEntryControl = false;
int s_textEntryHistory = 0;
float s_lookX = 0.0f;
float s_lookY = 0.0f;
float s_mouseX = 0.0f;
float s_mouseY = 0.0f;
bool s_directLook = false;
int s_wheelSteps = 0;
int s_hotbarWheel = 0;
uint32_t s_previousHold = 0;
bool s_inGame = false;
unsigned char s_keyContext[kKeyCount];
unsigned char s_mouseContext[ePcOsMouseButton_Count];
unsigned long long s_containerUntil = 0;
int s_cursorX = 0;
int s_cursorY = 0;
bool s_cursorKnown = false;
bool s_scriptedCursor = false;
float s_scriptedCursorX = 0.0f;
float s_scriptedCursorY = 0.0f;
bool s_mousePointer = false;
bool s_pointerValid = false;
int s_hotbarRequest = -1;
int s_containerHotbarKey = -1;
int s_containerWheel = 0;
bool s_debugCameraRequest = false;
bool s_debugFillAreaRequest = false;
bool s_debugSchematicRequest = false;
char16_t s_chatRequest = 0;
bool s_chatOpen = false;
std::vector<int> modPresses;
bool s_screenshotRequest = false;
std::vector<int> s_scriptKeys;
bool s_hoverPending = false;
PcInputModPad s_modPad;
bool s_modPointer = false;
float s_modPointerX = 0.0f;
float s_modPointerY = 0.0f;
bool s_modPressed = false;
bool s_modChat = false;
PcGlyphSet s_lastDevice = ePcGlyphSet_Keyboard;
unsigned int s_scriptedPad = 0;
PcGlyphSet s_forcedGlyphs = ePcGlyphSet_Auto;
bool s_nintendoButtons = false;
bool s_glideKeys = false;
bool s_lobbyKeys = false;
bool s_spectatorKeys = false;
bool s_glideGameKeys = false;
bool s_backgroundInput = false;
unsigned int s_slotIds[kPadSlots];
PcOsGamepadState s_slotStates[kPadSlots];
uint32_t s_proPreviousHold[kPadSlots];
KPADConnectCallback s_connectCallbacks[kPadSlots - 1];

struct PadButtons
{
    uint32_t a;
    uint32_t b;
    uint32_t x;
    uint32_t y;
    uint32_t l;
    uint32_t r;
    uint32_t zl;
    uint32_t zr;
    uint32_t stickL;
    uint32_t stickR;
    uint32_t plus;
    uint32_t minus;
    uint32_t up;
    uint32_t down;
    uint32_t left;
    uint32_t right;
};

const PadButtons kGamePadButtons = {VPAD_BUTTON_A,       VPAD_BUTTON_B,       VPAD_BUTTON_X,    VPAD_BUTTON_Y,
                                    VPAD_BUTTON_L,       VPAD_BUTTON_R,       VPAD_BUTTON_ZL,   VPAD_BUTTON_ZR,
                                    VPAD_BUTTON_STICK_L, VPAD_BUTTON_STICK_R, VPAD_BUTTON_PLUS, VPAD_BUTTON_MINUS,
                                    VPAD_BUTTON_UP,      VPAD_BUTTON_DOWN,    VPAD_BUTTON_LEFT, VPAD_BUTTON_RIGHT};
const PadButtons kProButtons = {
    WPAD_PRO_BUTTON_A,       WPAD_PRO_BUTTON_B,       WPAD_PRO_BUTTON_X,    WPAD_PRO_BUTTON_Y,
    WPAD_PRO_BUTTON_L,       WPAD_PRO_BUTTON_R,       WPAD_PRO_BUTTON_ZL,   WPAD_PRO_BUTTON_ZR,
    WPAD_PRO_BUTTON_STICK_L, WPAD_PRO_BUTTON_STICK_R, WPAD_PRO_BUTTON_PLUS, WPAD_PRO_BUTTON_MINUS,
    WPAD_PRO_BUTTON_UP,      WPAD_PRO_BUTTON_DOWN,    WPAD_PRO_BUTTON_LEFT, WPAD_PRO_BUTTON_RIGHT};

float Clamp(float value)
{
    return value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
}

const char* SlotName(int slot)
{
    static const char* const kNames[kPadSlots] = {"the GamePad", "Pro Controller 1", "Pro Controller 2",
                                                  "Pro Controller 3", "Pro Controller 4"};
    return kNames[slot];
}

void NotifyProConnection(int slot, bool connected)
{
    KPADConnectCallback callback = s_connectCallbacks[slot - 1];
    if (callback != 0)
        callback(static_cast<WPADChan>(slot - 1), connected ? WPAD_ERROR_NONE : WPAD_ERROR_NO_CONTROLLER);
}

bool GamepadActive(const PcOsGamepadState& state)
{
    float values[] = {state.leftX, state.leftY, state.rightX, state.rightY, state.leftTrigger, state.rightTrigger};
    for (unsigned int index = 0; index < sizeof(values) / sizeof(values[0]); ++index)
    {
        if (values[index] > kGlyphStickThreshold || values[index] < -kGlyphStickThreshold)
            return true;
    }
    return state.buttons != 0;
}

PcGlyphSet GlyphSetForKind(PcOsGamepadKind kind)
{
    switch (kind)
    {
    case ePcOsGamepadKind_PS3:
        return ePcGlyphSet_PS3;
    case ePcOsGamepadKind_PS4:
        return ePcGlyphSet_PS4;
    case ePcOsGamepadKind_Xbox360:
        return ePcGlyphSet_Xbox360;
    default:
        return ePcGlyphSet_XboxOne;
    }
}

void PollGamepads()
{
    PcOsGamepadState states[kPcOsGamepadMax];
    size_t count = PcOsGamepadPoll(states, kPcOsGamepadMax);
    if (s_focused || s_backgroundInput)
    {
        for (size_t index = 0; index < count; ++index)
        {
            if (GamepadActive(states[index]))
                s_lastDevice = GlyphSetForKind(states[index].kind);
        }
    }
    for (int slot = 0; slot < kPadSlots; ++slot)
    {
        if (s_slotIds[slot] == 0)
            continue;
        bool present = false;
        for (size_t index = 0; index < count && !present; ++index)
            present = states[index].id == s_slotIds[slot];
        if (present)
            continue;
        printf("[pc] pad: %s disconnected from %s\n", s_slotStates[slot].name, SlotName(slot));
        fflush(stdout);
        s_slotIds[slot] = 0;
        memset(&s_slotStates[slot], 0, sizeof(s_slotStates[slot]));
        if (slot != kGamePadSlot)
            NotifyProConnection(slot, false);
    }
    for (size_t index = 0; index < count; ++index)
    {
        int slot = -1;
        for (int candidate = 0; candidate < kPadSlots && slot < 0; ++candidate)
        {
            if (s_slotIds[candidate] == states[index].id)
                slot = candidate;
        }
        if (slot < 0)
        {
            for (int candidate = 0; candidate < kPadSlots && slot < 0; ++candidate)
            {
                if (s_slotIds[candidate] == 0)
                    slot = candidate;
            }
            if (slot < 0)
                continue;
            s_slotIds[slot] = states[index].id;
            s_proPreviousHold[slot] = 0;
            printf("[pc] pad: %s connected as %s\n", states[index].name, SlotName(slot));
            fflush(stdout);
            PcOsGamepadSetPlayer(states[index].id, slot + 1);
            if (slot != kGamePadSlot)
                NotifyProConnection(slot, true);
        }
        s_slotStates[slot] = states[index];
        if (!s_focused && !s_backgroundInput)
        {
            PcOsGamepadState& state = s_slotStates[slot];
            state.buttons = 0;
            state.leftX = state.leftY = state.rightX = state.rightY = 0.0f;
            state.leftTrigger = state.rightTrigger = 0.0f;
        }
    }
}

uint32_t MapButtons(const PcOsGamepadState& state, const PadButtons& target)
{
    unsigned int buttons = state.buttons;
    uint32_t hold = 0;
    uint32_t south = s_nintendoButtons ? target.b : target.a;
    uint32_t east = s_nintendoButtons ? target.a : target.b;
    if (buttons & ePcOsGamepad_South)
        hold |= south;
    if (buttons & ePcOsGamepad_East)
        hold |= east;
    if (buttons & ePcOsGamepad_West)
        hold |= target.y;
    if (buttons & ePcOsGamepad_North)
        hold |= target.x;
    if (buttons & ePcOsGamepad_LeftShoulder)
        hold |= target.l;
    if (buttons & ePcOsGamepad_RightShoulder)
        hold |= target.r;
    if (state.leftTrigger > kTriggerThreshold)
        hold |= target.zl;
    if (state.rightTrigger > kTriggerThreshold)
        hold |= target.zr;
    if (buttons & ePcOsGamepad_LeftStick)
        hold |= target.stickL;
    if (buttons & ePcOsGamepad_RightStick)
        hold |= target.stickR;
    if (buttons & ePcOsGamepad_Start)
        hold |= target.plus;
    if (buttons & (ePcOsGamepad_Back | ePcOsGamepad_Touchpad))
        hold |= target.minus;
    if (buttons & ePcOsGamepad_Up)
        hold |= target.up;
    if (buttons & ePcOsGamepad_Down)
        hold |= target.down;
    if (buttons & ePcOsGamepad_Left)
        hold |= target.left;
    if (buttons & ePcOsGamepad_Right)
        hold |= target.right;
    return hold;
}

float Stronger(float first, float second)
{
    float firstMagnitude = first < 0.0f ? -first : first;
    float secondMagnitude = second < 0.0f ? -second : second;
    return secondMagnitude > firstMagnitude ? second : first;
}

InputContext CurrentContext()
{
    if (s_inGame)
        return eInputContext_Game;
    if (PcOsMonotonicNanoseconds() < s_containerUntil)
        return eInputContext_Container;
    return eInputContext_Menu;
}

int MouseIndex(int code)
{
    const int index = code - kPcKeyMouseFirst;
    return index >= 0 && index < ePcOsMouseButton_Count ? index : -1;
}

bool CodeDown(int code)
{
    if (code > 0 && code < kKeyCount)
        return s_keys[code];
    const int mouse = MouseIndex(code);
    return mouse >= 0 && s_mouseButtons[mouse];
}

bool CodeHeldIn(int code, InputContext context)
{
    if (code > 0 && code < kKeyCount)
        return s_keys[code] && s_keyContext[code] == context;
    const int mouse = MouseIndex(code);
    return mouse >= 0 && s_mouseButtons[mouse] && s_mouseContext[mouse] == context;
}

bool CodeActive(int code, InputContext context)
{
    if (code > 0 && code < kKeyCount)
        return (s_keys[code] || s_keyTapped[code]) && s_keyContext[code] == context;
    const int mouse = MouseIndex(code);
    return mouse >= 0 && (s_mouseButtons[mouse] || s_mouseTapped[mouse]) && s_mouseContext[mouse] == context;
}

bool BindingActive(PcKeyAction action, InputContext context)
{
    return CodeActive(PcKeyBinding(action), context);
}

uint32_t ActionButtons(int controllerAction)
{
    if (g_pInput == 0 || gProfile == 0)
        return 0;
    const int pad = gProfile->GetPrimaryPad();
    const uint32_t mapping = g_pInput->GetGameJoypadMaps(pad < 0 ? 0 : pad, static_cast<unsigned char>(controllerAction));
    uint32_t buttons = 0;
    for (unsigned int index = 0; index < sizeof(kJoypadBits) / sizeof(kJoypadBits[0]); ++index)
    {
        if ((mapping & kJoypadBits[index]) != 0)
            buttons |= kJoypadVpad[index];
    }
    return buttons;
}

void CodePressed(int code, InputContext context)
{
    for (int slot = 0; slot < kHotbarSlots; ++slot)
    {
        if (PcKeyBinding(static_cast<PcKeyAction>(ePcKey_Hotbar1 + slot)) != code)
            continue;
        if (context == eInputContext_Container)
            s_containerHotbarKey = slot;
        else
            s_hotbarRequest = slot;
    }
    if (code == PcKeyBinding(ePcKey_Screenshot))
        s_screenshotRequest = true;
    if (s_inGame && context == eInputContext_Game && !s_chatOpen && !s_textEntry)
    {
        for (int act = ePcKey_Count; act < PcKeyBindingCount(); act++)
        {
            if (PcKeyBinding((PcKeyAction)act) != code)
                continue;
            if ((int)modPresses.size() < act - ePcKey_Count + 1)
                modPresses.resize(act - ePcKey_Count + 1, 0);
            modPresses[act - ePcKey_Count]++;
        }
    }
    if (s_inGame && context == eInputContext_Game && s_chatRequest == 0)
    {
        if (code == PcKeyBinding(ePcKey_Chat))
            s_chatRequest = u't';
        else if (code == PcKeyBinding(ePcKey_Command))
            s_chatRequest = u'/';
    }
}

bool CaptureEvent(const PcOsWindowEvent& event)
{
    if (event.type == ePcOsWindowEvent_Key && event.key >= 0 && event.key < kKeyCount)
    {
        if (!event.down)
        {
            s_captureSwallowKey[event.key] = false;
            s_keys[event.key] = false;
            return true;
        }
        if (s_keys[event.key] || s_captureSwallowKey[event.key])
            return true;
        s_captureSwallowKey[event.key] = true;
        s_captureResult = event.key == kKeyEscape ? -1 : event.key;
        s_capturing = false;
        return true;
    }
    if (event.type == ePcOsWindowEvent_MouseButton && event.key >= 0 && event.key < ePcOsMouseButton_Count)
    {
        if (!event.down)
        {
            s_captureSwallowMouse[event.key] = false;
            s_mouseButtons[event.key] = false;
            return true;
        }
        if (s_mouseButtons[event.key] || s_captureSwallowMouse[event.key])
            return true;
        s_captureSwallowMouse[event.key] = true;
        s_captureResult = kPcKeyMouseFirst + event.key;
        s_capturing = false;
        return true;
    }
    return event.type == ePcOsWindowEvent_Text;
}

bool LogicalFrame(float* left, float* top, float* frameWidth, float* frameHeight)
{
    int width = 0;
    int height = 0;
    PcOsWindowGetClientSize(&width, &height);
    if (width <= 0 || height <= 0)
        return false;
    *frameWidth = static_cast<float>(width);
    *frameHeight = *frameWidth * kLogicalHeight / kLogicalWidth;
    if (*frameHeight > static_cast<float>(height))
    {
        *frameHeight = static_cast<float>(height);
        *frameWidth = *frameHeight * kLogicalWidth / kLogicalHeight;
    }
    *left = (static_cast<float>(width) - *frameWidth) * 0.5f;
    *top = (static_cast<float>(height) - *frameHeight) * 0.5f;
    return true;
}

bool CursorToLogical(float* x, float* y)
{
    if (s_modPointer)
    {
        *x = s_modPointerX;
        *y = s_modPointerY;
        return true;
    }
    if (s_scriptedCursor)
    {
        *x = s_scriptedCursorX;
        *y = s_scriptedCursorY;
        return true;
    }
    float left;
    float top;
    float frameWidth;
    float frameHeight;
    if (!s_cursorKnown || !LogicalFrame(&left, &top, &frameWidth, &frameHeight))
        return false;
    *x = (static_cast<float>(s_cursorX) - left) * kLogicalWidth / frameWidth;
    *y = (static_cast<float>(s_cursorY) - top) * kLogicalHeight / frameHeight;
    return *x >= 0.0f && *x < kLogicalWidth && *y >= 0.0f && *y < kLogicalHeight;
}

void ApplyModPointer()
{
    const InputContext context = CurrentContext();
    bool inside = false;
    float x = 0.0f;
    float y = 0.0f;
    if (s_modPad.pointer && context != eInputContext_Game)
    {
        int width = 0;
        int height = 0;
        PcGetRenderSize(&width, &height);
        if (width > 0 && height > 0)
        {
            const float scale = static_cast<float>(width) / kLogicalWidth < static_cast<float>(height) / kLogicalHeight
                                    ? static_cast<float>(width) / kLogicalWidth
                                    : static_cast<float>(height) / kLogicalHeight;
            const float left = (static_cast<float>(width) - kLogicalWidth * scale) * 0.5f;
            const float top = (static_cast<float>(height) - kLogicalHeight * scale) * 0.5f;
            x = (s_modPad.pointerX * width - left) / scale;
            y = (s_modPad.pointerY * height - top) / scale;
            inside = x >= 0.0f && x < kLogicalWidth && y >= 0.0f && y < kLogicalHeight;
        }
    }
    if (inside && (!s_modPointer || x != s_modPointerX || y != s_modPointerY))
    {
        s_hoverPending = true;
        s_mousePointer = true;
    }
    s_modPointer = inside;
    s_modPointerX = x;
    s_modPointerY = y;
    const bool press = inside && s_modPad.press;
    if (press && !s_modPressed)
    {
        s_mouseContext[ePcOsMouseButton_Left] = static_cast<unsigned char>(context);
        s_mouseTapped[ePcOsMouseButton_Left] = true;
        s_mouseButtons[ePcOsMouseButton_Left] = true;
        s_mousePointer = true;
    }
    else if (!press && s_modPressed)
        s_mouseButtons[ePcOsMouseButton_Left] = false;
    s_modPressed = press;
}

bool NavigationInput(const PcOsGamepadState& pad)
{
    if (s_keys[kKeyLeft] || s_keys[kKeyRight] || s_keys[kKeyUp] || s_keys[kKeyDown] || s_keys['W'] || s_keys['A']
        || s_keys['S'] || s_keys['D'])
        return true;
    float x = pad.leftX < 0.0f ? -pad.leftX : pad.leftX;
    float y = pad.leftY < 0.0f ? -pad.leftY : pad.leftY;
    return x > kNavigationStick || y > kNavigationStick
           || (pad.buttons & (ePcOsGamepad_Up | ePcOsGamepad_Down | ePcOsGamepad_Left | ePcOsGamepad_Right)) != 0;
}

void SetCaptured(bool captured)
{
    s_captured = captured;
    PcOsWindowSetMouseCaptured(captured);
    s_lookX = 0.0f;
    s_lookY = 0.0f;
    s_mouseX = 0.0f;
    s_mouseY = 0.0f;
}
}

bool PcInputActive()
{
    return PcGetDisplayMode() != ePcDisplay_Headless;
}

void PcInputHandleEvent(const PcOsWindowEvent& event)
{
    if (event.type == ePcOsWindowEvent_Key || event.type == ePcOsWindowEvent_MouseButton
        || event.type == ePcOsWindowEvent_MouseWheel
        || (event.type == ePcOsWindowEvent_MouseMove && (!event.down || event.x * event.x + event.y * event.y > 4)))
        s_lastDevice = ePcGlyphSet_Keyboard;
    if (s_logInput && (event.type == ePcOsWindowEvent_Key || event.type == ePcOsWindowEvent_MouseButton))
    {
        printf("[pc] input: %s %d %s at %llu ms, frame %llu\n",
               event.type == ePcOsWindowEvent_Key ? "key" : "mouse", event.key, event.down ? "down" : "up",
               PcOsMonotonicNanoseconds() / 1000000ULL, s_inputFrame);
        fflush(stdout);
    }
    if (s_capturing && CaptureEvent(event))
        return;
    if (event.type == ePcOsWindowEvent_Key && event.key >= 0 && event.key < kKeyCount && s_captureSwallowKey[event.key])
    {
        if (!event.down)
            s_captureSwallowKey[event.key] = false;
        return;
    }
    if (event.type == ePcOsWindowEvent_MouseButton && event.key >= 0 && event.key < ePcOsMouseButton_Count
        && s_captureSwallowMouse[event.key])
    {
        if (!event.down)
            s_captureSwallowMouse[event.key] = false;
        return;
    }
    if (s_textEntry && (event.type == ePcOsWindowEvent_Key || event.type == ePcOsWindowEvent_Text))
    {
        if (event.type == ePcOsWindowEvent_Key && event.key == kKeyControl)
            s_textEntryControl = event.down;
        if (event.type == ePcOsWindowEvent_Key && event.down)
        {
            bool edited = true;
            if (event.key == kKeyBackspace)
                PcTextEntryBackspace();
            else if (event.key == kKeyDelete)
                PcTextEntryDelete();
            else if (event.key == kKeyLeft)
                PcTextEntryMoveCursor(-1, s_textEntryControl);
            else if (event.key == kKeyRight)
                PcTextEntryMoveCursor(1, s_textEntryControl);
            else if (event.key == kKeyHome)
                s_textEntryCursor = 0;
            else if (event.key == kKeyEnd)
                s_textEntryCursor = s_textEntryText.size();
            else if (event.key == kKeyUp)
                s_textEntryHistory = -1;
            else if (event.key == kKeyDown)
                s_textEntryHistory = 1;
            else
            {
                edited = false;
                if (event.key == kKeyReturn)
                    s_textEntryResult = ePcTextEntry_Accepted;
                else if (event.key == kKeyEscape)
                    s_textEntryResult = ePcTextEntry_Cancelled;
            }
            if (edited)
                ++s_textEntryKeyboardTyping;
        }
        else if (event.type == ePcOsWindowEvent_Text && event.character == 0x16)
        {
            PcTextEntryPaste();
            ++s_textEntryKeyboardTyping;
        }
        else if (event.type == ePcOsWindowEvent_Text && event.character >= 0x20 && event.character != 0x7F
                 && event.character < 0x10000)
        {
            PcTextEntryType(static_cast<char16_t>(event.character));
            ++s_textEntryKeyboardTyping;
        }
        return;
    }
    switch (event.type)
    {
    case ePcOsWindowEvent_Key:
        if (event.key >= 0 && event.key < kKeyCount)
        {
            if (event.down && !s_keys[event.key])
            {
                s_keyTapped[event.key] = true;
                s_keyContext[event.key] = static_cast<unsigned char>(CurrentContext());
                CodePressed(event.key, CurrentContext());
                if (s_inGame && s_scriptKeys.size() < 32)
                    s_scriptKeys.push_back(event.key);
                if (event.key == kKeyF3)
                    PcDebugOverlayToggle();
                if (event.key == kKeyF6 && s_inGame)
                    s_debugCameraRequest = true;
                if (event.key == kKeyF7 && s_inGame)
                    s_debugFillAreaRequest = true;
                if (event.key == kKeyF8 && s_inGame)
                    s_debugSchematicRequest = true;
            }
            s_keys[event.key] = event.down;
        }
        break;
    case ePcOsWindowEvent_MouseButton:
        if (event.key >= 0 && event.key < ePcOsMouseButton_Count)
        {
            if (event.down && !s_mouseButtons[event.key])
            {
                s_mouseContext[event.key] = static_cast<unsigned char>(CurrentContext());
                s_mouseTapped[event.key] = true;
                CodePressed(kPcKeyMouseFirst + event.key, CurrentContext());
            }
            s_mouseButtons[event.key] = event.down;
            if (!s_captured)
                s_mousePointer = true;
        }
        break;
    case ePcOsWindowEvent_MouseMove:
        if (!event.down)
        {
            s_cursorX = event.x;
            s_cursorY = event.y;
            s_cursorKnown = true;
            s_mousePointer = true;
            s_hoverPending = true;
        }
        else if (s_captured)
        {
            if (s_directLook)
            {
                s_mouseX += static_cast<float>(event.x);
                s_mouseY += static_cast<float>(event.y);
            }
            else
            {
                s_lookX += static_cast<float>(event.x) * kMouseLookScale;
                s_lookY -= static_cast<float>(event.y) * kMouseLookScale;
            }
        }
        break;
    case ePcOsWindowEvent_MouseWheel:
        if (CurrentContext() == eInputContext_Game)
        {
            s_hotbarWheel += event.delta > 0 ? 1 : -1;
            PcInputNoteHotbarWheel(event.delta > 0 ? 0 : 1);
        }
        else if (CurrentContext() == eInputContext_Container)
            s_containerWheel += event.delta > 0 ? 1 : -1;
        else
            s_wheelSteps += event.delta > 0 ? 1 : -1;
        break;
    case ePcOsWindowEvent_Focus:
        s_focused = event.down;
        if (!event.down)
        {
            memset(s_keys, 0, sizeof(s_keys));
            memset(s_mouseButtons, 0, sizeof(s_mouseButtons));
            memset(s_keyTapped, 0, sizeof(s_keyTapped));
            memset(s_mouseTapped, 0, sizeof(s_mouseTapped));
            if (s_captured)
                SetCaptured(false);
        }
        break;
    default:
        break;
    }
}

void PcInputFillGamePad(VPADStatus* status)
{
    PollGamepads();
    ApplyModPointer();
    memset(status, 0, sizeof(VPADStatus));
    const PcOsGamepadState& pad = s_slotStates[kGamePadSlot];
    uint32_t hold = s_slotIds[kGamePadSlot] != 0 ? MapButtons(pad, kGamePadButtons) : 0;
    hold |= s_modPointer && CurrentContext() != eInputContext_Game ? s_modPad.hold & ~(VPAD_BUTTON_ZL | VPAD_BUTTON_ZR)
                                                                   : s_modPad.hold;
    if (s_scriptedPad != 0)
    {
        PcOsGamepadState scripted;
        memset(&scripted, 0, sizeof(scripted));
        scripted.buttons = s_scriptedPad;
        hold |= MapButtons(scripted, kGamePadButtons);
        s_lastDevice = ePcGlyphSet_XboxOne;
    }
    const InputContext context = CurrentContext();
    for (unsigned int index = 0; index < sizeof(kKeyBindings) / sizeof(kKeyBindings[0]); ++index)
    {
        if (s_keys[kKeyBindings[index].key] || s_keyTapped[kKeyBindings[index].key])
            hold |= kKeyBindings[index].buttons[s_keyContext[kKeyBindings[index].key]];
    }
    if (context == eInputContext_Game)
    {
        for (int action = 0; action < ePcKey_Count; ++action)
        {
            const int controllerAction = PcKeyBindingGameAction(static_cast<PcKeyAction>(action));
            if (controllerAction >= 0 && PcInputKeySectionActive(PcKeyBindingSection(static_cast<PcKeyAction>(action)))
                && BindingActive(static_cast<PcKeyAction>(action), eInputContext_Game))
                hold |= ActionButtons(controllerAction);
        }
        if (BindingActive(ePcKey_PlayerList, eInputContext_Game))
            hold |= VPAD_BUTTON_MINUS;
        if (s_glideKeys && BindingActive(ePcKey_GlideGhost, eInputContext_Game))
            hold |= VPAD_BUTTON_R;
    }
    else if (context == eInputContext_Container
             && (BindingActive(ePcKey_Inventory, eInputContext_Container)
                 || BindingActive(ePcKey_Crafting, eInputContext_Container)))
    {
        hold |= VPAD_BUTTON_B;
    }
    if (context != eInputContext_Game && NavigationInput(pad))
        s_mousePointer = false;

    const bool left = (s_mouseButtons[ePcOsMouseButton_Left] || s_mouseTapped[ePcOsMouseButton_Left])
                      && s_mouseContext[ePcOsMouseButton_Left] == context;
    const bool right = (s_mouseButtons[ePcOsMouseButton_Right] || s_mouseTapped[ePcOsMouseButton_Right])
                       && s_mouseContext[ePcOsMouseButton_Right] == context;
    memset(s_keyTapped, 0, sizeof(s_keyTapped));
    memset(s_mouseTapped, 0, sizeof(s_mouseTapped));
    uint32_t wheelUp = 0;
    uint32_t wheelDown = 0;
    switch (context)
    {
    case eInputContext_Container:
        if (left)
            hold |= s_keys[kKeyShift] ? VPAD_BUTTON_X : VPAD_BUTTON_A;
        if (right)
            hold |= VPAD_BUTTON_Y;
        break;
    case eInputContext_Menu:
    {
        wheelUp = VPAD_BUTTON_UP;
        wheelDown = VPAD_BUTTON_DOWN;
        float x = 0.0f;
        float y = 0.0f;
        if (s_mousePointer && CursorToLogical(&x, &y))
        {
            status->tpNormal.x = static_cast<uint16_t>(x);
            status->tpNormal.y = static_cast<uint16_t>(y);
            status->tpNormal.touched = left ? 1 : 0;
            status->tpNormal.validity = 0;
        }
        break;
    }
    default:
        break;
    }
    if (s_wheelSteps > 0 && !(s_previousHold & wheelUp))
    {
        hold |= wheelUp;
        --s_wheelSteps;
    }
    else if (s_wheelSteps < 0 && !(s_previousHold & wheelDown))
    {
        hold |= wheelDown;
        ++s_wheelSteps;
    }

    s_heldAfterTextEntry &= hold;
    hold &= ~s_heldAfterTextEntry;
    status->hold = hold;
    status->trigger = hold & ~s_previousHold;
    status->release = s_previousHold & ~hold;
    ++s_inputFrame;
    if (s_logInput && (status->trigger != 0 || status->release != 0 || status->tpNormal.touched))
    {
        printf("[pc] input: frame %llu game pressed 0x%x released 0x%x touch %d (%d, %d) context %d\n", s_inputFrame,
               static_cast<unsigned int>(status->trigger), static_cast<unsigned int>(status->release),
               status->tpNormal.touched, status->tpNormal.x, status->tpNormal.y, static_cast<int>(context));
        fflush(stdout);
    }
    s_previousHold = hold;

    status->leftStick.x = Clamp((CodeDown(PcKeyBinding(ePcKey_Right)) ? 1.0f : 0.0f)
                                - (CodeDown(PcKeyBinding(ePcKey_Left)) ? 1.0f : 0.0f));
    status->leftStick.y = Clamp((CodeDown(PcKeyBinding(ePcKey_Forward)) ? 1.0f : 0.0f)
                                - (CodeDown(PcKeyBinding(ePcKey_Back)) ? 1.0f : 0.0f));
    status->rightStick.x = Clamp(s_lookX);
    status->rightStick.y = Clamp(s_lookY);
    status->leftStick.x = Stronger(status->leftStick.x, pad.leftX);
    status->leftStick.y = Stronger(status->leftStick.y, pad.leftY);
    status->rightStick.x = Clamp(Stronger(status->rightStick.x, pad.rightX));
    status->rightStick.y = Clamp(Stronger(status->rightStick.y, pad.rightY));
    status->leftStick.x = Clamp(Stronger(status->leftStick.x, s_modPad.leftX));
    status->leftStick.y = Clamp(Stronger(status->leftStick.y, s_modPad.leftY));
    status->rightStick.x = Clamp(Stronger(status->rightStick.x, s_modPad.rightX));
    status->rightStick.y = Clamp(Stronger(status->rightStick.y, s_modPad.rightY));
    s_lookX *= kMouseLookDecay;
    s_lookY *= kMouseLookDecay;
    if (s_lookX > -0.01f && s_lookX < 0.01f)
        s_lookX = 0.0f;
    if (s_lookY > -0.01f && s_lookY < 0.01f)
        s_lookY = 0.0f;
    if (s_chatOpen)
    {
        status->trigger = 0;
        status->release = s_previousHold;
        status->hold = 0;
        s_previousHold = 0;
        status->leftStick.x = status->leftStick.y = 0.0f;
        status->rightStick.x = status->rightStick.y = 0.0f;
    }
}

bool PcInputFillProController(int channel, KPADStatus* status)
{
    int slot = channel + 1;
    memset(status, 0, sizeof(KPADStatus));
    if (channel < 0 || slot >= kPadSlots || s_slotIds[slot] == 0)
    {
        status->extensionType = WPAD_EXT_DEV_NOT_FOUND;
        status->error = KPAD_ERROR_NO_SAMPLES;
        return false;
    }
    const PcOsGamepadState& pad = s_slotStates[slot];
    uint32_t hold = MapButtons(pad, kProButtons);
    s_proHeldAfterTextEntry[slot] &= hold;
    hold &= ~s_proHeldAfterTextEntry[slot];
    status->extensionType = WPAD_EXT_PRO_CONTROLLER;
    status->error = KPAD_ERROR_OK;
    status->pro.hold = hold;
    status->pro.trigger = hold & ~s_proPreviousHold[slot];
    status->pro.release = s_proPreviousHold[slot] & ~hold;
    status->pro.leftStick.x = pad.leftX;
    status->pro.leftStick.y = pad.leftY;
    status->pro.rightStick.x = pad.rightX;
    status->pro.rightStick.y = pad.rightY;
    status->pro.wired = 1;
    s_proPreviousHold[slot] = hold;
    return true;
}

KPADConnectCallback PcInputSetConnectCallback(int channel, KPADConnectCallback callback)
{
    if (channel < 0 || channel >= kPadSlots - 1)
        return 0;
    KPADConnectCallback previous = s_connectCallbacks[channel];
    s_connectCallbacks[channel] = callback;
    return previous;
}

void PcInputSetNintendoButtons(bool nintendo)
{
    s_nintendoButtons = nintendo;
}

void PcInputSetBackgroundInput(bool background)
{
    s_backgroundInput = background;
}

void PcInputSetInGame(bool inGame)
{
    if (!inGame)
        s_hotbarWheel = 0;
    s_inGame = inGame;
    const bool window = PcGetDisplayMode() == ePcDisplay_Window;
    const bool capture = inGame && s_focused && window;
    if (capture != s_captured)
        SetCaptured(capture);
    if (window)
        PcOsWindowSetCursorHidden(!s_captured && s_focused && CurrentContext() != eInputContext_Game);
}

bool PcInputMenuPointer(float* x, float* y)
{
    s_containerUntil = PcOsMonotonicNanoseconds() + kContainerLingerNanoseconds;
    s_pointerValid = !s_inGame && s_mousePointer && CursorToLogical(x, y);
    return s_pointerValid;
}

bool PcInputMenuPointerActive()
{
    return !s_inGame && s_mousePointer && s_pointerValid;
}

bool PcInputMenuCursor(int* windowX, int* windowY)
{
    if (s_captured || !s_mousePointer || CurrentContext() != eInputContext_Menu)
        return false;
    if (!s_scriptedCursor)
    {
        if (!s_cursorKnown)
            return false;
        *windowX = s_cursorX;
        *windowY = s_cursorY;
        return true;
    }
    float left;
    float top;
    float frameWidth;
    float frameHeight;
    if (!LogicalFrame(&left, &top, &frameWidth, &frameHeight))
        return false;
    *windowX = static_cast<int>(left + s_scriptedCursorX * frameWidth / kLogicalWidth);
    *windowY = static_cast<int>(top + s_scriptedCursorY * frameHeight / kLogicalHeight);
    return true;
}

bool PcInputConsumeHover(float* x, float* y)
{
    bool pending = s_hoverPending;
    s_hoverPending = false;
    return pending && CurrentContext() == eInputContext_Menu && s_mousePointer
           && !s_mouseButtons[ePcOsMouseButton_Left] && CursorToLogical(x, y);
}

PcGlyphSet PcInputGlyphSet()
{
    return s_forcedGlyphs != ePcGlyphSet_Auto ? s_forcedGlyphs : s_lastDevice;
}

int PcInputGlyphContext()
{
    return static_cast<int>(CurrentContext());
}

bool PcInputNintendoButtons()
{
    return s_nintendoButtons;
}

namespace
{
const unsigned long long kMouseLookWindowMilliseconds = 250;
unsigned long long s_mouseLookTimes[4];
unsigned long long s_hotbarWheelTimes[2];
}

void PcInputNoteHotbarWheel(int direction)
{
    s_hotbarWheelTimes[direction & 1] = PcOsRealTimeMilliseconds();
}

bool PcInputRecentHotbarWheel(int direction)
{
    if (direction < 0 || direction > 1 || s_hotbarWheelTimes[direction] == 0)
        return false;
    return PcOsRealTimeMilliseconds() - s_hotbarWheelTimes[direction] <= kMouseLookWindowMilliseconds;
}

void PcInputNoteMouseLook(float x, float y)
{
    const unsigned long long now = PcOsRealTimeMilliseconds();
    if (y < 0.0f)
        s_mouseLookTimes[0] = now;
    if (y > 0.0f)
        s_mouseLookTimes[1] = now;
    if (x < 0.0f)
        s_mouseLookTimes[2] = now;
    if (x > 0.0f)
        s_mouseLookTimes[3] = now;
}

bool PcInputRecentMouseLook(int direction)
{
    if (direction < 0 || direction > 3 || s_mouseLookTimes[direction] == 0)
        return false;
    return PcOsRealTimeMilliseconds() - s_mouseLookTimes[direction] <= kMouseLookWindowMilliseconds;
}

void PcInputSetGlyphSet(PcGlyphSet set)
{
    s_forcedGlyphs = set;
}

void PcInputSetScriptedCursor(float x, float y)
{
    s_hoverPending = true;
    s_scriptedCursor = true;
    s_scriptedCursorX = x;
    s_scriptedCursorY = y;
    s_mousePointer = true;
}

char16_t PcInputConsumeChatKey()
{
    char16_t key = s_chatRequest;
    s_chatRequest = 0;
    return key;
}

void PcInputSetChatOpen(bool open)
{
    s_chatOpen = open;
}

bool PcInputConsumeDebugCameraKey()
{
    bool requested = s_debugCameraRequest;
    s_debugCameraRequest = false;
    return requested;
}

bool PcInputConsumeDebugFillAreaKey()
{
    bool requested = s_debugFillAreaRequest;
    s_debugFillAreaRequest = false;
    return requested;
}

bool PcInputConsumeDebugSchematicKey()
{
    bool requested = s_debugSchematicRequest;
    s_debugSchematicRequest = false;
    return requested;
}

int PcInputConsumeHotbarWheel()
{
    int steps = s_hotbarWheel;
    s_hotbarWheel = 0;
    return steps;
}

int PcInputConsumeContainerHotbarKey()
{
    int slot = s_containerHotbarKey;
    s_containerHotbarKey = -1;
    return slot;
}

int PcInputConsumeContainerWheel()
{
    int steps = s_containerWheel;
    s_containerWheel = 0;
    return steps;
}

bool PcInputContainerLeftHeld()
{
    return s_mouseButtons[ePcOsMouseButton_Left] && s_mouseContext[ePcOsMouseButton_Left] == eInputContext_Container;
}

int PcInputConsumeHotbarKey()
{
    int slot = s_hotbarRequest;
    s_hotbarRequest = -1;
    return slot;
}

bool PcInputSneakHeld()
{
    return (CodeHeldIn(PcKeyBinding(ePcKey_Sneak), eInputContext_Game) && !s_textEntry)
           || (s_modPad.sneak && CurrentContext() == eInputContext_Game);
}

bool PcInputKeyboardStrafing()
{
    return CodeHeldIn(PcKeyBinding(ePcKey_Forward), eInputContext_Game) && !CodeDown(PcKeyBinding(ePcKey_Back))
           && CodeDown(PcKeyBinding(ePcKey_Left)) != CodeDown(PcKeyBinding(ePcKey_Right)) && !s_textEntry;
}

bool PcInputSprintHeld()
{
    return CodeDown(PcKeyBinding(ePcKey_Sprint)) && CodeDown(PcKeyBinding(ePcKey_Forward))
           && !CodeDown(PcKeyBinding(ePcKey_Back));
}

bool PcInputBindingHeld(PcKeyAction action)
{
    return CodeHeldIn(PcKeyBinding(action), eInputContext_Game) && !s_textEntry && !s_chatOpen;
}

uint32_t PcInputActionButtons(int controllerAction)
{
    return ActionButtons(controllerAction);
}

void PcInputBeginKeyCapture()
{
    s_capturing = true;
    s_captureResult = -2;
}

void PcInputCancelKeyCapture()
{
    s_capturing = false;
    s_captureResult = -2;
}

bool PcInputKeyCapturing()
{
    return s_capturing;
}

int PcInputTakeKeyCapture()
{
    const int result = s_captureResult;
    s_captureResult = -2;
    return result;
}

bool PcInputConsumeScreenshotKey()
{
    bool requested = s_screenshotRequest;
    s_screenshotRequest = false;
    return requested;
}

int PcInputConsumeScriptKey()
{
    if (s_scriptKeys.empty())
        return -1;
    const int key = s_scriptKeys.front();
    s_scriptKeys.erase(s_scriptKeys.begin());
    return key;
}

bool PcInputPushToTalkHeld()
{
    return s_focused && !s_chatOpen && !s_textEntry && CodeDown(PcKeyBinding(ePcKey_PushToTalk));
}

bool PcInputModKeyHeld(int action)
{
    if (action < ePcKey_Count || action >= PcKeyBindingCount())
        return false;
    return s_focused && !s_chatOpen && !s_textEntry && CurrentContext() == eInputContext_Game
           && CodeHeldIn(PcKeyBinding((PcKeyAction)action), eInputContext_Game);
}

int PcInputModKeyPresses(int action)
{
    int i = action - ePcKey_Count;
    if (i < 0 || i >= (int)modPresses.size())
        return 0;
    int n = modPresses[i];
    modPresses[i] = 0;
    return n;
}

bool PcInputKeyHeld(int key)
{
    return key >= 0 && key < kKeyCount && s_keys[key];
}

void PcInputSetDirectLook(bool direct)
{
    if (direct == s_directLook)
        return;
    s_directLook = direct;
    s_lookX = 0.0f;
    s_lookY = 0.0f;
    s_mouseX = 0.0f;
    s_mouseY = 0.0f;
}

void PcInputConsumeMouseDelta(float* x, float* y)
{
    *x = s_mouseX;
    *y = s_mouseY;
    s_mouseX = 0.0f;
    s_mouseY = 0.0f;
}

void PcTextEntryBegin(const char16_t* initialText, unsigned int maxLength)
{
    s_textEntry = true;
    s_textEntryMax = maxLength;
    s_textEntryText.clear();
    for (; initialText != 0 && *initialText != 0 && s_textEntryText.size() < maxLength; ++initialText)
        s_textEntryText.push_back(*initialText);
    s_textEntryCursor = s_textEntryText.size();
    s_textEntryControl = false;
    s_textEntryHistory = 0;
    s_textEntryResult = ePcTextEntry_Pending;
    memset(s_keys, 0, sizeof(s_keys));
    memset(s_keyTapped, 0, sizeof(s_keyTapped));
}

void PcTextEntryEnd()
{
    s_heldAfterTextEntry = s_previousHold;
    s_previousHold = 0;
    for (int slot = 0; slot < kPadSlots; ++slot)
    {
        s_proHeldAfterTextEntry[slot] = s_proPreviousHold[slot];
        s_proPreviousHold[slot] = 0;
    }
    s_textEntry = false;
    s_textEntryResult = ePcTextEntry_Pending;
}

bool PcTextEntryActive()
{
    return s_textEntry;
}

const char16_t* PcTextEntryText()
{
    return s_textEntryText.c_str();
}

bool PcInputPointerContext()
{
    return CurrentContext() != eInputContext_Game;
}

void PcInputSetModPad(const PcInputModPad& pad)
{
    if (pad.chat && !s_modChat && !s_chatOpen)
        s_chatRequest = u't';
    s_modChat = pad.chat;
    s_modPad = pad;
}

void PcInputSetScriptedPad(unsigned int buttons)
{
    s_scriptedPad = buttons;
}

void PcTextEntryType(char16_t character)
{
    if (s_textEntry && s_textEntryText.size() < s_textEntryMax)
    {
        s_textEntryText.insert(s_textEntryCursor, 1, character);
        ++s_textEntryCursor;
    }
}

void PcTextEntryBackspace()
{
    if (s_textEntry && s_textEntryCursor > 0)
    {
        s_textEntryText.erase(s_textEntryCursor - 1, 1);
        --s_textEntryCursor;
    }
}

void PcTextEntryDelete()
{
    if (s_textEntry && s_textEntryCursor < s_textEntryText.size())
        s_textEntryText.erase(s_textEntryCursor, 1);
}

void PcTextEntryMoveCursor(int direction, bool word)
{
    if (!s_textEntry)
        return;
    if (direction < 0)
    {
        if (s_textEntryCursor > 0)
            --s_textEntryCursor;
        while (word && s_textEntryCursor > 0 && s_textEntryText[s_textEntryCursor] == u' ')
            --s_textEntryCursor;
        while (word && s_textEntryCursor > 0 && s_textEntryText[s_textEntryCursor - 1] != u' ')
            --s_textEntryCursor;
    }
    else
    {
        if (s_textEntryCursor < s_textEntryText.size())
            ++s_textEntryCursor;
        while (word && s_textEntryCursor < s_textEntryText.size() && s_textEntryText[s_textEntryCursor] != u' ')
            ++s_textEntryCursor;
        while (word && s_textEntryCursor < s_textEntryText.size() && s_textEntryText[s_textEntryCursor] == u' ')
            ++s_textEntryCursor;
    }
}

void PcTextEntryPaste()
{
    if (!s_textEntry)
        return;
    std::wstring clipboard = PcOsClipboardText();
    std::u16string text;
    for (std::wstring::size_type i = 0; i < clipboard.length(); ++i)
    {
        wchar_t character = clipboard[i];
        if (character == L'\n' || character == L'\t')
            character = L' ';
        if (character >= 0x20 && character != 0x7F && character < 0x10000)
            text.push_back(static_cast<char16_t>(character));
    }
    std::u16string::size_type first = text.find_first_not_of(u' ');
    if (first == std::u16string::npos)
        return;
    text = text.substr(first, text.find_last_not_of(u' ') - first + 1);
    if (s_textEntryText.size() + text.size() > s_textEntryMax)
        text.resize(s_textEntryMax - s_textEntryText.size());
    s_textEntryText.insert(s_textEntryCursor, text);
    s_textEntryCursor += text.size();
}

void PcTextEntrySetText(const std::u16string& text)
{
    if (!s_textEntry)
        return;
    s_textEntryText = text.substr(0, s_textEntryMax);
    s_textEntryCursor = s_textEntryText.size();
}

unsigned int PcTextEntryCursor()
{
    return static_cast<unsigned int>(s_textEntryCursor);
}

int PcTextEntryConsumeHistoryKey()
{
    int key = s_textEntryHistory;
    s_textEntryHistory = 0;
    return key;
}

void PcTextEntrySetResult(PcTextEntryResult result)
{
    if (s_textEntry)
        s_textEntryResult = result;
}

PcTextEntryResult PcTextEntryCurrentResult()
{
    return s_textEntryResult;
}

unsigned int PcTextEntryKeyboardTyping()
{
    return s_textEntryKeyboardTyping;
}

void PcInputSetLogging(bool log)
{
    s_logInput = log;
}

bool PcInputLogging()
{
    return s_logInput;
}

void PcInputSetGlideKeys(bool active)
{
    s_glideKeys = active;
}

bool PcInputGlideKeys()
{
    return s_glideKeys;
}

void PcInputSetMiniGameKeys(bool lobby, bool spectating, bool glide)
{
    s_lobbyKeys = lobby;
    s_spectatorKeys = spectating;
    s_glideGameKeys = glide;
}

bool PcInputKeySectionActive(PcKeySection section)
{
    switch (section)
    {
    case ePcKeySection_MiniGameLobby:
        return s_lobbyKeys;
    case ePcKeySection_Spectating:
        return s_spectatorKeys;
    case ePcKeySection_Glide:
        return s_glideGameKeys;
    default:
        return true;
    }
}
