#ifndef GAME_CORE_GUIMESSAGE_HPP
#define GAME_CORE_GUIMESSAGE_HPP

#include <string>

class GuiMessage
{
public:
    GuiMessage(const std::wstring& message);

    std::wstring message;
    int age;
};

#endif
