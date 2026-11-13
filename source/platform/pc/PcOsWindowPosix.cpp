#ifndef _WIN32

#include "platform/pc/PcOs.hpp"

bool PcOsWindowCreate(int, int, const char*, bool, bool)
{
    return false;
}

bool PcOsWindowPollEvent(PcOsWindowEvent*)
{
    return false;
}

void PcOsWindowSwapBuffers()
{
}

void PcOsWindowGetClientSize(int* width, int* height)
{
    *width = 0;
    *height = 0;
}

void PcOsWindowSetMouseCaptured(bool)
{
}

void PcOsWindowSetFullscreen(bool)
{
}

void PcOsWindowSetCursorHidden(bool)
{
}

void PcOsWindowSetSwapInterval(int)
{
}

std::wstring PcOsClipboardText()
{
    return std::wstring();
}

void* PcOsGlGetProcAddress(const char*)
{
    return 0;
}

#endif
