#include "game/core/Integer.hpp"

#include <cwchar>

int Integer::parseInt(const std::wstring& value, int radix)
{
    return static_cast<int>(wcstol(value.c_str(), NULL, radix));
}
