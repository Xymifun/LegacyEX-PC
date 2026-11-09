#ifndef GAME_CORE_CONSOLEINPUTSOURCE_HPP
#define GAME_CORE_CONSOLEINPUTSOURCE_HPP

#include <string>

class ConsoleInputSource
{
public:
    virtual ~ConsoleInputSource() {}
    virtual void info(const std::wstring& message) = 0;
    virtual void warn(const std::wstring& message) = 0;
    virtual std::wstring getConsoleName() = 0;
};

#endif
