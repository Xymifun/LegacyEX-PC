#include "game/core/DecimalFormat.hpp"

#include <wchar.h>

std::wstring DecimalFormat::format(double value)
{
    wchar_t buffer[256];

    swprintf(buffer, 256, pattern.c_str(), value);

    std::wstring formatted(buffer);
    return formatted;
}
