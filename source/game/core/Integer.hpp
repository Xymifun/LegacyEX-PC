#ifndef GAME_CORE_INTEGER_HPP
#define GAME_CORE_INTEGER_HPP

#include <string>

class Integer
{
public:
    static int parseInt(const std::wstring& value, int radix);
};

#endif
