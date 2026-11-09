#ifndef GAME_CORE_DECIMALFORMAT_HPP
#define GAME_CORE_DECIMALFORMAT_HPP

#include <string>

class DecimalFormat
{
public:
    DecimalFormat(const std::wstring& pattern) : pattern(pattern) {}

    std::wstring format(double value);

protected:
    std::wstring pattern;
};

#endif
