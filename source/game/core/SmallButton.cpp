#include "game/core/SmallButton.hpp"

SmallButton::SmallButton(int id, int x, int y, const std::wstring& message)
    : Button(id, x, y, 150, 20, message),
      mOption(NULL)
{
}

SmallButton::SmallButton(int id, int x, int y, int width, int height, const std::wstring& message)
    : Button(id, x, y, width, height, message),
      mOption(NULL)
{
}

SmallButton::SmallButton(int id, int x, int y, const Options::Option* option, const std::wstring& message)
    : Button(id, x, y, 150, 20, message),
      mOption(option)
{
}

const Options::Option* SmallButton::getOption()
{
    return mOption;
}
