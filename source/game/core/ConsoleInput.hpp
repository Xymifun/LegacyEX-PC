#ifndef GAME_CORE_CONSOLEINPUT_HPP
#define GAME_CORE_CONSOLEINPUT_HPP

#include <string>

class ConsoleInputSource;

class ConsoleInput
{
public:
    ConsoleInput(const std::wstring& input, ConsoleInputSource* source);
};

#endif
