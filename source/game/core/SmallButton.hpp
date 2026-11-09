#ifndef GAME_CORE_SMALLBUTTON_HPP
#define GAME_CORE_SMALLBUTTON_HPP

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Options.hpp"

class SmallButton : public Button
{
public:
    SmallButton(int id, int x, int y, const std::wstring& message);
    SmallButton(int id, int x, int y, int width, int height, const std::wstring& message);
    SmallButton(int id, int x, int y, const Options::Option* option, const std::wstring& message);

    const Options::Option* getOption();

private:
    const Options::Option* mOption;
};

#endif
