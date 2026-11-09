#include "game/core/StringFormat.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>

std::wstring formatwstr(const wchar_t* format, ...)
{
    wchar_t buffer[256];
    std::memset(buffer, 0, sizeof(buffer));
    va_list arguments;
    va_start(arguments, format);
    std::vswprintf(buffer, 256, format, arguments);
    va_end(arguments);
    return std::wstring(buffer);
}

std::string formatstr(const char* format, ...)
{
    char buffer[256];
    std::memset(buffer, 0, sizeof(buffer));
    va_list arguments;
    va_start(arguments, format);
    std::vsprintf(buffer, format, arguments);
    va_end(arguments);
    return std::string(buffer);
}
