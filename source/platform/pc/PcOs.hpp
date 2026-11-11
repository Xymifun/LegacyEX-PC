#ifndef PLATFORM_PC_PCOS_HPP
#define PLATFORM_PC_PCOS_HPP

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include <string>
#include <vector>

struct PcOsCalendar
{
    int year;
    int month;
    int dayOfWeek;
    int day;
    int hour;
    int minute;
    int second;
};

unsigned long long PcOsMonotonicNanoseconds();
unsigned long long PcOsRealTimeMilliseconds();
void PcOsCalendarFromUnixSeconds(long long seconds, bool local, PcOsCalendar* calendar);
void PcOsSleepMilliseconds(unsigned int milliseconds);
bool PcOsRandomBytes(void* buffer, size_t size);
void PcOsUsePreciseTimers();
void PcOsPrepareOutput(bool ownWindow, bool keepConsole);
[[noreturn]] void PcOsExitImmediately(int code);

void* PcOsAllocPages(void* address, size_t size);
bool PcOsFreePages(void* address, size_t size);
void PcOsPhysicalMemory(unsigned long long* total, unsigned long long* available);

void PcOsSetCurrentThreadName(const char* name);
uintptr_t PcOsProgramBase();
clockid_t PcOsConditionClock();

const size_t kPcOsLockBytes = 24;

void PcOsLockAcquire(void* lock);
bool PcOsLockTryAcquire(void* lock);
void PcOsLockRelease(void* lock);

bool PcOsMakeDirectory(const char* path);
bool PcOsReplaceFile(const char* from, const char* to);
bool PcOsRemoveFile(const char* path);
bool PcOsRemoveDirectory(const char* path);
void PcOsInstallStopHandler(void (*handler)());
std::string PcOsUserDataDirectory();
bool PcOsFileSystemIsCaseSensitive();

typedef long long PcOsSocket;
const PcOsSocket kPcOsInvalidSocket = -1;
const unsigned int kPcOsLoopbackAddress = 0x7F000001;
const unsigned int kPcOsBroadcastAddress = 0xFFFFFFFF;

bool PcOsNetInitialise();
size_t PcOsListIPv4Broadcasts(unsigned int* addresses, size_t capacity);
size_t PcOsListIPv4Addresses(unsigned int* addresses, size_t capacity);
bool PcOsParseIPv4(const char* text, unsigned int* address);
bool PcOsResolveIPv4(const char* host, unsigned int* address);
void PcOsFormatIPv4(unsigned int address, char* text, size_t size);
PcOsSocket PcOsTcpListen(unsigned short port);
PcOsSocket PcOsTcpAccept(PcOsSocket listener, unsigned int* address);
PcOsSocket PcOsTcpConnect(unsigned int address, unsigned short port);
bool PcOsSocketSendAll(PcOsSocket socket, const void* data, size_t size);
int PcOsSocketReceive(PcOsSocket socket, void* buffer, size_t size);
int PcOsSocketPeek(PcOsSocket socket, void* buffer, size_t size);
int PcOsSocketWaitReadable(PcOsSocket socket, unsigned int timeoutMilliseconds);
void PcOsSocketSetSendTimeout(PcOsSocket socket, unsigned int timeoutMilliseconds);
void PcOsSocketShutdown(PcOsSocket socket);
void PcOsSocketClose(PcOsSocket socket);
PcOsSocket PcOsUdpOpen(unsigned short port);
bool PcOsUdpSendTo(PcOsSocket socket, unsigned int address, unsigned short port, const void* data, size_t size);
int PcOsUdpReceiveFrom(PcOsSocket socket, void* buffer, size_t size, unsigned int* address,
                       unsigned int timeoutMilliseconds);

enum PcOsWindowEventType
{
    ePcOsWindowEvent_Close,
    ePcOsWindowEvent_Key,
    ePcOsWindowEvent_Text,
    ePcOsWindowEvent_MouseMove,
    ePcOsWindowEvent_MouseButton,
    ePcOsWindowEvent_MouseWheel,
    ePcOsWindowEvent_Focus
};

enum PcOsMouseButton
{
    ePcOsMouseButton_Left,
    ePcOsMouseButton_Right,
    ePcOsMouseButton_Middle,
    ePcOsMouseButton_X1,
    ePcOsMouseButton_X2,
    ePcOsMouseButton_Count
};

struct PcOsWindowEvent
{
    PcOsWindowEventType type;
    int key;
    bool down;
    int x;
    int y;
    int delta;
    unsigned int character;
};

typedef void (*PcOsAudioFillFn)(short* frames, unsigned int count);

bool PcOsAudioOpen(unsigned int rate, unsigned int framesPerBuffer, PcOsAudioFillFn fill);

struct PcOsMicDevice
{
    std::string id;
    std::string name;
};

typedef void (*PcOsMicDeliverFn)(const float* samples, unsigned int count);

void PcOsMicListDevices(std::vector<PcOsMicDevice>* devices);
bool PcOsMicOpen(const std::string& id, unsigned int rate, PcOsMicDeliverFn deliver, std::string* openedName);
void PcOsMicClose();

enum PcOsGamepadButton
{
    ePcOsGamepad_South = 1 << 0,
    ePcOsGamepad_East = 1 << 1,
    ePcOsGamepad_West = 1 << 2,
    ePcOsGamepad_North = 1 << 3,
    ePcOsGamepad_LeftShoulder = 1 << 4,
    ePcOsGamepad_RightShoulder = 1 << 5,
    ePcOsGamepad_LeftStick = 1 << 6,
    ePcOsGamepad_RightStick = 1 << 7,
    ePcOsGamepad_Back = 1 << 8,
    ePcOsGamepad_Start = 1 << 9,
    ePcOsGamepad_Guide = 1 << 10,
    ePcOsGamepad_Up = 1 << 11,
    ePcOsGamepad_Down = 1 << 12,
    ePcOsGamepad_Left = 1 << 13,
    ePcOsGamepad_Right = 1 << 14,
    ePcOsGamepad_Touchpad = 1 << 15
};

const size_t kPcOsGamepadMax = 8;

enum PcOsGamepadKind
{
    ePcOsGamepadKind_XboxOne,
    ePcOsGamepadKind_Xbox360,
    ePcOsGamepadKind_PS4,
    ePcOsGamepadKind_PS3
};

struct PcOsGamepadState
{
    unsigned int id;
    PcOsGamepadKind kind;
    unsigned int buttons;
    float leftX;
    float leftY;
    float rightX;
    float rightY;
    float leftTrigger;
    float rightTrigger;
    char name[48];
};

size_t PcOsGamepadPoll(PcOsGamepadState* states, size_t capacity);
void PcOsGamepadSetPlayer(unsigned int id, int player);

bool PcOsWindowCreate(int width, int height, const char* title, bool visible);
bool PcOsWindowPollEvent(PcOsWindowEvent* event);
void PcOsWindowSwapBuffers();
void PcOsWindowGetClientSize(int* width, int* height);
void PcOsWindowSetMouseCaptured(bool captured);
void PcOsWindowSetFullscreen(bool fullscreen);
void PcOsWindowSetCursorHidden(bool hidden);
void PcOsWindowSetSwapInterval(int interval);
std::wstring PcOsClipboardText();
void* PcOsGlGetProcAddress(const char* name);

#endif
