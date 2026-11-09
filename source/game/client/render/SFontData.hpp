#ifndef GAME_CLIENT_RENDER_SFONTDATA_HPP
#define GAME_CLIENT_RENDER_SFONTDATA_HPP

#include <string>

struct SFontData
{
    std::string fontName;
    std::wstring textureFileName;
    unsigned int numGlyphs;
    const unsigned short* glyphUnicodes;
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int glyphsPerRow;
    unsigned int glyphRows;
    unsigned int glyphWidth;
    unsigned int glyphHeight;
    float charHeightRatio;
    float charWidthRatio;
    float glyphWidthScale;
    unsigned int spaceWidth;

    ~SFontData();
};

#endif
