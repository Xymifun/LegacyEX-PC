#ifdef _WIN32

#include <windows.h>
#include <windowsx.h>

#include "platform/pc/PcOs.hpp"

#include <deque>

extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

namespace
{
const wchar_t kWindowClassName[] = L"MinecraftPCWindow";
const int kWglContextMajorVersionArb = 0x2091;
const int kWglContextMinorVersionArb = 0x2092;
const int kWglContextProfileMaskArb = 0x9126;
const int kWglContextFlagsArb = 0x2094;
const int kWglContextForwardCompatibleBitArb = 0x0002;
const int kWglContextCoreProfileBitArb = 0x00000001;
const unsigned short kHidUsagePageGeneric = 0x01;
const unsigned short kHidUsageMouse = 0x02;

typedef HGLRC(WINAPI* CreateContextAttribsProc)(HDC, HGLRC, const int*);
typedef BOOL(WINAPI* SwapIntervalProc)(int);

HWND s_window = 0;
HDC s_deviceContext = 0;
HGLRC s_glContext = 0;
HMODULE s_openGlModule = 0;
bool s_mouseCaptured = false;
bool s_fullscreen = false;
bool s_cursorHidden = false;
DWORD s_windowedStyle = 0;
WINDOWPLACEMENT s_windowedPlacement;
std::deque<PcOsWindowEvent> s_events;

PcOsWindowEvent MakeEvent(PcOsWindowEventType type)
{
    PcOsWindowEvent event;
    event.type = type;
    event.key = 0;
    event.down = false;
    event.x = 0;
    event.y = 0;
    event.delta = 0;
    event.character = 0;
    return event;
}

void PushMouseButton(PcOsMouseButton button, bool down, LPARAM lParam)
{
    PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_MouseButton);
    event.key = button;
    event.down = down;
    event.x = GET_X_LPARAM(lParam);
    event.y = GET_Y_LPARAM(lParam);
    s_events.push_back(event);
}

void ClipCursorToWindow()
{
    RECT rect;
    GetClientRect(s_window, &rect);
    POINT topLeft = {rect.left, rect.top};
    POINT bottomRight = {rect.right, rect.bottom};
    ClientToScreen(s_window, &topLeft);
    ClientToScreen(s_window, &bottomRight);
    RECT clip = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
    ClipCursor(&clip);
}

POINT ClientCentre()
{
    RECT rect;
    GetClientRect(s_window, &rect);
    POINT centre = {(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2};
    ClientToScreen(s_window, &centre);
    return centre;
}

void RecentreCursor()
{
    POINT centre = ClientCentre();
    POINT cursor;
    if (GetCursorPos(&cursor) && (cursor.x != centre.x || cursor.y != centre.y))
        SetCursorPos(centre.x, centre.y);
}

void ConfineCursor()
{
    ClipCursorToWindow();
    RecentreCursor();
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        s_events.push_back(MakeEvent(ePcOsWindowEvent_Close));
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_Key);
        event.key = static_cast<int>(wParam);
        event.down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
        s_events.push_back(event);
        bool firstPress = event.down && (lParam & (1 << 30)) == 0;
        bool altEnter = wParam == VK_RETURN && message == WM_SYSKEYDOWN && (lParam & (1 << 29)) != 0;
        if (firstPress && (wParam == VK_F11 || altEnter))
        {
            PcOsWindowSetFullscreen(!s_fullscreen);
            return 0;
        }
        if (wParam == VK_F10 || wParam == VK_MENU)
            return 0;
        break;
    }
    case WM_CHAR:
    {
        PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_Text);
        event.character = static_cast<unsigned int>(wParam);
        s_events.push_back(event);
        return 0;
    }
    case WM_MOUSEMOVE:
        if (!s_mouseCaptured)
        {
            PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_MouseMove);
            event.x = GET_X_LPARAM(lParam);
            event.y = GET_Y_LPARAM(lParam);
            s_events.push_back(event);
        }
        return 0;
    case WM_INPUT:
    {
        RAWINPUT input;
        UINT size = sizeof(input);
        if (s_mouseCaptured
            && GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER))
                   != static_cast<UINT>(-1)
            && input.header.dwType == RIM_TYPEMOUSE && (input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
        {
            PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_MouseMove);
            event.x = input.data.mouse.lLastX;
            event.y = input.data.mouse.lLastY;
            event.down = true;
            s_events.push_back(event);
            RecentreCursor();
        }
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        PushMouseButton(ePcOsMouseButton_Left, message == WM_LBUTTONDOWN, lParam);
        return 0;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        PushMouseButton(ePcOsMouseButton_Right, message == WM_RBUTTONDOWN, lParam);
        return 0;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        PushMouseButton(ePcOsMouseButton_Middle, message == WM_MBUTTONDOWN, lParam);
        return 0;
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        PushMouseButton(GET_XBUTTON_WPARAM(wParam) == XBUTTON2 ? ePcOsMouseButton_X2 : ePcOsMouseButton_X1,
                        message == WM_XBUTTONDOWN, lParam);
        return TRUE;
    case WM_MOUSEWHEEL:
    {
        PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_MouseWheel);
        event.delta = GET_WHEEL_DELTA_WPARAM(wParam);
        s_events.push_back(event);
        return 0;
    }
    case WM_ACTIVATE:
    {
        PcOsWindowEvent event = MakeEvent(ePcOsWindowEvent_Focus);
        event.down = LOWORD(wParam) != WA_INACTIVE;
        s_events.push_back(event);
        if (event.down && s_mouseCaptured)
            ConfineCursor();
        break;
    }
    case WM_SIZE:
    case WM_MOVE:
        if (s_mouseCaptured)
            ConfineCursor();
        break;
    case WM_SETCURSOR:
        if ((s_mouseCaptured || s_cursorHidden) && LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(0);
            return TRUE;
        }
        break;
    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

bool CreateGlContext()
{
    PIXELFORMATDESCRIPTOR format;
    ZeroMemory(&format, sizeof(format));
    format.nSize = sizeof(format);
    format.nVersion = 1;
    format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cDepthBits = 24;
    format.cStencilBits = 8;
    format.iLayerType = PFD_MAIN_PLANE;
    int chosen = ChoosePixelFormat(s_deviceContext, &format);
    if (chosen == 0 || !SetPixelFormat(s_deviceContext, chosen, &format))
        return false;

    HGLRC legacy = wglCreateContext(s_deviceContext);
    if (legacy == 0 || !wglMakeCurrent(s_deviceContext, legacy))
        return false;
    CreateContextAttribsProc createContextAttribs = reinterpret_cast<CreateContextAttribsProc>(
        reinterpret_cast<void*>(wglGetProcAddress("wglCreateContextAttribsARB")));
    if (createContextAttribs == 0)
    {
        s_glContext = legacy;
        return true;
    }
    const int attributes[] = {kWglContextMajorVersionArb, 3, kWglContextMinorVersionArb, 3, kWglContextProfileMaskArb,
                              kWglContextCoreProfileBitArb, 0};
    HGLRC core = createContextAttribs(s_deviceContext, 0, attributes);
    if (core == 0)
    {
        const int forwardCompatible[] = {kWglContextMajorVersionArb, 3, kWglContextMinorVersionArb, 3,
                                         kWglContextProfileMaskArb, kWglContextCoreProfileBitArb, kWglContextFlagsArb,
                                         kWglContextForwardCompatibleBitArb, 0};
        core = createContextAttribs(s_deviceContext, 0, forwardCompatible);
        if (core != 0)
            printf("[pc] gl: using a forward-compatible 3.3 core context\n");
    }
    wglMakeCurrent(0, 0);
    wglDeleteContext(legacy);
    if (core == 0 || !wglMakeCurrent(s_deviceContext, core))
        return false;
    s_glContext = core;
    return true;
}
}

bool PcOsWindowCreate(int width, int height, const char* title, bool visible, bool maximized)
{
    HINSTANCE instance = GetModuleHandleW(0);
    WNDCLASSEXW windowClass;
    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(0, reinterpret_cast<LPCWSTR>(IDC_ARROW));
    windowClass.hIcon =
        static_cast<HICON>(LoadImageW(instance, MAKEINTRESOURCEW(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    windowClass.hIconSm = static_cast<HICON>(LoadImageW(instance, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                                        0));
    windowClass.lpszClassName = kWindowClassName;
    if (RegisterClassExW(&windowClass) == 0)
        return false;

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);
    wchar_t wideTitle[128];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wideTitle, 128);
    s_window = CreateWindowExW(0, kWindowClassName, wideTitle, style, CW_USEDEFAULT, CW_USEDEFAULT,
                               rect.right - rect.left, rect.bottom - rect.top, 0, 0, instance, 0);
    if (s_window == 0)
        return false;
    s_deviceContext = GetDC(s_window);
    if (!CreateGlContext())
        return false;
    s_openGlModule = LoadLibraryW(L"opengl32.dll");

    RAWINPUTDEVICE mouse;
    mouse.usUsagePage = kHidUsagePageGeneric;
    mouse.usUsage = kHidUsageMouse;
    mouse.dwFlags = 0;
    mouse.hwndTarget = s_window;
    RegisterRawInputDevices(&mouse, 1, sizeof(mouse));

    if (visible)
    {
        ShowWindow(s_window, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
        UpdateWindow(s_window);
    }
    return true;
}

bool PcOsWindowPollEvent(PcOsWindowEvent* event)
{
    if (s_events.empty())
    {
        MSG message;
        while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    if (s_events.empty())
        return false;
    *event = s_events.front();
    s_events.pop_front();
    return true;
}

void PcOsWindowSwapBuffers()
{
    SwapBuffers(s_deviceContext);
}

void PcOsWindowGetClientSize(int* width, int* height)
{
    RECT rect;
    GetClientRect(s_window, &rect);
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}

void PcOsWindowSetMouseCaptured(bool captured)
{
    if (captured == s_mouseCaptured)
        return;
    s_mouseCaptured = captured;
    ShowCursor(captured ? FALSE : TRUE);
    if (captured)
        ConfineCursor();
    else
        ClipCursor(0);
}

void PcOsWindowSetCursorHidden(bool hidden)
{
    if (hidden == s_cursorHidden)
        return;
    s_cursorHidden = hidden;
    POINT cursor;
    RECT client;
    if (s_window != 0 && GetCursorPos(&cursor) && ScreenToClient(s_window, &cursor) && GetClientRect(s_window, &client)
        && PtInRect(&client, cursor))
        SetCursor(hidden ? 0 : LoadCursorW(0, reinterpret_cast<LPCWSTR>(IDC_ARROW)));
}

void PcOsWindowSetFullscreen(bool fullscreen)
{
    if (s_window == 0 || fullscreen == s_fullscreen)
        return;
    if (fullscreen)
    {
        MONITORINFO monitor;
        monitor.cbSize = sizeof(monitor);
        if (!GetMonitorInfoW(MonitorFromWindow(s_window, MONITOR_DEFAULTTONEAREST), &monitor))
            return;
        s_windowedStyle = static_cast<DWORD>(GetWindowLongW(s_window, GWL_STYLE));
        s_windowedPlacement.length = sizeof(s_windowedPlacement);
        GetWindowPlacement(s_window, &s_windowedPlacement);
        SetWindowLongW(s_window, GWL_STYLE, static_cast<LONG>((s_windowedStyle & ~WS_OVERLAPPEDWINDOW) | WS_POPUP));
        const RECT& area = monitor.rcMonitor;
        SetWindowPos(s_window, HWND_TOP, area.left, area.top, area.right - area.left, area.bottom - area.top,
                     SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
    }
    else
    {
        SetWindowLongW(s_window, GWL_STYLE, static_cast<LONG>(s_windowedStyle));
        SetWindowPlacement(s_window, &s_windowedPlacement);
        SetWindowPos(s_window, 0, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
    }
    s_fullscreen = fullscreen;
    if (s_mouseCaptured)
        ConfineCursor();
}

void PcOsWindowSetSwapInterval(int interval)
{
    SwapIntervalProc swapInterval =
        reinterpret_cast<SwapIntervalProc>(reinterpret_cast<void*>(wglGetProcAddress("wglSwapIntervalEXT")));
    if (swapInterval != 0)
        swapInterval(interval);
}

std::wstring PcOsClipboardText()
{
    std::wstring text;
    if (!OpenClipboard(s_window))
        return text;
    HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data != 0)
    {
        const wchar_t* characters = static_cast<const wchar_t*>(GlobalLock(data));
        if (characters != 0)
        {
            text = characters;
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
    return text;
}

void* PcOsGlGetProcAddress(const char* name)
{
    void* address = reinterpret_cast<void*>(wglGetProcAddress(name));
    if (address == 0 || address == reinterpret_cast<void*>(1) || address == reinterpret_cast<void*>(2)
        || address == reinterpret_cast<void*>(3) || address == reinterpret_cast<void*>(-1))
        address = s_openGlModule != 0 ? reinterpret_cast<void*>(GetProcAddress(s_openGlModule, name)) : 0;
    return address;
}

#endif
