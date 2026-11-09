#ifndef GAME_CORE_STRINGBUILDERCORE_HPP
#define GAME_CORE_STRINGBUILDERCORE_HPP

#include <string>

class StringBuilderCore
{
public:
    virtual ~StringBuilderCore() {}

    virtual std::wstring getString() = 0;
};

#endif
