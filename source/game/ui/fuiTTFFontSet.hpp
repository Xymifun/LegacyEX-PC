#ifndef GAME_UI_FUITTFFONTSET_HPP
#define GAME_UI_FUITTFFONTSET_HPP

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/ui/fuiFontSet.hpp"
#include "vendor/stb_truetype.hpp"

class fuiTTFFontSet : public fuiFontSet
{
public:
    class fuiTTFFont
    {
    public:
        const unsigned char* m_data;
        stbtt_fontinfo m_info;
        int m_ascent;
        int m_descent;
        int m_lineGap;
    };

    class GlyphInfo
    {
    public:
        int m_codepoint;
        int m_slot;
        int m_advanceWidth;
        float m_leftSideBearing;
        int m_x0;
        int m_y0;
        int m_x1;
        int m_y1;
        long long m_lastUsed;
    };

    class FontSizeData
    {
    public:
        FontSizeData(float fontSize, fuiTTFFontSet* fontSet, int ttfIndex);
        ~FontSizeData();

        int getTextureIndex();
        GlyphInfo* createGlyph(int codepoint);
        GlyphInfo* getGlyphInfo(int codepoint);
        void getGlyphTexPos(unsigned short codepoint, float& texX, float& texY);
        void getGlyphWidthHeight(float& texWidth, float& texHeight);
        float renderGlyph(unsigned short codepoint, float x, float y, unsigned int colour,
                          std::vector<fuiFontSet::FontVert> (&verts)[4]);
        void setPixel(int x, int y, unsigned char value);
        void loadGlyphImage(GlyphInfo* glyph);
        void loadGlyph(int codepoint);
        void uploadTextureData();

        int m_glyphWidth;
        int m_glyphHeight;
        int m_glyphsPerRow;
        int m_glyphsPerColumn;
        int m_textureWidth;
        int m_textureHeight;
        int m_maxGlyphs;
        boost::unordered_map<int, GlyphInfo*> m_glyphs;
        GlyphInfo** m_glyphSlots;
        float m_fontSize;
        float m_scale;
        int m_ascent;
        unsigned char* m_textureData;
        unsigned char* m_glyphBitmap;
        unsigned char* m_glyphBitmapRGBA;
        int m_textureIndex;
        fuiTTFFontSet* m_fontSet;
        int m_ttfIndex;
        long long m_glyphUseCounter;
        bool m_textureDirty;
    };

    fuiTTFFontSet(const char* name, const void* ttfData, unsigned short missingGlyphChar);
    fuiTTFFontSet(const char* name, int numTTFs, const void** ttfData, unsigned short missingGlyphChar);

    void init(const char* name, int numTTFs, const unsigned char** ttfData, unsigned short missingGlyphChar);
    int getTTFIndex(int codepoint);
    FontSizeData* getFontSizeData(float fontSize, int ttfIndex);
    int getStbGlyphIndex(unsigned short codepoint, int ttfIndex);
    void renderFontTexture();

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
    virtual ~fuiTTFFontSet();

    std::vector<fuiTTFFont> m_fonts;
    unsigned short m_missingGlyphChar;
    std::vector<FontSizeData*> m_fontSizeData;
};

#endif
