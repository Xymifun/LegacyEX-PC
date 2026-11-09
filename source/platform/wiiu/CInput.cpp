#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/XLanguage.hpp"
#include "game/util/Mth.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <new>

#include <cafe/sci.h>
#include <coreinit/filesystem.h>
#include <coreinit/messagequeue.h>
#include <coreinit/systeminfo.h>
#include <coreinit/userconfig.h>
#include <nn/erreula.h>
#include <nn/swkbd.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <vpad/input.h>

extern "C" int SCIGetCafeLanguage(unsigned int* language);
int XEnableGuestSignin(int enable)
{
    (void)enable;
    return 0;
}

extern "C" int OSResumeThread(OSThread* thread);
extern "C" void KPADReset(void);

static void ConnectCallbackAdapter(WPADChan channel, WPADError result)
{
    CInput::ConnectCallback(static_cast<int>(channel), static_cast<int>(result));
}

extern "C" int _SCIReadSysConfig(const char* key, unsigned int type, unsigned int size, void* value);
int SCIGetSystemSettings(void* settings);

enum
{
    SCI_CAFE_LANGUAGE_JAPANESE = 0,
    SCI_CAFE_LANGUAGE_ENGLISH = 1,
    SCI_CAFE_LANGUAGE_FRENCH = 2,
    SCI_CAFE_LANGUAGE_GERMAN = 3,
    SCI_CAFE_LANGUAGE_ITALIAN = 4,
    SCI_CAFE_LANGUAGE_SPANISH = 5,
    SCI_CAFE_LANGUAGE_SIMPLIFIED_CHINESE = 6,
    SCI_CAFE_LANGUAGE_KOREAN = 7,
    SCI_CAFE_LANGUAGE_DUTCH = 8,
    SCI_CAFE_LANGUAGE_PORTUGUESE = 9,
    SCI_CAFE_LANGUAGE_RUSSIAN = 10,
    SCI_CAFE_LANGUAGE_TRADITIONAL_CHINESE = 11
};

enum
{
    SCI_PLATFORM_REGION_JPN = 1,
    SCI_PLATFORM_REGION_USA = 2,
    SCI_PLATFORM_REGION_EUR = 4,
    SCI_PLATFORM_REGION_CHN = 0x10,
    SCI_PLATFORM_REGION_KOR = 0x20,
    SCI_PLATFORM_REGION_TWN = 0x40
};

enum
{
    KEYBOARD_FLAG_QWERTY_US_ENG = 1 << 1,
    KEYBOARD_FLAG_QWERTY_US_FRA = 1 << 2,
    KEYBOARD_FLAG_QWERTY_US_SPA = 1 << 3,
    KEYBOARD_FLAG_QWERTY_EU_ENG = 1 << 5,
    KEYBOARD_FLAG_QWERTY_EU_FRA = 1 << 6,
    KEYBOARD_FLAG_QWERTY_EU_DEU = 1 << 7,
    KEYBOARD_FLAG_QWERTY_EU_ITA = 1 << 8,
    KEYBOARD_FLAG_QWERTY_EU_SPA = 1 << 9,
    KEYBOARD_FLAG_QWERTY_EU_NLD = 1 << 10,
    KEYBOARD_FLAG_QWERTY_EU_POR = 1 << 11,
    KEYBOARD_FLAG_ALL = (1 << 19) - 1
};

enum
{
    TOUCH_INPUT_BUTTONS = 0x00c03cff
};

double multiBreak(double first, double second, double third)
{
    if (first < second)
    {
        return second;
    }
    if (static_cast<float>(first - third) < 0.0f)
    {
        third = first;
    }
    return third;
}

extern "C" int VerifyLanguageRange(int* language)
{
    if (*language < 12 && static_cast<signed char>(*language) >= 0)
    {
        return 1;
    }
    *language = 0xff;
    return 0;
}

int VerifyCountryRange(unsigned int* country)
{
    int result = 1;
    if (*country < 0x100)
    {
        if (static_cast<int>(*country) < 0)
        {
            result = 0;
            *country = 0;
        }
    }
    else
    {
        result = 0;
        *country = 0xff;
    }
    return result;
}

extern "C" int SCIGetCafeLanguageCode(unsigned int* language)
{
    return _SCIReadSysConfig("cafe.language", 3, 4, language);
}

extern "C" int SCIGetCafeLanguage(unsigned int* language)
{
    int result = SCIGetCafeLanguageCode(language);
    VerifyLanguageRange(reinterpret_cast<int*>(language));
    if (result == 1)
    {
        if (*language == 0xff)
        {
            unsigned int platformRegion;
            result = SCIGetPlatformRegion(&platformRegion);
            if (result != 1)
            {
                *language = 1;
                return result;
            }
            if (platformRegion == SCI_PLATFORM_REGION_JPN)
            {
                *language = SCI_CAFE_LANGUAGE_JAPANESE;
            }
            else if (platformRegion == SCI_PLATFORM_REGION_CHN)
            {
                *language = SCI_CAFE_LANGUAGE_SIMPLIFIED_CHINESE;
            }
            else if (platformRegion == SCI_PLATFORM_REGION_KOR)
            {
                *language = SCI_CAFE_LANGUAGE_KOREAN;
            }
            else if (platformRegion == SCI_PLATFORM_REGION_TWN)
            {
                *language = SCI_CAFE_LANGUAGE_TRADITIONAL_CHINESE;
            }
            else
            {
                *language = SCI_CAFE_LANGUAGE_ENGLISH;
            }
        }
        if (result == 1)
        {
            return 1;
        }
    }
    *language = 1;
    return result;
}

extern "C" int SCIGetPlatformRegion(unsigned int* region)
{
    unsigned char settings[0x48] = {};
    int result = SCIGetSystemSettings(settings);
    *region = settings[3];
    return result;
}

static KPADError s_connectionResults[4];
static KPADStatus s_kpadStatus[4][16];
static VPADStatus s_vpadStatus[16];
static int s_vpadSampleCount;
static VPADReadError s_vpadReadError;
static unsigned char s_fontStack[0x20000];
static OSThread s_fontThread;
static unsigned char s_predictStack[0x20000];
static OSThread s_predictThread;
static OSMessage s_fontMessage;
static OSMessage s_predictMessage;
static OSMessageQueue s_fontQueue;
static OSMessageQueue s_predictQueue;
CInput* g_pInputInstance = 0;
unsigned int g_inputPlatformFlag = 0;

static unsigned int s_xLanguage = 0xff;
static unsigned int s_xLocale = 0xff;
static const char* s_errorViewerJumpArgument = "my app";

extern "C" int _SCIReadSysConfig(const char* key, unsigned int type, unsigned int size, void* value)
{
    UCHandle handle = UCOpen();
    if (handle < 0)
        return 0;

    UCSysConfig config;
    std::memset(&config, 0, sizeof(config));
    if (key != 0)
        std::strncpy(config.name, key, sizeof(config.name) - 1);
    config.access = 1911;
    config.dataType = static_cast<UCDataType>(type);
    config.dataSize = size;
    config.data = value;
    UCError error = UCReadSysConfig(handle, 1, &config);
    UCClose(handle);
    if (error == 0)
        return 1;
    if (error == -0x200009)
        return -4;
    if (error == -0x200015)
        return -3;
    return -1;
}

unsigned int XGetLanguage()
{
    unsigned int language = GameSettings::GetMinecraftLanguage(0);
    if (language != 0)
        return language;
    if (s_xLanguage == 0xff && SCIGetCafeLanguage(&s_xLanguage) != 1)
        s_xLanguage = 0xff;
    switch (s_xLanguage)
    {
    case SCI_CAFE_LANGUAGE_JAPANESE:
        return XC_LANGUAGE_JAPANESE;
    case SCI_CAFE_LANGUAGE_ENGLISH:
        return XC_LANGUAGE_ENGLISH;
    case SCI_CAFE_LANGUAGE_FRENCH:
        return XC_LANGUAGE_FRENCH;
    case SCI_CAFE_LANGUAGE_GERMAN:
        return XC_LANGUAGE_GERMAN;
    case SCI_CAFE_LANGUAGE_ITALIAN:
        return XC_LANGUAGE_ITALIAN;
    case SCI_CAFE_LANGUAGE_SPANISH:
        return XC_LANGUAGE_SPANISH;
    case SCI_CAFE_LANGUAGE_SIMPLIFIED_CHINESE:
        return XC_LANGUAGE_ZH_HANS;
    case SCI_CAFE_LANGUAGE_KOREAN:
        return XC_LANGUAGE_KOREAN;
    case SCI_CAFE_LANGUAGE_DUTCH:
        return XC_LANGUAGE_DUTCH;
    case SCI_CAFE_LANGUAGE_PORTUGUESE:
        return XC_LANGUAGE_PORTUGUESE;
    case SCI_CAFE_LANGUAGE_RUSSIAN:
        return XC_LANGUAGE_RUSSIAN;
    case SCI_CAFE_LANGUAGE_TRADITIONAL_CHINESE:
        return XC_LANGUAGE_TCHINESE;
    default:
        return XC_LANGUAGE_ENGLISH;
    }
}

int XGetLocale()
{
    int minecraftLocale = GameSettings::GetMinecraftLocale(0);
    if (minecraftLocale != 0)
        return minecraftLocale;
    if (s_xLocale == 0xff && SCIGetCafeLanguage(&s_xLocale) != 1)
        s_xLocale = 0xff;
    switch (s_xLocale)
    {
    case SCI_CAFE_LANGUAGE_JAPANESE:
        return XC_LOCALE_JAPAN;
    case SCI_CAFE_LANGUAGE_FRENCH:
        return gApp.IsAmericanSKU() ? XC_LOCALE_CANADA : XC_LOCALE_FRANCE;
    case SCI_CAFE_LANGUAGE_GERMAN:
        return XC_LOCALE_GERMANY;
    case SCI_CAFE_LANGUAGE_ITALIAN:
        return XC_LOCALE_ITALY;
    case SCI_CAFE_LANGUAGE_SPANISH:
        return gApp.IsAmericanSKU() ? XC_LOCALE_MEXICO : XC_LOCALE_SPAIN;
    case SCI_CAFE_LANGUAGE_SIMPLIFIED_CHINESE:
        return XC_LOCALE_CHINA;
    case SCI_CAFE_LANGUAGE_KOREAN:
        return XC_LOCALE_KOREA;
    case SCI_CAFE_LANGUAGE_DUTCH:
        return XC_LOCALE_NETHERLANDS;
    case SCI_CAFE_LANGUAGE_PORTUGUESE:
        return gApp.IsAmericanSKU() ? XC_LOCALE_BRAZIL : XC_LOCALE_PORTUGAL;
    case SCI_CAFE_LANGUAGE_RUSSIAN:
        return XC_LOCALE_RUSSIAN_FEDERATION;
    case SCI_CAFE_LANGUAGE_TRADITIONAL_CHINESE:
        return XC_LOCALE_TAIWAN;
    default:
        return XC_LOCALE_UNITED_STATES;
    }
}

static inline int GetStickValue(float value)
{
    return static_cast<short>(static_cast<int>(fclamp(value, -1.0f, 1.0f) * 32767.0f));
}

static inline int ApplyDeadzone(int value, int deadzone)
{
    if (value > deadzone)
    {
        return value - deadzone;
    }
    if (value < -deadzone)
    {
        return value + deadzone;
    }
    return 0;
}

static inline float NormaliseStick(int value, float range)
{
    float result = static_cast<float>(value) / range;
    if (result > -0.15f && result < 0.15f)
    {
        return 0.0f;
    }
    return result;
}

static inline long long GetTimeValue(const CInput::TIME_VALUE& value)
{
    return static_cast<long long>(value.high) << 32 | value.low;
}

static inline void SetTimeValue(CInput::TIME_VALUE& value, long long time)
{
    value.high = static_cast<unsigned int>(time >> 32);
    value.low = static_cast<unsigned int>(time);
}

CInput* g_pInput = new (std::nothrow) CInput;

CInput::CInput()
{
    g_pInputInstance = this;
    joypadMapsReady = 0;
    menuDisplayed[0] = 0;
    menuDisplayed[1] = 0;
    menuDisplayed[2] = 0;
    menuDisplayed[3] = 0;
    InitTime();
    debugSequence = 0;
    debugSequenceLength = 0;
    debugSequenceCallback = 0;
    inputTick = 0;
    initialRepeatDelay = 0.5f;
    repeatDelay = 0.8f;
    circleCrossSwapped = 0;
    for (int pad = 0; pad < 4; ++pad)
        padRemap[pad] = -1;
    KeyboardOnCreate();
    ErrorViewerOnCreate();
}
void CInput::InitTime()
{
    for (int pad = 0; pad < 5; ++pad)
    {
        idleStartTimes[pad].high = 0;
        idleStartTimes[pad].low = 0;
    }
    timeScale = 1.0f / static_cast<float>(OSGetSystemInfo()->busClockSpeed >> 2);
}
void CInput::ConnectCallback(int channel, int result)
{
    if (result != 0)
    {
        s_connectionResults[channel] = static_cast<KPADError>(0xFD);
    }
    else if (g_inputPlatformFlag != 0)
    {
        KPADEnableMpls(static_cast<KPADChan>(channel), static_cast<KPADMplsMode>(3));
    }
    else
    {
        WPADControlDpd(static_cast<WPADChan>(channel), WPAD_DPD_FMT_NONE, 0);
    }
}
void CInput::Initialise(int count, unsigned char gameMapCount, unsigned char alternateMapCount)
{
    VPADInit();
    WPADEnableURCC(1);
    KPADInit();
    KPADReset();
    std::memset(joypads, 0, sizeof(joypads));
    for (int channel = 0; channel < 4; ++channel)
    {
        KPADSetConnectCallback(static_cast<KPADChan>(channel), ConnectCallbackAdapter);
        s_connectionResults[channel] = static_cast<KPADError>(0xFD);
    }

    controllerCount = 0;
    deadzone = 10000;
    maximumInput = 10000;
    movementRange = 0x7fff;
    movementSpan = 0x58ef;
    movementSpanFloat = 22767.0f;
    controllerOptions = 0;
    for (int pad = 0; pad < 5; ++pad)
    {
        joypads[pad].connected = 0;
        joypads[pad].sensitivity = 1.0f;
        joypads[pad].statusIndex = 0xff;
        joypads[pad].controllerId = -1;
        joypads[pad].lockedController = -1;
        joypads[pad].leftXSource = &joypads[pad].rightX;
        joypads[pad].leftYSource = &joypads[pad].rightY;
        joypads[pad].rightXSource = &joypads[pad].leftX;
        joypads[pad].rightYSource = &joypads[pad].leftY;
        joypads[pad].leftTriggerSource = &joypads[pad].hasTouch;
        joypads[pad].rightTriggerSource = &joypads[pad].touchPressed;
        if (pad == 0)
        {
            joypads[pad].statusStorage.vpad.statuses = new VPADStatus[count];
        }
        else
        {
            joypads[pad].statusStorage.kpad.statuses = new KPADStatus[count];
        }
        gameJoypadMaps[pad] = new unsigned int[gameMapCount];
        alternateJoypadMaps[pad] = new unsigned int[gameMapCount];
        for (unsigned int map = 0; map < gameMapCount; ++map)
        {
            gameJoypadMaps[pad][map] = 0;
        }
        for (unsigned int map = 0; map < gameMapCount; ++map)
        {
            alternateJoypadMaps[pad][map] = 0;
        }
    }
    controllerConfiguration[0] = static_cast<unsigned char>(count);
    controllerConfiguration[1] = gameMapCount;
    controllerConfiguration[2] = alternateMapCount;
    joypadMapsReady = 0;
    for (int pad = 0; pad < 4; ++pad)
    {
        padRemap[pad] = -1;
    }
    InitialiseErrorViewer();
    InitialiseKeyboard();
}
void CInput::ReadAllPads()
{
    s_vpadSampleCount = VPADRead(static_cast<VPADChan>(0), s_vpadStatus, 16, &s_vpadReadError);
    if (s_vpadReadError == VPAD_READ_SUCCESS || s_vpadReadError == VPAD_READ_NO_SAMPLES)
    {
        VPADGetTPCalibratedPoint(static_cast<VPADChan>(0), &s_vpadStatus[0].tpNormal, &s_vpadStatus[0].tpNormal);
    }
    else
    {
        std::memset(s_vpadStatus, 0, sizeof(VPADStatus));
    }

    KPADError readError;
    for (int channel = 0; channel < 4; ++channel)
    {
        KPADReadEx(static_cast<KPADChan>(channel), s_kpadStatus[channel], 16, &readError);
    }

    JOYPAD& drc = joypads[0];
    drc.previousSampleIndex = drc.sampleIndex;
    drc.sampleIndex = 0;
    drc.statusIndex = static_cast<unsigned char>((drc.statusIndex + 1) % controllerConfiguration[0]);
    static_cast<VPADStatus*>(drc.statusStorage.vpad.statuses)[drc.statusIndex] = s_vpadStatus[0];
    bool drcConnected = s_vpadReadError == VPAD_READ_SUCCESS || s_vpadReadError == VPAD_READ_NO_SAMPLES;
    if (!drc.connected)
    {
        std::memset(&static_cast<VPADStatus*>(drc.statusStorage.vpad.statuses)[drc.statusIndex], 0, sizeof(VPADStatus));
    }
    drc.setConnected(drcConnected);

    bool preserveConnections = inputTick == 0;
    if (!preserveConnections)
    {
        --inputTick;
    }
    for (int channel = 0; channel < 4; ++channel)
    {
        JOYPAD& joypad = joypads[channel + 1];
        joypad.previousSampleIndex = joypad.sampleIndex;
        joypad.sampleIndex = 0;
        joypad.statusIndex = static_cast<unsigned char>((joypad.statusIndex + 1) % controllerConfiguration[0]);
        KPADStatus& status = static_cast<KPADStatus*>(joypad.statusStorage.kpad.statuses)[joypad.statusIndex];
        status = s_kpadStatus[channel][0];
        bool supported = status.extensionType == 2 || status.extensionType == 7 || status.extensionType == 0x1f;
        bool connected = supported && (status.error == 0 || status.error == -2);
        if (!preserveConnections
            && (status.extensionType == 0xff || (status.extensionType == 0x1f && status.error == -4)))
        {
            connected = true;
        }
        if (!connected)
        {
            std::memset(&status, 0, sizeof(KPADStatus));
        }
        joypad.setConnected(connected);
    }
}
void CInput::ClearJoypadValues(JOYPAD* joypad)
{
    joypad->touchValue = 0;
    joypad->rawValue = 0;
    joypad->pressedValue = 0;
    joypad->releasedValue = 0;
    joypad->rawRightStickX = 0;
    joypad->rawRightStickY = 0;
    joypad->rawLeftStickX = 0;
    joypad->rawLeftStickY = 0;
    joypad->leftStickX = 0;
    joypad->leftStickY = 0;
    joypad->rightStickX = 0;
    joypad->rightStickY = 0;
    joypad->leftX = 0.0f;
    joypad->leftY = 0.0f;
    joypad->rightX = 0.0f;
    joypad->rightY = 0.0f;
    joypad->hasTouch = 0;
    joypad->touchPressed = 0;
}
void CInput::JoypadConnected(int pad)
{
    ClearJoypadValues(&joypads[pad]);
}
void CInput::JoypadRemoved(int pad)
{
    ClearJoypadValues(&joypads[pad]);
}
void CInput::SetDRCJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    VPADStatus* status = static_cast<VPADStatus*>(joypad->statusStorage.vpad.statuses) + joypad->statusIndex;
    if ((status->hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetJoypadValuesDRC(JOYPAD* joypad)
{
    VPADStatus* status = static_cast<VPADStatus*>(joypad->statusStorage.vpad.statuses) + joypad->statusIndex;
    static const unsigned int source[] = {0x4000, 0x8000, 0x1000,  0x2000,  0x40,  0x80,  0x10,  0x20,
                                          8,      4,      0x20000, 0x40000, 0x200, 0x100, 0x800, 0x400};
    static const unsigned int destination[] = {1,    2,    4,     8,     0x400000, 0x800000, 0x40,   0x80,
                                               0x10, 0x20, 0x100, 0x200, 0x400,    0x800,    0x1000, 0x2000};
    for (int button = 0; button < 16; ++button)
    {
        SetDRCJoypadValue(joypad, source[button], destination[button]);
    }
    int leftX = GetStickValue(status->leftStick.x);
    int leftY = GetStickValue(status->leftStick.y);
    int rightX = GetStickValue(status->rightStick.x);
    int rightY = GetStickValue(status->rightStick.y);
    if (leftX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x4000;
    if (leftX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x8000;
    if (rightY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x10000;
    if (rightY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x20000;
    if (rightX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x40000;
    if (rightX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x80000;
    if (leftY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x100000;
    if (leftY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x200000;
    joypad->rawRightStickX = ApplyDeadzone(rightX, deadzone);
    joypad->rawRightStickY = ApplyDeadzone(rightY, deadzone);
    joypad->rawLeftStickX = ApplyDeadzone(leftX, deadzone);
    joypad->rawLeftStickY = ApplyDeadzone(leftY, deadzone);
    joypad->leftStickX = joypad->rawRightStickX / static_cast<int>(movementSpan);
    joypad->leftStickY = joypad->rawRightStickY / static_cast<int>(movementSpan);
    joypad->rightStickX = joypad->rawLeftStickX / static_cast<int>(movementSpan);
    joypad->rightStickY = joypad->rawLeftStickY / static_cast<int>(movementSpan);
    joypad->leftX = NormaliseStick(joypad->rawRightStickX, movementSpanFloat);
    joypad->leftY = NormaliseStick(joypad->rawRightStickY, movementSpanFloat);
    joypad->rightX = NormaliseStick(joypad->rawLeftStickX, movementSpanFloat);
    joypad->rightY = NormaliseStick(joypad->rawLeftStickY, movementSpanFloat);
}
void CInput::GetStartTime(int pad, int type)
{
    BUTTON_TIME& time = buttonTimes[pad][type];
    long long now = OSGetTime();
    SetTimeValue(time.lastUpdate, now);
    time.elapsed.high = 0;
    time.elapsed.low = 0;
    time.heldTime = 0.0f;
    time.repeatTime = 0.0f;
    time.repeated = 0;
    SetTimeValue(idleStartTimes[pad], now);
}
void CInput::UpdateTime(int pad, int type)
{
    BUTTON_TIME& time = buttonTimes[pad][type];
    long long now = OSGetTime();
    long long elapsed = now - GetTimeValue(time.lastUpdate);
    SetTimeValue(time.lastUpdate, now);
    SetTimeValue(time.elapsed, GetTimeValue(time.elapsed) + elapsed);
    time.heldTime += static_cast<float>(elapsed) * timeScale;
    SetTimeValue(idleStartTimes[pad], now);
}
bool CInput::UpdateDRCPad()
{
    JOYPAD& joypad = joypads[0];
    unsigned int previous = joypad.rawValue;
    controllerCount = 0;
    joypad.rawValue = 0;
    joypad.previousValue = previous;
    joypad.rawRightStickX = 0;
    joypad.rawRightStickY = 0;
    joypad.rawLeftStickX = 0;
    joypad.rawLeftStickY = 0;
    joypad.hasTouch = 0;
    joypad.touchPressed = 0;
    if (!joypad.sampleIndex)
    {
        if (joypad.newlyConnected)
        {
            JoypadConnected(0);
        }
        if (joypad.disconnected)
        {
            JoypadRemoved(0);
        }
        if (!joypad.connected)
        {
            if (joypad.disconnected)
            {
                ClearJoypadValues(&joypad);
            }
            return true;
        }

        controllerCount = 1;
        SetJoypadValuesDRC(&joypad);
        joypad.rawValue |= joypad.touchValue;
        joypad.touchValue = 0;
        joypad.pressedValue = joypad.rawValue & ~joypad.previousValue;
        joypad.releasedValue = joypad.previousValue & ~joypad.rawValue;
        for (int button = 0; button < 24; ++button)
        {
            unsigned int mask = 1u << button;
            if (joypad.pressedValue & mask)
            {
                GetStartTime(0, button);
            }
            else if (joypad.rawValue & mask)
            {
                UpdateTime(0, button);
            }
        }
    }
    return true;
}
void CInput::SetClassicJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    if ((status->classic.hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetFreestyleJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    if ((status->nunchuk.hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetURCCJoypadValue(JOYPAD* joypad, unsigned int sourceMask, unsigned int mappedMask)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    if ((status->pro.hold & sourceMask) == sourceMask)
    {
        joypad->rawValue |= mappedMask;
    }
}
void CInput::SetJoypadValues(JOYPAD* joypad)
{
    KPADStatus* status = static_cast<KPADStatus*>(joypad->statusStorage.kpad.statuses) + joypad->statusIndex;
    int leftX = 0;
    int leftY = 0;
    int rightX = 0;
    int rightY = 0;
    if (status->extensionType == 2)
    {
        static const unsigned int source[] = {0x10,   0x40,  0x20,   8, 4,      0x80, 0x200,
                                              0x2000, 0x400, 0x1000, 1, 0x4000, 2,    0x8000};
        static const unsigned int destination[] = {2,    1,    4,    8,     0x400000, 0x800000, 0x40,
                                                   0x80, 0x10, 0x20, 0x400, 0x800,    0x1000,   0x2000};
        for (int button = 0; button < 14; ++button)
        {
            SetClassicJoypadValue(joypad, source[button], destination[button]);
        }
        rightX = GetStickValue(status->classic.rightStick.x);
        rightY = GetStickValue(status->classic.rightStick.y);
        leftX = GetStickValue(status->classic.leftStick.x);
        leftY = GetStickValue(status->classic.leftStick.y);
    }
    else if (status->extensionType == 1)
    {
        static const unsigned int source[] = {0x800, 0x400, 0x200, 0x100, 0x2000, 0x4000, 0x10, 0x1000, 8, 4, 1, 2};
        static const unsigned int destination[] = {2,    1,    4,     8,     0x400000, 0x800000,
                                                   0x10, 0x20, 0x400, 0x800, 0x1000,   0x2000};
        for (int button = 0; button < 12; ++button)
        {
            SetFreestyleJoypadValue(joypad, source[button], destination[button]);
        }
        leftX = GetStickValue(status->nunchuk.stick.x);
        leftY = GetStickValue(status->nunchuk.stick.y);
    }
    else if (status->extensionType == 0x1f)
    {
        static const unsigned int source[] = {0x10,  0x40,   0x20,    8,       4, 0x80,   0x200, 0x2000,
                                              0x400, 0x1000, 0x10000, 0x20000, 1, 0x4000, 2,     0x8000};
        static const unsigned int destination[] = {2,    1,    4,     8,     0x400000, 0x800000, 0x40,   0x80,
                                                   0x10, 0x20, 0x100, 0x200, 0x400,    0x800,    0x1000, 0x2000};
        for (int button = 0; button < 16; ++button)
        {
            SetURCCJoypadValue(joypad, source[button], destination[button]);
        }
        rightX = GetStickValue(status->pro.rightStick.x);
        rightY = GetStickValue(status->pro.rightStick.y);
        leftX = GetStickValue(status->pro.leftStick.x);
        leftY = GetStickValue(status->pro.leftStick.y);
    }
    if (leftX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x4000;
    if (leftX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x8000;
    if (rightY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x10000;
    if (rightY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x20000;
    if (rightX > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x40000;
    if (rightX < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x80000;
    if (leftY < -static_cast<int>(maximumInput))
        joypad->rawValue |= 0x100000;
    if (leftY > static_cast<int>(maximumInput))
        joypad->rawValue |= 0x200000;
    joypad->rawRightStickX = ApplyDeadzone(rightX, deadzone);
    joypad->rawRightStickY = ApplyDeadzone(rightY, deadzone);
    joypad->rawLeftStickX = ApplyDeadzone(leftX, deadzone);
    joypad->rawLeftStickY = ApplyDeadzone(leftY, deadzone);
    joypad->leftStickX = joypad->rawRightStickX / static_cast<int>(movementSpan);
    joypad->leftStickY = joypad->rawRightStickY / static_cast<int>(movementSpan);
    joypad->rightStickX = joypad->rawLeftStickX / static_cast<int>(movementSpan);
    joypad->rightStickY = joypad->rawLeftStickY / static_cast<int>(movementSpan);
    joypad->leftX = NormaliseStick(joypad->rawRightStickX, movementSpanFloat);
    joypad->leftY = NormaliseStick(joypad->rawRightStickY, movementSpanFloat);
    joypad->rightX = NormaliseStick(joypad->rawLeftStickX, movementSpanFloat);
    joypad->rightY = NormaliseStick(joypad->rawLeftStickY, movementSpanFloat);
}
bool CInput::UpdateJoypads()
{
    int connected = 0;
    for (int pad = 1; pad < 5; ++pad)
    {
        JOYPAD& joypad = joypads[pad];
        joypad.previousValue = joypad.rawValue;
        joypad.rawValue = joypad.fakeValue;
        if (joypad.fakeTimer)
        {
            --joypad.fakeTimer;
        }
        else
        {
            joypad.fakeValue = 0;
        }
        joypad.rawRightStickX = 0;
        joypad.rawRightStickY = 0;
        joypad.rawLeftStickX = 0;
        joypad.rawLeftStickY = 0;
        joypad.hasTouch = 0;
        joypad.touchPressed = 0;
    }
    for (int pad = 1; pad < 5; ++pad)
    {
        JOYPAD& joypad = joypads[pad];
        if (joypad.sampleIndex)
        {
            ClearJoypadValues(&joypad);
            if (joypad.connected)
            {
                ++connected;
            }
            continue;
        }
        if (joypad.newlyConnected)
        {
            JoypadConnected(pad);
        }
        if (joypad.disconnected)
        {
            JoypadRemoved(pad);
        }
        if (joypad.connected)
        {
            SetJoypadValues(&joypad);
            joypad.rawValue |= joypad.touchValue;
            joypad.touchValue = 0;
            if (!joypad.previousSampleIndex)
            {
                joypad.pressedValue = joypad.rawValue & ~joypad.previousValue;
                joypad.releasedValue = joypad.previousValue & ~joypad.rawValue;
            }
            else
            {
                joypad.pressedValue = 0;
                joypad.releasedValue = 0;
            }
            for (int button = 0; button < 24; ++button)
            {
                unsigned int mask = 1u << button;
                if (joypad.pressedValue & mask)
                {
                    GetStartTime(pad, button);
                }
                else if (joypad.rawValue & mask)
                {
                    UpdateTime(pad, button);
                }
            }
        }
        else if (joypad.disconnected)
        {
            ClearJoypadValues(&joypad);
        }
        if (joypad.connected)
        {
            ++connected;
        }
    }
    controllerCount += connected;
    return true;
}
void CInput::Tick()
{
    ReadAllPads();
    TickErrorViewer();
    TickKeyboard();
    UpdateDRCPad();
    UpdateJoypads();
}
void CInput::SetDeadzoneAndMovementRange(unsigned int value, unsigned int maximum, unsigned int range)
{
    deadzone = value;
    maximumInput = maximum;
    movementRange = range;
    movementSpan = range - value;
    movementSpanFloat = static_cast<float>(movementSpan);
}
void CInput::SetGameControllerTypeJoypadMaps(int controllerType, unsigned char map, unsigned int value)
{
    unsigned int** maps = controllerType == 0 ? gameJoypadMaps : alternateJoypadMaps;
    for (int pad = 0; pad < 5; ++pad)
    {
        maps[pad][map] = value;
    }
}
bool CInput::RemapIpad(int& pad)
{
    bool result = false;
    if (static_cast<unsigned int>(pad) < 4)
    {
        if (padRemap[pad] >= 0)
        {
            pad = padRemap[pad];
            result = true;
        }
    }
    else if (static_cast<unsigned int>(pad - 4) < 5)
    {
        pad -= 4;
        result = true;
    }
    return result;
}
void CInput::SetGameJoypadMaps(int pad, unsigned char map, unsigned int value)
{
    RemapIpad(pad);
    gameJoypadMaps[pad][map] = value;
    alternateJoypadMaps[pad][map] = value;
}
unsigned int CInput::GetGameJoypadMaps(int pad, unsigned char map)
{
    int originalPad = pad;
    if (!RemapIpad(pad))
    {
        return 0;
    }
    return IsPadClassicController(originalPad) ? alternateJoypadMaps[pad][map] : gameJoypadMaps[pad][map];
}
void CInput::SetJoypadMapArraySetupComplete()
{
    joypadMapsReady = 1;
}
void CInput::SetJoypadSensitivity(int pad, float sensitivity)
{
    if (RemapIpad(pad))
    {
        joypads[pad].sensitivity = sensitivity;
    }
}
unsigned int CInput::only_one_bit_set(unsigned int value)
{
    return value != 0 && (value & (value - 1)) == 0;
}
unsigned int CInput::allowInput(int pad, unsigned char map, unsigned int value)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return 0;
    }
    unsigned int mapping = GetGameJoypadMaps(pad, map);
    if (value == 0 || value != mapping)
    {
        return 0;
    }
    if (only_one_bit_set(mapping))
    {
        for (unsigned char other = controllerConfiguration[2] + 1; other < controllerConfiguration[1]; ++other)
        {
            unsigned int otherMapping = GetGameJoypadMaps(pad, other);
            if (!only_one_bit_set(otherMapping) && (mapping & otherMapping)
                && (joypads[mappedPad].rawValue & mapping & otherMapping)
                && (joypads[mappedPad].rawValue & otherMapping) == otherMapping)
            {
                return 0;
            }
        }
    }
    return 1;
}
unsigned int CInput::GetValue(int pad, unsigned char map, bool repeated)
{
    bool menuOpen = pad < 4 && menuDisplayed[pad];
    int mappedPad = pad;
    if (!RemapIpad(mappedPad) || (menuOpen && map > controllerConfiguration[2]))
    {
        return 0;
    }
    unsigned int value = joypads[mappedPad].rawValue & GetGameJoypadMaps(pad, map);
    if (map > controllerConfiguration[2] && !allowInput(pad, map, value))
    {
        return 0;
    }
    if (!repeated || value == 0)
    {
        return value;
    }
    int button = 0;
    unsigned int buttons = value;
    while ((buttons & 1) == 0)
    {
        ++button;
        buttons >>= 1;
    }
    BUTTON_TIME& time = buttonTimes[mappedPad][button];
    BUTTON_TIME* repeatTime = &time;
    if (!only_one_bit_set(buttons))
    {
        int secondButton = button + 1;
        unsigned int remainingButtons = buttons >> 1;
        while ((remainingButtons & 1) == 0)
        {
            ++secondButton;
            remainingButtons >>= 1;
        }
        BUTTON_TIME& secondTime = buttonTimes[mappedPad][secondButton];
        if (time.heldTime <= secondTime.heldTime)
        {
            if (secondTime.heldTime - time.heldTime < 0.001f)
            {
                secondTime.heldTime = secondTime.heldTime + 0.002f;
            }
        }
        else
        {
            if (time.heldTime - secondTime.heldTime < 0.001f)
            {
                time.heldTime = time.heldTime + 0.002f;
            }
            repeatTime = &secondTime;
        }
    }
    if (repeatTime->heldTime >= initialRepeatDelay)
    {
        if (repeatTime->repeatTime == 0.0f)
        {
            repeatTime->repeatTime = initialRepeatDelay;
            return value;
        }
        if (repeatTime->heldTime - repeatTime->repeatTime > repeatDelay)
        {
            repeatTime->repeatTime += repeatDelay;
            return value;
        }
    }
    else if (!repeatTime->repeated)
    {
        repeatTime->repeated = 1;
        return value;
    }
    return 0;
}
void CInput::SetFakePadValue(int pad, unsigned char map)
{
    unsigned char menuOpen = 0;
    if (pad < 4)
    {
        menuOpen = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!menuOpen || map <= controllerConfiguration[2]))
    {
        joypads[mappedPad].fakeValue |= GetGameJoypadMaps(pad, map);
        joypads[mappedPad].fakeTimer = 10;
    }
}
bool CInput::ButtonPressed(int pad, unsigned char map, unsigned int mask)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return false;
    }
    if (map == 0xff)
    {
        return joypads[mappedPad].pressedValue != 0;
    }
    unsigned int mapping = GetGameJoypadMaps(pad, map);
    if (map <= controllerConfiguration[2])
    {
        return (joypads[mappedPad].pressedValue & mapping) != 0;
    }
    if (pad < 4 && menuDisplayed[pad])
    {
        return false;
    }
    unsigned int value = joypads[mappedPad].pressedValue & mapping;
    if (!only_one_bit_set(mapping) && (joypads[mappedPad].rawValue & mask) != 0)
    {
        value |= mask;
    }
    return value == mapping && allowInput(pad, map, value);
}
bool CInput::ButtonReleased(int pad, unsigned char map)
{
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!menuDisplayed[pad] || map <= controllerConfiguration[2]))
    {
        return (joypads[mappedPad].releasedValue & GetGameJoypadMaps(pad, map)) != 0;
    }
    return false;
}
bool CInput::ButtonDown(int pad, unsigned char map)
{
    int mappedPad = pad;
    if (!RemapIpad(mappedPad))
    {
        return false;
    }
    if (map == 0xff)
    {
        return joypads[mappedPad].rawValue != 0;
    }
    if (pad < 4 && menuDisplayed[pad] && map > controllerConfiguration[2])
    {
        return false;
    }
    unsigned int mapping = GetGameJoypadMaps(pad, map);
    unsigned int value = joypads[mappedPad].rawValue & mapping;
    if (map <= controllerConfiguration[2])
    {
        return value != 0;
    }
    return value == mapping && allowInput(pad, map, value);
}
void CInput::SetJoypadStickAxisMap(int pad, unsigned int destination, unsigned int source)
{
    if (!RemapIpad(pad))
    {
        return;
    }
    float* sourceValue = 0;
    if (source == 0)
        sourceValue = &joypads[pad].rightX;
    else if (source == 1)
        sourceValue = &joypads[pad].rightY;
    else if (source == 2)
        sourceValue = &joypads[pad].leftX;
    else if (source == 3)
        sourceValue = &joypads[pad].leftY;
    if (destination == 0)
        joypads[pad].leftXSource = sourceValue;
    else if (destination == 1)
        joypads[pad].leftYSource = sourceValue;
    else if (destination == 2)
        joypads[pad].rightXSource = sourceValue;
    else if (destination == 3)
        joypads[pad].rightYSource = sourceValue;
}
void CInput::SetJoypadStickTriggerMap(int pad, unsigned int destination, unsigned int source)
{
    if (!RemapIpad(pad))
    {
        return;
    }
    unsigned char* sourceValue = 0;
    if (source == 0)
        sourceValue = &joypads[pad].hasTouch;
    else if (source == 1)
        sourceValue = &joypads[pad].touchPressed;
    if (destination == 0)
        joypads[pad].leftTriggerSource = sourceValue;
    else if (destination == 1)
        joypads[pad].rightTriggerSource = sourceValue;
}
void CInput::SetKeyRepeatRate(float initialDelay, float delay)
{
    initialRepeatDelay = initialDelay;
    repeatDelay = delay;
}
void CInput::SetDebugSequence(const char* sequence, InputCallback callback, void* context)
{
    debugSequenceLength = std::strlen(sequence);
    debugSequence = new char[debugSequenceLength + 1];
    std::strcpy(debugSequence, sequence);
    debugSequenceCallback = callback;
    debugSequenceContext = context;
}
double CInput::GetIdleSeconds(int pad)
{
    return static_cast<float>(OSGetTime() - GetTimeValue(idleStartTimes[pad])) * timeScale;
}
unsigned char CInput::IsPadConnected(int pad)
{
    if (RemapIpad(pad))
    {
        return joypads[pad].connected;
    }
    return pad < controllerCount;
}
void CInput::SetCircleCrossSwapped(bool value)
{
    circleCrossSwapped = value;
}
unsigned char CInput::IsCircleCrossSwapped()
{
    return circleCrossSwapped;
}
void CInput::MapTouchInput(int pad, unsigned int mask)
{
    joypads[pad].touchValue = joypads[pad].touchValue & ~TOUCH_INPUT_BUTTONS | mask;
}
float CInput::GetJoypadStick_LX(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].leftXSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
float CInput::GetJoypadStick_LY(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].leftYSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
float CInput::GetJoypadStick_RX(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].rightXSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
float CInput::GetJoypadStick_RY(int pad, bool checkMenu)
{
    bool menuShown = false;
    if (pad < 4)
    {
        menuShown = menuDisplayed[pad];
    }
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!checkMenu || !menuShown))
    {
        return *joypads[mappedPad].rightYSource * joypads[mappedPad].sensitivity;
    }
    return 0.0;
}
unsigned char CInput::GetJoypadLTrigger(int pad, bool raw)
{
    if (pad < 4 && raw && menuDisplayed[pad])
    {
        return 0;
    }
    if (RemapIpad(pad) && joypads[pad].leftTriggerSource != 0)
    {
        return *joypads[pad].leftTriggerSource;
    }
    return 0;
}
unsigned char CInput::GetJoypadRTrigger(int pad, bool raw)
{
    if (pad < 4 && raw && menuDisplayed[pad])
    {
        return 0;
    }
    if (RemapIpad(pad) && joypads[pad].rightTriggerSource != 0)
    {
        return *joypads[pad].rightTriggerSource;
    }
    return 0;
}
unsigned short* CInput::GetTouchPadData(int pad, C_4JInput::ETouchType type, bool previous)
{
    if (!RemapIpad(pad) || pad != 0 || !joypads[pad].connected || (previous && joypads[pad].sampleIndex))
    {
        return 0;
    }
    if (type == C_4JInput::ETouchType_0)
    {
        return reinterpret_cast<unsigned short*>(
            &static_cast<VPADStatus*>(joypads[pad].statusStorage.vpad.statuses)[joypads[pad].statusIndex].tpNormal);
    }
    if (type == C_4JInput::ETouchType_1)
    {
        return reinterpret_cast<unsigned short*>(
            &static_cast<VPADStatus*>(joypads[pad].statusStorage.vpad.statuses)[joypads[pad].statusIndex].tpFiltered1);
    }
    if (type == C_4JInput::ETouchType_2)
    {
        return reinterpret_cast<unsigned short*>(
            &static_cast<VPADStatus*>(joypads[pad].statusStorage.vpad.statuses)[joypads[pad].statusIndex].tpFiltered2);
    }
    return 0;
}
void CInput::SetMenuDisplayed(int pad, bool displayed)
{
    menuDisplayed[pad] = displayed;
}
void CInput::SetLocalMultiplayer(bool value)
{
    localMultiplayer = value;
}
unsigned int CInput::IsLocalMultiplayerAvailable()
{
    return 1;
}
unsigned int CInput::UsingRemoteVita()
{
    return 0;
}
unsigned int CInput::VerifyStrings(wchar_t**, int, int (*)(void*, _STRING_VERIFY_RESPONSE*), void*)
{
    return 1;
}
unsigned int CInput::LockPad(int pad, int controllerId)
{
    if (!RemapIpad(pad))
    {
        return 0;
    }
    int previousPad = padRemap[controllerId];
    if (previousPad != pad)
    {
        if (previousPad >= 0)
        {
            joypads[previousPad].lockedController = -1;
        }
        int previousController = joypads[pad].lockedController;
        if (previousController >= 0)
        {
            padRemap[previousController] = -1;
        }
        padRemap[controllerId] = pad;
        joypads[pad].lockedController = controllerId;
        if (previousPad >= 0)
        {
            for (int controller = 0; controller < 4; ++controller)
            {
                if (padRemap[controller] == -1)
                {
                    padRemap[controller] = previousPad;
                    joypads[previousPad].lockedController = controller;
                    break;
                }
            }
        }
    }
    return 1;
}
int CInput::UnlockPad(int pad)
{
    if (pad < 4)
    {
        int mappedPad = padRemap[pad];
        if (mappedPad >= 0)
        {
            joypads[mappedPad].lockedController = -1;
            padRemap[pad] = -1;
            unsigned int previousButtons = joypads[mappedPad].rawValue;
            ClearJoypadValues(&joypads[mappedPad]);
            joypads[mappedPad].rawValue = previousButtons;
            return mappedPad + 4;
        }
    }
    return -1;
}
unsigned int CInput::IsPadLocked(int pad)
{
    if (RemapIpad(pad))
    {
        return joypads[pad].lockedController >= 0;
    }
    return 0;
}
int CInput::GetControllerID(int pad)
{
    return RemapIpad(pad) ? pad + 4 : -1;
}
unsigned char CInput::IsPadDRCController(int pad)
{
    return RemapIpad(pad) && pad == 0 && joypads[0].connected;
}
unsigned int CInput::IsPadClassicController(int pad)
{
    if (RemapIpad(pad) && pad != 0)
    {
        KPADStatus* status = joypads[pad].statusStorage.kpad.statuses;
        if (status->extensionType == WPAD_EXT_CLASSIC || status->extensionType == WPAD_EXT_MPLUS_CLASSIC)
        {
            return 1;
        }
    }
    return 0;
}
unsigned char CInput::IsDRCConnected()
{
    return joypads[0].connected;
}
void CInput::AcquiredForeground()
{
    inputTick = 20;
}
bool CInput::IsSet(int pad, unsigned char map)
{
    int mappedPad = pad;
    if (RemapIpad(mappedPad) && (!menuDisplayed[pad] || map <= controllerConfiguration[2]))
    {
        return (joypads[mappedPad].rawValue & GetGameJoypadMaps(pad, map)) != 0;
    }
    return false;
}
float CInput::GetJoypadStick_Menu_LX(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].rightX : 0.0;
}
float CInput::GetJoypadStick_Menu_LY(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].rightY : 0.0;
}
float CInput::GetJoypadStick_Menu_RX(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].leftX : 0.0;
}
float CInput::GetJoypadStick_Menu_RY(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].leftY : 0.0;
}
unsigned char CInput::GetJoypadLTrigger_Menu(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].hasTouch : 0;
}
unsigned char CInput::GetJoypadRTrigger_Menu(int pad)
{
    int mappedPad = pad;
    return RemapIpad(mappedPad) ? joypads[mappedPad].touchPressed : 0;
}
intptr_t CInput::GetVPadStatus()
{
    if (joypads[0].sampleIndex == 0)
    {
        joypads[0].sampleIndex = 1;
        return (intptr_t)(static_cast<VPADStatus*>(joypads[0].statusStorage.vpad.statuses) + joypads[0].statusIndex);
    }
    return 0;
}
intptr_t CInput::GetKPadStatus(int pad)
{
    if (joypads[pad].sampleIndex == 0)
    {
        joypads[pad].sampleIndex = 1;
        return (intptr_t)(static_cast<KPADStatus*>(joypads[pad].statusStorage.kpad.statuses) + joypads[pad].statusIndex);
    }
    return 0;
}
void CInput::CancelQueuedVerifyStrings(int (*callback)(void*, _STRING_VERIFY_RESPONSE*), void* context)
{
}
void CInput::CancelAllVerifyInProgress()
{
}
unsigned char CInput::IsShowingOnscreenKeyboard()
{
    return keyboardVisible;
}
void CInput::SetKeyboardMode(eKeyboardProcessState value)
{
    keyboardState = value;
}
unsigned int CInput::RequestKeyboard(const wchar_t* title, const wchar_t* initialText, unsigned int pad,
                                     unsigned int maximumLength, KeyboardCallback callback, void* context,
                                     C_4JInput::EKeyboardMode mode)
{
    if (keyboardState != eKeyboardProcessState_0)
    {
        g_pInputInstance->DebugPrintf("CInput::RequestKeyboard: Ignored a keyboard request while keyboard is active\n");
        return 1;
    }

    SetKeyboardMode(eKeyboardProcessState_1);
    const wchar_t* diagnosticText = initialText == 0 ? L"" : initialText;
    g_pInputInstance->DebugPrintf("CInput::RequestKeyboard: Opening keyboard : maxChars - %d init str - %ls\n",
                                  maximumLength, diagnosticText);
    keyboardCallback = callback;
    keyboardMaximumLength = maximumLength;
    keyboardContext = context;
    keyboardPad = pad;
    keyboardMode = mode;
    g_pInputInstance->RemapIpad(keyboardPad);
    std::memset(keyboardText, 0, 0x200);
    std::memcpy(keyboardText, initialText, maximumLength << 1);
    return 0;
}
void CInput::DestroyKeyboard()
{
    if (keyboardState == eKeyboardProcessState_2)
    {
        destroyKeyboardRequested = 1;
    }
}
void CInput::GetText(wchar_t* value, int length)
{
    std::wcsncpy(value, keyboardText, length);
}
int CInput::ProcFont(int value, void* context)
{
    for (;;)
    {
        OSMessage message;
        OSReceiveMessage(&s_fontQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);
        nn::swkbd::CalcSubThreadFont();
    }
}
int CInput::ProcPredict(int value, void* context)
{
    for (;;)
    {
        OSMessage message;
        OSReceiveMessage(&s_predictQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);
        nn::swkbd::CalcSubThreadPredict();
    }
}

void CInput::KeyboardOnCreate()
{
    keyboardState = eKeyboardProcessState_0;
    destroyKeyboardRequested = 0;
    OSInitMessageQueue(&s_fontQueue, &s_fontMessage, 1);
    OSInitMessageQueue(&s_predictQueue, &s_predictMessage, 1);
    C4JThreadImpl::OSCreateThread4J(&s_fontThread, CInput::ProcFont, 0, 0, s_fontStack + sizeof(s_fontStack),
                                    sizeof(s_fontStack), 0x11, 0);
    OSResumeThread(&s_fontThread);
    C4JThreadImpl::OSCreateThread4J(&s_predictThread, CInput::ProcPredict, 0, 0,
                                    s_predictStack + sizeof(s_predictStack), sizeof(s_predictStack), 0x12, 0);
    OSResumeThread(&s_predictThread);
}
unsigned int CInput::GetKeyboardControllerType()
{
    static const unsigned char controllerTypes[5] = {4, 0, 1, 2, 3};
    if (keyboardPad < 5)
    {
        return controllerTypes[keyboardPad];
    }
    return keyboardPad;
}
unsigned int CInput::GetKeyboardMode()
{
    switch (static_cast<unsigned int>(keyboardMode))
    {
    case 1:
    case 6:
    case 7:
        keyboardControllerType = KEYBOARD_FLAG_ALL;
        return 1;
    case 3:
    case 5:
        break;
    default:
        keyboardControllerType = KEYBOARD_FLAG_ALL;
        return 0;
    }
    switch (keyboardLanguage)
    {
    case SCI_PLATFORM_REGION_JPN:
    case SCI_PLATFORM_REGION_CHN:
    case SCI_PLATFORM_REGION_KOR:
        keyboardControllerType = KEYBOARD_FLAG_QWERTY_US_ENG;
        return 0;
    case 2:
        switch (keyboardRegion)
        {
        case SCI_CAFE_LANGUAGE_FRENCH:
            keyboardControllerType = KEYBOARD_FLAG_QWERTY_US_FRA;
            return 0;
        case SCI_CAFE_LANGUAGE_SPANISH:
            keyboardControllerType = KEYBOARD_FLAG_QWERTY_US_SPA;
            return 0;
        default:
            keyboardControllerType = KEYBOARD_FLAG_QWERTY_US_ENG;
            return 0;
        }
    case 4:
    {
        static const unsigned short accessFlags[] = {KEYBOARD_FLAG_QWERTY_EU_FRA, KEYBOARD_FLAG_QWERTY_EU_DEU,
                                                     KEYBOARD_FLAG_QWERTY_EU_ITA, KEYBOARD_FLAG_QWERTY_EU_SPA};
        if (keyboardRegion < SCI_CAFE_LANGUAGE_FRENCH)
        {
            keyboardControllerType = KEYBOARD_FLAG_QWERTY_EU_ENG;
            return 0;
        }
        if (keyboardRegion <= SCI_CAFE_LANGUAGE_SPANISH)
        {
            keyboardControllerType = accessFlags[keyboardRegion - SCI_CAFE_LANGUAGE_FRENCH];
            return 0;
        }
        if (keyboardRegion == SCI_CAFE_LANGUAGE_DUTCH)
        {
            keyboardControllerType = KEYBOARD_FLAG_QWERTY_EU_NLD;
            return 0;
        }
        if (keyboardRegion == SCI_CAFE_LANGUAGE_PORTUGUESE)
        {
            keyboardControllerType = KEYBOARD_FLAG_QWERTY_EU_POR;
            return 0;
        }
        keyboardControllerType = KEYBOARD_FLAG_QWERTY_EU_ENG;
        return 0;
    }
    }
    return 0;
}
unsigned char CInput::GetLanguageType()
{
    static const unsigned char regionLanguages[] = {0, 1, 1, 6, 7, 11};
    static const unsigned char languages[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    if (keyboardRegion < 12)
    {
        return languages[keyboardRegion];
    }
    if (keyboardLanguage < 6)
    {
        return regionLanguages[keyboardLanguage];
    }
    return 1;
}
void CInput::TickKeyboard()
{
    if (keyboardState == eKeyboardProcessState_1)
    {
        nn::swkbd::AppearArg appearArg;
        appearArg.keyboardArg.receiverArg.unk_0x00 = 0;
        appearArg.keyboardArg.receiverArg.unk_0x04 = 0;
        appearArg.keyboardArg.receiverArg.unk_0x08 = 0;
        appearArg.keyboardArg.receiverArg.unk_0x0C = -1;
        appearArg.keyboardArg.receiverArg.unk_0x10 = 0;
        appearArg.keyboardArg.receiverArg.unk_0x14 = -1;
        appearArg.keyboardArg.configArg.controllerType = static_cast<nn::swkbd::ControllerType>(
            GetKeyboardControllerType());
        appearArg.keyboardArg.configArg.keyboardMode = static_cast<nn::swkbd::KeyboardMode>(GetKeyboardMode());
        appearArg.keyboardArg.configArg.accessFlags = keyboardControllerType;
        appearArg.keyboardArg.configArg.languageType = static_cast<nn::swkbd::LanguageType>(GetLanguageType());
        appearArg.keyboardArg.configArg.disableNewLine = true;
        appearArg.inputFormArg.type = static_cast<nn::swkbd::InputFormType>(1);
        appearArg.inputFormArg.initialText = reinterpret_cast<const char16_t*>(keyboardText);
        appearArg.inputFormArg.unk_0x04 = -1;
        appearArg.inputFormArg.hintText = 0;
        appearArg.inputFormArg.maxTextLength = keyboardMaximumLength;
        appearArg.inputFormArg.passwordMode = static_cast<nn::swkbd::PasswordMode>(0);
        appearArg.inputFormArg.unk_0x18 = 0;
        appearArg.inputFormArg.drawInput0Cursor = false;
        appearArg.inputFormArg.higlightInitialText = false;
        appearArg.inputFormArg.showCopyPasteButtons = false;
        nn::swkbd::AppearInputForm(appearArg);
        SetKeyboardMode(eKeyboardProcessState_2);
        return;
    }

    if (keyboardState == eKeyboardProcessState_2)
    {
        keyboardVisible = 1;
        nn::swkbd::ControllerInfo controllerInfo;
        std::memset(&controllerInfo, 0, sizeof(controllerInfo));
        if (keyboardPad == 0)
        {
            controllerInfo.vpad = (VPADStatus*)g_pInputInstance->GetVPadStatus();
        }
        else
        {
            controllerInfo.kpad[keyboardPad - 1] = (KPADStatus*)g_pInputInstance->GetKPadStatus(keyboardPad);
        }
        nn::swkbd::Calc(controllerInfo);
        OSMessage fontMessage;
        OSMessage predictMessage;
        if (nn::swkbd::IsNeedCalcSubThreadFont())
        {
            OSSendMessage(&s_fontQueue, &fontMessage, OS_MESSAGE_FLAGS_NONE);
        }
        if (nn::swkbd::IsNeedCalcSubThreadPredict())
        {
            OSSendMessage(&s_predictQueue, &predictMessage, OS_MESSAGE_FLAGS_NONE);
        }
        if (nn::swkbd::IsDecideOkButton(0))
        {
            std::wcsncpy(keyboardText, (const wchar_t*)nn::swkbd::GetInputFormString(), keyboardMaximumLength);
            keyboardText[keyboardMaximumLength] = 0;
            if (keyboardCallback)
            {
                keyboardCallback(keyboardContext, true);
            }
            nn::swkbd::DisappearInputForm();
        }
        else if (nn::swkbd::IsDecideCancelButton(0) || destroyKeyboardRequested)
        {
            if (keyboardCallback)
            {
                keyboardCallback(keyboardContext, false);
            }
            nn::swkbd::DisappearInputForm();
            destroyKeyboardRequested = 0;
        }
        if (static_cast<int>(nn::swkbd::GetStateKeyboard()) == 0)
        {
            SetKeyboardMode(eKeyboardProcessState_0);
        }
    }
}
void CInput::InitialiseKeyboard()
{
    keyboardFileClient = static_cast<FSClient*>(std::malloc(sizeof(FSClient)));
    FSAddClient(keyboardFileClient, FS_ERROR_FLAG_NONE);
    nn::swkbd::CreateArg createArg;
    keyboardErrorViewerWorkMemory = new unsigned char[nn::swkbd::GetWorkMemorySize(0)];
    createArg.workMemory = keyboardErrorViewerWorkMemory;
    createArg.fsClient = keyboardFileClient;
    unsigned int platformRegion;
    if (SCIGetPlatformRegion(&platformRegion) == 1)
    {
        switch (platformRegion)
        {
        case SCI_PLATFORM_REGION_JPN:
            keyboardLanguage = 0;
            break;
        case SCI_PLATFORM_REGION_USA:
            keyboardLanguage = 1;
            break;
        case SCI_PLATFORM_REGION_CHN:
            keyboardLanguage = 3;
            break;
        case SCI_PLATFORM_REGION_KOR:
            keyboardLanguage = 4;
            break;
        case SCI_PLATFORM_REGION_TWN:
            keyboardLanguage = 5;
            break;
        default:
            keyboardLanguage = 2;
            break;
        }
    }
    createArg.regionType = static_cast<nn::swkbd::RegionType>(keyboardLanguage);
    if (SCIGetCafeLanguage(&keyboardRegion) != 1)
    {
        keyboardRegion = 0xff;
    }
    nn::swkbd::Create(createArg);
}
unsigned char CInput::IsShowingErrorViewer()
{
    return errorViewerVisible;
}
void CInput::SetErrorViewerMode(eErrorViewerProcessState value)
{
    errorViewerState = value;
}
void CInput::RequestErrorViewer(unsigned int value, unsigned int type, ErrorViewerCallback callback, void* context)
{
    if (errorViewerState == ERROR_VIEWER_NONE)
    {
        SetErrorViewerMode(ERROR_VIEWER_REQUESTED);
        errorViewerController = value;
        errorViewerContext = context;
        errorViewerMessage = 0;
        errorViewerCode = type;
        errorViewerTitle = 0;
        errorViewerCallback = callback;
    }
}
void CInput::RequestErrorViewer(unsigned int value, wchar_t* title, wchar_t* message, ErrorViewerCallback callback,
                                void* context)
{
    if (errorViewerState == ERROR_VIEWER_NONE)
    {
        SetErrorViewerMode(ERROR_VIEWER_REQUESTED);
        errorViewerContext = context;
        errorViewerController = value;
        errorViewerMessage = message;
        errorViewerCallback = callback;
        errorViewerTitle = title;
    }
}
void CInput::DestroyErrorViewer()
{
    if (errorViewerState == ERROR_VIEWER_ACTIVE)
    {
        SetErrorViewerMode(ERROR_VIEWER_DESTROYED);
    }
}
void CInput::ErrorViewerOnCreate()
{
    errorViewerVisible = 1;
    errorViewerState = ERROR_VIEWER_NONE;
    errorViewerMessage = 0;
    errorViewerController = 0;
    errorViewerTitle = 0;
}
void CInput::InitialiseErrorViewer()
{
    errorViewerFileClient = static_cast<FSClient*>(std::malloc(sizeof(FSClient)));
    FSAddClient(errorViewerFileClient, FS_ERROR_FLAG_NONE);
    nn::erreula::CreateArg createArg;
    keyboardErrorViewerWorkMemory = std::malloc(nn::erreula::GetWorkMemorySize());
    createArg.workMemory = keyboardErrorViewerWorkMemory;
    createArg.fsClient = errorViewerFileClient;
    createArg.region = static_cast<nn::erreula::RegionType>(0);
    createArg.language = static_cast<nn::erreula::LangType>(0);
    unsigned int platformRegion = SCI_PLATFORM_REGION_EUR;
    if (SCIGetPlatformRegion(&platformRegion) == 1)
    {
        createArg.region = platformRegion == SCI_PLATFORM_REGION_EUR
                               ? static_cast<nn::erreula::RegionType>(2)
                               : static_cast<nn::erreula::RegionType>(platformRegion == SCI_PLATFORM_REGION_USA);
    }
    unsigned int language;
    if (SCIGetCafeLanguage(&language) == 1)
    {
        createArg.language = static_cast<int>(language) > 11 ? static_cast<nn::erreula::LangType>(1)
                                                             : static_cast<nn::erreula::LangType>(language);
    }
    nn::erreula::Create(createArg);
}
unsigned int CInput::GetErrorViewerControllerType()
{
    static const unsigned char controllerTypes[5] = {0, 0, 1, 2, 3};
    return errorViewerController < 5 ? controllerTypes[errorViewerController] : errorViewerController;
}
void CInput::TickErrorViewer()
{
    if (errorViewerState == ERROR_VIEWER_REQUESTED)
    {
        nn::erreula::AppearArg appearArg;
        appearArg.errorArg.controllerType = static_cast<nn::erreula::ControllerType>(GetErrorViewerControllerType());
        appearArg.errorArg.renderTarget = static_cast<nn::erreula::RenderTarget>(2);
        if (errorViewerTitle == 0)
        {
            appearArg.errorArg.errorCode = errorViewerCode;
        }
        else
        {
            appearArg.errorArg.errorType = static_cast<nn::erreula::ErrorType>(2);
            appearArg.errorArg.errorTitle = reinterpret_cast<const char16_t*>(errorViewerTitle);
            appearArg.errorArg.errorMessage = reinterpret_cast<const char16_t*>(errorViewerMessage);
        }
        nn::erreula::AppearErrorViewer(appearArg);
        SetErrorViewerMode(ERROR_VIEWER_ACTIVE);
    }
    else if (errorViewerState == ERROR_VIEWER_ACTIVE)
    {
        nn::erreula::ControllerInfo controllerInfo;
        std::memset(&controllerInfo, 0, sizeof(controllerInfo));
        errorViewerVisible = 1;
        controllerInfo.vpad = (VPADStatus*)g_pInputInstance->GetVPadStatus();
        for (int pad = 1; pad < 5; ++pad)
        {
            controllerInfo.kpad[pad - 1] = (KPADStatus*)g_pInputInstance->GetKPadStatus(pad);
        }
        nn::erreula::Calc(controllerInfo);
        if (nn::erreula::IsDecideSelectButtonError() && static_cast<int>(nn::erreula::GetStateErrorViewer()) == 2)
        {
            nn::erreula::DisappearErrorViewer();
        }
        if (static_cast<int>(nn::erreula::GetStateErrorViewer()) != 0)
        {
            return;
        }

        unsigned int result = static_cast<unsigned int>(nn::erreula::GetResultType());
        if (result < 2)
        {
            SetErrorViewerMode(ERROR_VIEWER_NONE);
            if (errorViewerCallback == 0)
            {
                return;
            }
            errorViewerCallback(errorViewerContext);
            return;
        }
        if (result == 2)
        {
            nn::erreula::AppearArg appearArg;
            appearArg.errorArg.controllerType = static_cast<nn::erreula::ControllerType>(
                GetErrorViewerControllerType());
            appearArg.errorArg.renderTarget = static_cast<nn::erreula::RenderTarget>(2);
            appearArg.errorArg.errorCode = nn::erreula::GetResultCode();
            nn::erreula::AppearErrorViewer(appearArg);
            return;
        }
        if (result != 3)
        {
            return;
        }
        SetErrorViewerMode(ERROR_VIEWER_NONE);
        int jumpResult = nn::erreula::Jump(s_errorViewerJumpArgument, std::strlen(s_errorViewerJumpArgument) + 1);
        std::printf("nn::erreula::Jump returned %d\n", jumpResult);
        return;
    }
    else if (errorViewerState == ERROR_VIEWER_DESTROYED)
    {
        nn::erreula::DisappearErrorViewer();
        SetErrorViewerMode(ERROR_VIEWER_DISAPPEARING);
    }
    else if (errorViewerState == ERROR_VIEWER_DISAPPEARING && static_cast<int>(nn::erreula::GetStateErrorViewer()) == 0)
    {
        SetErrorViewerMode(ERROR_VIEWER_NONE);
    }
    nn::erreula::ControllerInfo controllerInfo;
    std::memset(&controllerInfo, 0, sizeof(controllerInfo));
    nn::erreula::Calc(controllerInfo);
}
void CInput::DebugPrintf(const char*, ...)
{
}

void CInput::JOYPAD::setConnected(bool connected)
{
    if (this->connected != connected)
    {
        this->connected = connected;
        newlyConnected = connected;
        disconnected = connected ^ 1;
        return;
    }

    disconnected = 0;
    newlyConnected = 0;
}
