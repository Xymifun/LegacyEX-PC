#ifndef GAME_UI_FUIBITMAPFONT_HPP
#define GAME_UI_FUIBITMAPFONT_HPP

#include <vector>

#include "game/ui/fuiFontSet.hpp"
#include "game/ui/UIBitmapFont.hpp"

class fuiBitmapFont : public fuiFontSet
{
public:
    struct GlyphScaleData
    {
        float fontSize;
        float scale;
    };

    fuiBitmapFont(SFontData& fontData, unsigned short missingGlyphChar);

    virtual int getTextureIndex(float fontSize, int ttfIndex);
    virtual float getGlyphScale(float fontSize);
    virtual void getGlyphTexPos(unsigned short codepoint, float& texX, float& texY);
    virtual void getGlyphWidthHeight(float& texWidth, float& texHeight);
    virtual float renderGlyph(unsigned short codepoint, float x, float y, float fontSize, unsigned int colour,
                              std::vector<fuiFontSet::FontVert> (&verts)[4]);
    virtual float getRenderedGlyphWidth(unsigned short codepoint, float fontSize);
    virtual float getRenderedGlyphHeight(unsigned short codepoint, float fontSize);
    virtual bool isBitmapFont();
    virtual void preRender();
    virtual int getFallbackFontTexture(float fontSize);
    virtual void loadGlyph(int codepoint, float fontSize);
    virtual bool isGlyphLoaded(int codepoint, float fontSize);
    virtual bool hasGlyph(int codepoint);

    void registerFont();
    void LoadTTFFallbackFont();

private:
    UIBitmapFont m_bitmapFont;
    SFontData* m_sFontData;
    unsigned int m_textureIndex;
    bool m_registered;
    unsigned short m_missingGlyphChar;
    std::vector<GlyphScaleData> m_glyphScaleData;
};

#endif
