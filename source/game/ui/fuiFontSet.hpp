#ifndef GAME_UI_FUIFONTSET_HPP
#define GAME_UI_FUIFONTSET_HPP

#include <string>
#include <vector>

class fuiFontSet
{
public:
    struct FontVert
    {
        float x;
        float y;
        float u;
        float v;
        unsigned int colour;
    };

    fuiFontSet(const char* name) : m_name(name) {}

    virtual int getTextureIndex(float fontSize, int ttfIndex) = 0;
    virtual float getGlyphScale(float fontSize) = 0;
    virtual void getGlyphTexPos(unsigned short codepoint, float& texX, float& texY) = 0;
    virtual void getGlyphWidthHeight(float& texWidth, float& texHeight) = 0;
    virtual float renderGlyph(unsigned short codepoint, float x, float y, float fontSize, unsigned int colour,
                              std::vector<FontVert> (&verts)[4]) = 0;
    virtual float getRenderedGlyphWidth(unsigned short codepoint, float fontSize) = 0;
    virtual float getRenderedGlyphHeight(unsigned short codepoint, float fontSize) = 0;
    virtual bool isBitmapFont() = 0;
    virtual void preRender() = 0;
    virtual int getFallbackFontTexture(float fontSize) = 0;
    virtual void loadGlyph(int codepoint, float fontSize) = 0;
    virtual bool isGlyphLoaded(int codepoint, float fontSize) = 0;
    virtual bool hasGlyph(int codepoint) = 0;

    std::string m_name;
};

#endif
