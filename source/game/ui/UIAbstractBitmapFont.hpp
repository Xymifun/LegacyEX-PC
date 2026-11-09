#ifndef GAME_UI_UIABSTRACTBITMAPFONT_HPP
#define GAME_UI_UIABSTRACTBITMAPFONT_HPP

#include <string>

class UIAbstractBitmapFont
{
public:
    UIAbstractBitmapFont(const std::string& fontName);
    ~UIAbstractBitmapFont();

    void registerFont();

protected:
    std::string m_fontName;
    bool m_bRegistered;
    unsigned int m_numGlyphs;
};

#endif
