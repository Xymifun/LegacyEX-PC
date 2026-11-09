#ifndef GAME_CORE_STATFORMATTER_HPP
#define GAME_CORE_STATFORMATTER_HPP

#include <string>

class StatFormatter
{
public:
    virtual ~StatFormatter() {}

    virtual std::wstring format(int value) = 0;
};

#endif
