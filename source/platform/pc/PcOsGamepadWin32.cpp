#ifdef _WIN32

#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <xinput.h>

#include "platform/pc/PcOs.hpp"

#include <set>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

namespace
{
const unsigned short kSonyVendor = 0x054C;
const unsigned short kDualShock3Product = 0x0268;
const unsigned short kDualShock4Products[] = {0x05C4, 0x09CC, 0x0BA0};
const unsigned short kDualSenseProducts[] = {0x0CE6, 0x0DF2};
const unsigned short kHidUsagePageGeneric = 0x01;
const unsigned short kHidUsageGamepad = 0x05;
const unsigned short kHidUsageJoystick = 0x04;
const unsigned char kDualShock3EnableReports[] = {0xF2, 0xF5};
const DWORD kDualShock3ReportLength = 20;
const unsigned char kDualShock3Output[] = {0x01, 0x01, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF, 0x27,
                                           0x10, 0x00, 0x32, 0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF,
                                           0x27, 0x10, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
const size_t kDualShock3LedByte = 10;
const unsigned char kDualShock3PlayerLeds[] = {0x02, 0x04, 0x08, 0x10, 0x12, 0x14, 0x18};
const unsigned short kUsbReportLength = 64;
const unsigned char kDualSenseCalibrationReport = 0x05;
const unsigned char kDualShock4CalibrationReport = 0x02;
const DWORD kRescanMilliseconds = 2000;
const unsigned long long kXInputProbeMilliseconds = 1000;
const unsigned int kXInputIdBase = 0x100;
const unsigned int kHidIdBase = 0x1000;
const DWORD kXInputGetStateExOrdinal = 100;
const DWORD kXInputGetCapabilitiesExOrdinal = 108;
const unsigned short kMicrosoftVendor = 0x045E;
const unsigned short kXbox360Products[] = {0x028E, 0x028F, 0x0291, 0x02A0, 0x02A1, 0x0719};
const WORD kXInputGuide = 0x0400;
const float kTriggerRange = 255.0f;
const float kXInputStickRange = 32767.0f;
const float kHidStickCentre = 127.5f;

typedef DWORD(WINAPI* XInputGetStateProc)(DWORD, XINPUT_STATE*);

struct XInputCapabilitiesEx
{
    XINPUT_CAPABILITIES capabilities;
    WORD vendorId;
    WORD productId;
    WORD versionNumber;
    WORD unknown1;
    DWORD unknown2;
};

typedef DWORD(WINAPI* XInputGetCapabilitiesExProc)(DWORD, DWORD, DWORD, XInputCapabilitiesEx*);

struct HidPad
{
    HANDLE handle;
    unsigned int id;
    bool dualSense;
    bool dualShock3;
    bool bluetooth;
    bool setLeds;
    int player;
    int ledPlayer;
    DWORD outputLength;
    bool alive;
    DWORD reportLength;
    PcOsGamepadState state;
};

CRITICAL_SECTION s_lock;
bool s_started = false;
std::vector<HidPad*> s_hidPads;
std::set<std::string> s_knownPaths;
unsigned int s_nextHidId = kHidIdBase;
XInputGetStateProc s_xinputGetState = 0;
XInputGetCapabilitiesExProc s_xinputGetCapabilitiesEx = 0;
bool s_xinputConnected[XUSER_MAX_COUNT];
PcOsGamepadKind s_xinputKind[XUSER_MAX_COUNT];
unsigned long long s_xinputNextProbe[XUSER_MAX_COUNT];

float Clamp(float value, float minimum, float maximum)
{
    return value < minimum ? minimum : (value > maximum ? maximum : value);
}

float HidAxis(unsigned char value)
{
    return Clamp((static_cast<float>(value) - kHidStickCentre) / kHidStickCentre, -1.0f, 1.0f);
}

float XInputAxis(SHORT value)
{
    return Clamp(static_cast<float>(value) / kXInputStickRange, -1.0f, 1.0f);
}

bool Contains(const unsigned short* values, size_t count, unsigned short value)
{
    for (size_t index = 0; index < count; ++index)
    {
        if (values[index] == value)
            return true;
    }
    return false;
}

bool IsXInputPath(const std::string& path)
{
    for (size_t index = 0; index + 3 <= path.size(); ++index)
    {
        if ((path[index] == 'I' || path[index] == 'i') && (path[index + 1] == 'G' || path[index + 1] == 'g')
            && path[index + 2] == '_')
            return true;
    }
    return false;
}

void ParseDualShock3Report(HidPad* pad, const unsigned char* report, DWORD size)
{
    if (report[0] != 0x01 || size < kDualShock3ReportLength)
        return;
    static const struct
    {
        unsigned char byte;
        unsigned char mask;
        unsigned int button;
    } kButtons[] = {
        {2, 0x01, ePcOsGamepad_Back},          {2, 0x02, ePcOsGamepad_LeftStick},
        {2, 0x04, ePcOsGamepad_RightStick},    {2, 0x08, ePcOsGamepad_Start},
        {2, 0x10, ePcOsGamepad_Up},            {2, 0x20, ePcOsGamepad_Right},
        {2, 0x40, ePcOsGamepad_Down},          {2, 0x80, ePcOsGamepad_Left},
        {3, 0x04, ePcOsGamepad_LeftShoulder},  {3, 0x08, ePcOsGamepad_RightShoulder},
        {3, 0x10, ePcOsGamepad_North},         {3, 0x20, ePcOsGamepad_East},
        {3, 0x40, ePcOsGamepad_South},         {3, 0x80, ePcOsGamepad_West},
        {4, 0x01, ePcOsGamepad_Guide},
    };
    unsigned int buttons = 0;
    for (size_t index = 0; index < sizeof(kButtons) / sizeof(kButtons[0]); ++index)
    {
        if (report[kButtons[index].byte] & kButtons[index].mask)
            buttons |= kButtons[index].button;
    }

    EnterCriticalSection(&s_lock);
    pad->state.buttons = buttons;
    pad->state.leftX = HidAxis(report[6]);
    pad->state.leftY = -HidAxis(report[7]);
    pad->state.rightX = HidAxis(report[8]);
    pad->state.rightY = -HidAxis(report[9]);
    pad->state.leftTrigger = static_cast<float>(report[18]) / kTriggerRange;
    pad->state.rightTrigger = static_cast<float>(report[19]) / kTriggerRange;
    LeaveCriticalSection(&s_lock);
}

void ParseSonyReport(HidPad* pad, const unsigned char* report, DWORD size)
{
    if (pad->dualShock3)
    {
        ParseDualShock3Report(pad, report, size);
        return;
    }
    const unsigned char* data = 0;
    bool dualSenseLayout = false;
    if (pad->dualSense)
    {
        if (report[0] == 0x01 && !pad->bluetooth && size >= 11)
        {
            data = report + 1;
            dualSenseLayout = true;
        }
        else if (report[0] == 0x31 && size >= 12)
        {
            data = report + 2;
            dualSenseLayout = true;
        }
        else if (report[0] == 0x01 && size >= 10)
            data = report + 1;
    }
    else if (report[0] == 0x01 && size >= 10)
        data = report + 1;
    else if (report[0] == 0x11 && size >= 12)
        data = report + 3;
    if (data == 0)
        return;

    unsigned char face = dualSenseLayout ? data[7] : data[4];
    unsigned char shoulder = dualSenseLayout ? data[8] : data[5];
    unsigned char system = dualSenseLayout ? data[9] : data[6];
    unsigned char leftTrigger = dualSenseLayout ? data[4] : data[7];
    unsigned char rightTrigger = dualSenseLayout ? data[5] : data[8];

    static const unsigned int kHat[] = {ePcOsGamepad_Up,    ePcOsGamepad_Up | ePcOsGamepad_Right,
                                        ePcOsGamepad_Right, ePcOsGamepad_Down | ePcOsGamepad_Right,
                                        ePcOsGamepad_Down,  ePcOsGamepad_Down | ePcOsGamepad_Left,
                                        ePcOsGamepad_Left,  ePcOsGamepad_Up | ePcOsGamepad_Left};
    unsigned int buttons = 0;
    if ((face & 0x0F) < 8)
        buttons |= kHat[face & 0x0F];
    if (face & 0x10)
        buttons |= ePcOsGamepad_West;
    if (face & 0x20)
        buttons |= ePcOsGamepad_South;
    if (face & 0x40)
        buttons |= ePcOsGamepad_East;
    if (face & 0x80)
        buttons |= ePcOsGamepad_North;
    if (shoulder & 0x01)
        buttons |= ePcOsGamepad_LeftShoulder;
    if (shoulder & 0x02)
        buttons |= ePcOsGamepad_RightShoulder;
    if (shoulder & 0x10)
        buttons |= ePcOsGamepad_Back;
    if (shoulder & 0x20)
        buttons |= ePcOsGamepad_Start;
    if (shoulder & 0x40)
        buttons |= ePcOsGamepad_LeftStick;
    if (shoulder & 0x80)
        buttons |= ePcOsGamepad_RightStick;
    if (system & 0x01)
        buttons |= ePcOsGamepad_Guide;
    if (system & 0x02)
        buttons |= ePcOsGamepad_Touchpad;

    EnterCriticalSection(&s_lock);
    pad->state.buttons = buttons;
    pad->state.leftX = HidAxis(data[0]);
    pad->state.leftY = -HidAxis(data[1]);
    pad->state.rightX = HidAxis(data[2]);
    pad->state.rightY = -HidAxis(data[3]);
    pad->state.leftTrigger = static_cast<float>(leftTrigger) / kTriggerRange;
    pad->state.rightTrigger = static_cast<float>(rightTrigger) / kTriggerRange;
    LeaveCriticalSection(&s_lock);
}

bool RunningOnWine()
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    return ntdll != 0 && GetProcAddress(ntdll, "wine_get_version") != 0;
}

void SendDualShock3Leds(HidPad* pad, int player)
{
    if (pad->outputLength < sizeof(kDualShock3Output))
        return;
    std::vector<unsigned char> report(pad->outputLength, 0);
    memcpy(&report[0], kDualShock3Output, sizeof(kDualShock3Output));
    int index = player - 1;
    if (index >= static_cast<int>(sizeof(kDualShock3PlayerLeds)))
        index = sizeof(kDualShock3PlayerLeds) - 1;
    report[kDualShock3LedByte] = kDualShock3PlayerLeds[index];
    if (!HidD_SetOutputReport(pad->handle, &report[0], static_cast<ULONG>(report.size())))
    {
        DWORD written = 0;
        WriteFile(pad->handle, &report[0], static_cast<DWORD>(report.size()), &written, 0);
    }
}

DWORD WINAPI ReadHidPad(void* parameter)
{
    HidPad* pad = static_cast<HidPad*>(parameter);
    std::vector<unsigned char> report(pad->reportLength);
    for (;;)
    {
        DWORD read = 0;
        if (!ReadFile(pad->handle, &report[0], pad->reportLength, &read, 0) || read == 0)
            break;
        ParseSonyReport(pad, &report[0], read);
        if (pad->setLeds)
        {
            EnterCriticalSection(&s_lock);
            int player = pad->player;
            LeaveCriticalSection(&s_lock);
            if (player > 0 && player != pad->ledPlayer)
            {
                SendDualShock3Leds(pad, player);
                pad->ledPlayer = player;
            }
        }
    }
    CloseHandle(pad->handle);
    EnterCriticalSection(&s_lock);
    pad->alive = false;
    LeaveCriticalSection(&s_lock);
    return 0;
}

void OpenHidPad(const std::string& path)
{
    HANDLE probe = CreateFileA(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (probe == INVALID_HANDLE_VALUE)
        return;
    HIDD_ATTRIBUTES attributes;
    attributes.Size = sizeof(attributes);
    BOOL haveAttributes = HidD_GetAttributes(probe, &attributes);
    CloseHandle(probe);
    if (!haveAttributes || attributes.VendorID != kSonyVendor)
        return;
    bool dualSense = Contains(kDualSenseProducts, sizeof(kDualSenseProducts) / sizeof(kDualSenseProducts[0]),
                              attributes.ProductID);
    bool dualShock3 = attributes.ProductID == kDualShock3Product;
    if (!dualSense && !dualShock3
        && !Contains(kDualShock4Products, sizeof(kDualShock4Products) / sizeof(kDualShock4Products[0]),
                     attributes.ProductID))
        return;

    HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                OPEN_EXISTING, 0, 0);
    if (handle == INVALID_HANDLE_VALUE)
        handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (handle == INVALID_HANDLE_VALUE)
        return;
    PHIDP_PREPARSED_DATA preparsed = 0;
    HIDP_CAPS caps;
    memset(&caps, 0, sizeof(caps));
    if (HidD_GetPreparsedData(handle, &preparsed))
    {
        HidP_GetCaps(preparsed, &caps);
        HidD_FreePreparsedData(preparsed);
    }
    if (caps.UsagePage != kHidUsagePageGeneric
        || (caps.Usage != kHidUsageGamepad && !(dualShock3 && caps.Usage == kHidUsageJoystick))
        || caps.InputReportByteLength < (dualShock3 ? kDualShock3ReportLength : 10))
    {
        CloseHandle(handle);
        return;
    }

    HidPad* pad = new HidPad;
    memset(&pad->state, 0, sizeof(pad->state));
    pad->handle = handle;
    pad->dualSense = dualSense;
    pad->dualShock3 = dualShock3;
    pad->bluetooth = !dualShock3 && caps.InputReportByteLength != kUsbReportLength;
    pad->setLeds = dualShock3 && !RunningOnWine();
    pad->player = 0;
    pad->ledPlayer = 0;
    pad->outputLength = caps.OutputReportByteLength;
    pad->alive = true;
    pad->reportLength = caps.InputReportByteLength;
    if (pad->bluetooth && caps.FeatureReportByteLength > 0)
    {
        std::vector<unsigned char> feature(caps.FeatureReportByteLength);
        feature[0] = dualSense ? kDualSenseCalibrationReport : kDualShock4CalibrationReport;
        HidD_GetFeature(handle, &feature[0], static_cast<ULONG>(feature.size()));
    }
    if (dualShock3 && caps.FeatureReportByteLength > 0)
    {
        std::vector<unsigned char> feature(caps.FeatureReportByteLength);
        for (size_t index = 0; index < sizeof(kDualShock3EnableReports); ++index)
        {
            memset(&feature[0], 0, feature.size());
            feature[0] = kDualShock3EnableReports[index];
            HidD_GetFeature(handle, &feature[0], static_cast<ULONG>(feature.size()));
        }
    }
    pad->state.kind = dualShock3 ? ePcOsGamepadKind_PS3 : ePcOsGamepadKind_PS4;
    if (dualShock3)
        snprintf(pad->state.name, sizeof(pad->state.name), "DualShock 3");
    else
        snprintf(pad->state.name, sizeof(pad->state.name), "%s (%s)", dualSense ? "DualSense" : "DualShock 4",
                 pad->bluetooth ? "Bluetooth" : "USB");

    EnterCriticalSection(&s_lock);
    pad->id = s_nextHidId++;
    pad->state.id = pad->id;
    s_hidPads.push_back(pad);
    LeaveCriticalSection(&s_lock);

    HANDLE thread = CreateThread(0, 0, ReadHidPad, pad, 0, 0);
    if (thread != 0)
        CloseHandle(thread);
}

void ScanHidDevices()
{
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO info = SetupDiGetClassDevsA(&hidGuid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (info == INVALID_HANDLE_VALUE)
        return;
    std::set<std::string> present;
    SP_DEVICE_INTERFACE_DATA deviceInterface;
    deviceInterface.cbSize = sizeof(deviceInterface);
    for (DWORD index = 0; SetupDiEnumDeviceInterfaces(info, 0, &hidGuid, index, &deviceInterface); ++index)
    {
        DWORD size = 0;
        SetupDiGetDeviceInterfaceDetailA(info, &deviceInterface, 0, 0, &size, 0);
        if (size < sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A))
            continue;
        std::vector<char> buffer(size);
        SP_DEVICE_INTERFACE_DETAIL_DATA_A* detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_A*>(&buffer[0]);
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
        if (!SetupDiGetDeviceInterfaceDetailA(info, &deviceInterface, detail, size, 0, 0))
            continue;
        std::string path(detail->DevicePath);
        present.insert(path);
        if (s_knownPaths.count(path) != 0 || IsXInputPath(path))
            continue;
        s_knownPaths.insert(path);
        OpenHidPad(path);
    }
    SetupDiDestroyDeviceInfoList(info);
    for (std::set<std::string>::iterator it = s_knownPaths.begin(); it != s_knownPaths.end();)
    {
        if (present.count(*it) == 0)
            s_knownPaths.erase(it++);
        else
            ++it;
    }
}

DWORD WINAPI ScanHidDevicesLoop(void*)
{
    for (;;)
    {
        ScanHidDevices();
        Sleep(kRescanMilliseconds);
    }
    return 0;
}

void LoadXInput()
{
    static const char* const kLibraries[] = {"xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll"};
    for (size_t index = 0; index < sizeof(kLibraries) / sizeof(kLibraries[0]); ++index)
    {
        HMODULE module = LoadLibraryA(kLibraries[index]);
        if (module == 0)
            continue;
        s_xinputGetState = reinterpret_cast<XInputGetStateProc>(
            GetProcAddress(module, reinterpret_cast<LPCSTR>(static_cast<ULONG_PTR>(kXInputGetStateExOrdinal))));
        if (s_xinputGetState == 0)
            s_xinputGetState = reinterpret_cast<XInputGetStateProc>(GetProcAddress(module, "XInputGetState"));
        if (s_xinputGetState == 0)
            continue;
        s_xinputGetCapabilitiesEx = reinterpret_cast<XInputGetCapabilitiesExProc>(GetProcAddress(
            module, reinterpret_cast<LPCSTR>(static_cast<ULONG_PTR>(kXInputGetCapabilitiesExOrdinal))));
        return;
    }
}

void Start()
{
    if (s_started)
        return;
    s_started = true;
    InitializeCriticalSection(&s_lock);
    LoadXInput();
    HANDLE thread = CreateThread(0, 0, ScanHidDevicesLoop, 0, 0, 0);
    if (thread != 0)
        CloseHandle(thread);
}

PcOsGamepadKind XInputKind(DWORD slot)
{
    if (s_xinputGetCapabilitiesEx == 0)
        return ePcOsGamepadKind_XboxOne;
    XInputCapabilitiesEx capabilities;
    memset(&capabilities, 0, sizeof(capabilities));
    if (s_xinputGetCapabilitiesEx(1, slot, 0, &capabilities) != ERROR_SUCCESS)
        return ePcOsGamepadKind_XboxOne;
    if (capabilities.vendorId == kMicrosoftVendor
        && Contains(kXbox360Products, sizeof(kXbox360Products) / sizeof(kXbox360Products[0]), capabilities.productId))
        return ePcOsGamepadKind_Xbox360;
    if (capabilities.vendorId == kSonyVendor)
        return capabilities.productId == kDualShock3Product ? ePcOsGamepadKind_PS3 : ePcOsGamepadKind_PS4;
    return ePcOsGamepadKind_XboxOne;
}

bool PollXInput(DWORD slot, unsigned long long now, PcOsGamepadState* state)
{
    if (s_xinputGetState == 0 || (!s_xinputConnected[slot] && now < s_xinputNextProbe[slot]))
        return false;
    XINPUT_STATE input;
    memset(&input, 0, sizeof(input));
    if (s_xinputGetState(slot, &input) != ERROR_SUCCESS)
    {
        s_xinputConnected[slot] = false;
        s_xinputNextProbe[slot] = now + kXInputProbeMilliseconds;
        return false;
    }
    if (!s_xinputConnected[slot])
        s_xinputKind[slot] = XInputKind(slot);
    s_xinputConnected[slot] = true;

    static const struct
    {
        WORD source;
        unsigned int destination;
    } kButtons[] = {
        {XINPUT_GAMEPAD_A, ePcOsGamepad_South},
        {XINPUT_GAMEPAD_B, ePcOsGamepad_East},
        {XINPUT_GAMEPAD_X, ePcOsGamepad_West},
        {XINPUT_GAMEPAD_Y, ePcOsGamepad_North},
        {XINPUT_GAMEPAD_LEFT_SHOULDER, ePcOsGamepad_LeftShoulder},
        {XINPUT_GAMEPAD_RIGHT_SHOULDER, ePcOsGamepad_RightShoulder},
        {XINPUT_GAMEPAD_LEFT_THUMB, ePcOsGamepad_LeftStick},
        {XINPUT_GAMEPAD_RIGHT_THUMB, ePcOsGamepad_RightStick},
        {XINPUT_GAMEPAD_BACK, ePcOsGamepad_Back},
        {XINPUT_GAMEPAD_START, ePcOsGamepad_Start},
        {kXInputGuide, ePcOsGamepad_Guide},
        {XINPUT_GAMEPAD_DPAD_UP, ePcOsGamepad_Up},
        {XINPUT_GAMEPAD_DPAD_DOWN, ePcOsGamepad_Down},
        {XINPUT_GAMEPAD_DPAD_LEFT, ePcOsGamepad_Left},
        {XINPUT_GAMEPAD_DPAD_RIGHT, ePcOsGamepad_Right},
    };
    memset(state, 0, sizeof(*state));
    state->id = kXInputIdBase + slot;
    for (size_t index = 0; index < sizeof(kButtons) / sizeof(kButtons[0]); ++index)
    {
        if (input.Gamepad.wButtons & kButtons[index].source)
            state->buttons |= kButtons[index].destination;
    }
    state->leftX = XInputAxis(input.Gamepad.sThumbLX);
    state->leftY = XInputAxis(input.Gamepad.sThumbLY);
    state->rightX = XInputAxis(input.Gamepad.sThumbRX);
    state->rightY = XInputAxis(input.Gamepad.sThumbRY);
    state->leftTrigger = static_cast<float>(input.Gamepad.bLeftTrigger) / kTriggerRange;
    state->rightTrigger = static_cast<float>(input.Gamepad.bRightTrigger) / kTriggerRange;
    state->kind = s_xinputKind[slot];
    static const char* const kKindNames[] = {"Xbox One", "Xbox 360", "PS4", "PS3"};
    snprintf(state->name, sizeof(state->name), "%s controller %lu (XInput)", kKindNames[state->kind],
             static_cast<unsigned long>(slot + 1));
    return true;
}
}

size_t PcOsGamepadPoll(PcOsGamepadState* states, size_t capacity)
{
    Start();
    size_t count = 0;
    unsigned long long now = GetTickCount64();
    for (DWORD slot = 0; slot < XUSER_MAX_COUNT && count < capacity; ++slot)
    {
        if (PollXInput(slot, now, &states[count]))
            ++count;
    }

    EnterCriticalSection(&s_lock);
    for (size_t index = 0; index < s_hidPads.size();)
    {
        HidPad* pad = s_hidPads[index];
        if (!pad->alive)
        {
            delete pad;
            s_hidPads.erase(s_hidPads.begin() + index);
            continue;
        }
        if (count < capacity)
            states[count++] = pad->state;
        ++index;
    }
    LeaveCriticalSection(&s_lock);
    return count;
}

void PcOsGamepadSetPlayer(unsigned int id, int player)
{
    Start();
    EnterCriticalSection(&s_lock);
    for (size_t index = 0; index < s_hidPads.size(); ++index)
    {
        if (s_hidPads[index]->id == id)
            s_hidPads[index]->player = player;
    }
    LeaveCriticalSection(&s_lock);
}

#endif
