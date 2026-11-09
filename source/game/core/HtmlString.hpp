#ifndef GAME_CORE_HTMLSTRING_HPP
#define GAME_CORE_HTMLSTRING_HPP

#include <string>
#include <vector>

#include "game/core/ColourTable.hpp"

class HtmlString
{
public:
    HtmlString(const HtmlString& other);
    HtmlString(std::wstring text, eMinecraftColour colour, bool italic, bool indent, int fontSize);

    std::wstring ToString();

    static std::wstring Compose(std::vector<HtmlString>* lines);

public:
    std::wstring mText;
    eMinecraftColour mColour;
    bool mItalic;
    bool mIndent;
    int mFontSize;
};

#endif
