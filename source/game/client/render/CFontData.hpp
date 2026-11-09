#ifndef GAME_CLIENT_RENDER_CFONTDATA_HPP
#define GAME_CLIENT_RENDER_CFONTDATA_HPP

#include <string>
#include <boost/unordered_map.hpp>

#include "game/client/render/SFontData.hpp"

class CFontData
{
public:
    CFontData();
    CFontData(SFontData& fontData, int* imageData);
    void release();
    SFontData& getFontData();
    std::string getFontName();
    unsigned short getGlyphId(unsigned int unicode);
    unsigned int getUnicode(unsigned short glyphId);
    void getPos(unsigned short glyphId, int& row, int& column);
    unsigned char* topLeftPixel(int row, int column);
    void moveCursor(unsigned char*& cursor, unsigned int x, unsigned int y);
    unsigned short getWidth(unsigned short glyphId);
    float getAdvance(unsigned short glyphId);
    bool unicodeIsWhitespace(unsigned int unicode);
    bool glyphIsWhitespace(unsigned short glyphId);

private:
    SFontData* m_fontData;
    boost::unordered_map<unsigned int, unsigned short> m_glyphIds;
    unsigned short* m_glyphWidths;
    unsigned char* m_glyphPixels;
    float* m_glyphAdvances;
};

extern SFontData g_mojangles7FontData;
extern SFontData g_mojangles11FontData;

#endif
