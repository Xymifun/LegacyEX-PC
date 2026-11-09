#ifndef GAME_UI_UIBITMAPFONT_HPP
#define GAME_UI_UIBITMAPFONT_HPP

#include "game/ui/UIAbstractBitmapFont.hpp"

class CFontData;
struct SFontData;

class UIBitmapFont : public UIAbstractBitmapFont
{
private:
    CFontData* m_fontData;

public:
    UIBitmapFont(SFontData& fontData);
    ~UIBitmapFont();

    CFontData* getFontData();
};

#endif
