#include "game/ui/fuiTTFFontSet.hpp"

#include <math.h>
#include <string.h>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/util/Math.hpp"

fuiTTFFontSet::fuiTTFFontSet(const char* name, const void* ttfData, unsigned short missingGlyphChar) : fuiFontSet(name)
{
    const unsigned char* data = (const unsigned char*)ttfData;
    init(name, 1, &data, missingGlyphChar);
}

fuiTTFFontSet::fuiTTFFontSet(const char* name, int numTTFs, const void** ttfData, unsigned short missingGlyphChar)
    : fuiFontSet(name)
{
    init(name, numTTFs, (const unsigned char**)ttfData, missingGlyphChar);
}

fuiTTFFontSet::~fuiTTFFontSet()
{
    for (unsigned int i = 0; i < m_fonts.size(); i++)
    {
        delete m_fonts[i].m_data;
    }

    m_fonts.clear();

    for (unsigned int i = 0; i < m_fontSizeData.size(); i++)
    {
        delete m_fontSizeData[i];
    }

    m_fontSizeData.clear();
}

void fuiTTFFontSet::init(const char* name, int numTTFs, const unsigned char** ttfData, unsigned short missingGlyphChar)
{
    m_missingGlyphChar = missingGlyphChar;

    for (int i = 0; i < numTTFs; i++)
    {
        fuiTTFFont font;

        font.m_data = ttfData[i];

        stbtt_InitFont(&font.m_info, font.m_data, 0);
        stbtt_GetFontVMetrics(&font.m_info, &font.m_ascent, &font.m_descent, &font.m_lineGap);

        m_fonts.push_back(font);
    }
}

int fuiTTFFontSet::getTTFIndex(int codepoint)
{
    for (unsigned int i = 0; i < m_fonts.size(); i++)
    {
        if (stbtt_FindGlyphIndex(&m_fonts[i].m_info, codepoint) != 0)
        {
            return i;
        }
    }

    return -1;
}

fuiTTFFontSet::FontSizeData* fuiTTFFontSet::getFontSizeData(float fontSize, int ttfIndex)
{
    for (unsigned int i = 0; i < m_fontSizeData.size(); i++)
    {
        if (m_fontSizeData[i]->m_fontSize == (int)fontSize && m_fontSizeData[i]->m_ttfIndex == ttfIndex)
        {
            return m_fontSizeData[i];
        }
    }

    FontSizeData* fontSizeData = new FontSizeData(fontSize, this, ttfIndex);

    m_fontSizeData.push_back(fontSizeData);

    return fontSizeData;
}

int fuiTTFFontSet::getStbGlyphIndex(unsigned short codepoint, int ttfIndex)
{
    return stbtt_FindGlyphIndex(&m_fonts[ttfIndex].m_info, codepoint);
}

int fuiTTFFontSet::getTextureIndex(float fontSize, int ttfIndex)
{
    return getFontSizeData(fontSize, ttfIndex)->getTextureIndex();
}

float fuiTTFFontSet::getGlyphScale(float fontSize)
{
    return 1.0f;
}

void fuiTTFFontSet::getGlyphTexPos(unsigned short codepoint, float& texX, float& texY)
{
}

void fuiTTFFontSet::getGlyphWidthHeight(float& texWidth, float& texHeight)
{
}

float fuiTTFFontSet::renderGlyph(unsigned short codepoint, float x, float y, float fontSize, unsigned int colour,
                                 std::vector<fuiFontSet::FontVert> (&verts)[4])
{
    int ttfIndex = getTTFIndex(codepoint);

    if (ttfIndex == -1)
    {
        ttfIndex = 0;
    }

    return getFontSizeData(fontSize, ttfIndex)->renderGlyph(codepoint, x, y, colour, verts);
}

float fuiTTFFontSet::getRenderedGlyphWidth(unsigned short codepoint, float fontSize)
{
    int ttfIndex = getTTFIndex(codepoint);

    if (ttfIndex == -1)
    {
        ttfIndex = 0;
    }

    FontSizeData* fontSizeData = getFontSizeData(fontSize, ttfIndex);
    float glyphScale = getGlyphScale(fontSize);

    GlyphInfo* glyph = fontSizeData->getGlyphInfo(codepoint);

    if (glyph != NULL)
    {
        return (float)glyph->m_advanceWidth * (fontSizeData->m_scale * glyphScale);
    }

    return 0.0f;
}

float fuiTTFFontSet::getRenderedGlyphHeight(unsigned short codepoint, float fontSize)
{
    int ttfIndex = getTTFIndex(codepoint);

    if (ttfIndex == -1)
    {
        ttfIndex = 0;
    }

    float glyphScale = getGlyphScale(fontSize);
    FontSizeData* fontSizeData = getFontSizeData(fontSize, ttfIndex);
    const fuiTTFFont& font = m_fonts[ttfIndex];
    float height = (float)(font.m_ascent - font.m_descent + font.m_lineGap);

    return height * fontSizeData->m_scale * glyphScale;
}

bool fuiTTFFontSet::isBitmapFont()
{
    return false;
}

void fuiTTFFontSet::preRender()
{
    for (unsigned int i = 0; i < m_fontSizeData.size(); i++)
    {
        m_fontSizeData[i]->uploadTextureData();
    }
}

int fuiTTFFontSet::getFallbackFontTexture(float fontSize)
{
    return -1;
}

void fuiTTFFontSet::loadGlyph(int codepoint, float fontSize)
{
    int ttfIndex = getTTFIndex(codepoint);

    if (ttfIndex == -1)
    {
        ttfIndex = 0;
    }

    getFontSizeData(fontSize, ttfIndex)->loadGlyph(codepoint);
}

bool fuiTTFFontSet::isGlyphLoaded(int codepoint, float fontSize)
{
    int ttfIndex = getTTFIndex(codepoint);

    if (ttfIndex == -1)
    {
        ttfIndex = 0;
    }

    GlyphInfo* glyph = getFontSizeData(fontSize, ttfIndex)->getGlyphInfo(codepoint);

    return glyph != NULL && glyph->m_slot >= 0;
}

bool fuiTTFFontSet::hasGlyph(int codepoint)
{
    if (m_missingGlyphChar == 0)
    {
        return getTTFIndex(codepoint) != -1;
    }

    return true;
}

void fuiTTFFontSet::renderFontTexture()
{
}

fuiTTFFontSet::FontSizeData::FontSizeData(float fontSize, fuiTTFFontSet* fontSet, int ttfIndex) : m_glyphs(11)
{
    m_fontSize = fontSize;
    m_fontSet = fontSet;
    m_ttfIndex = ttfIndex;

    fuiTTFFont& font = fontSet->m_fonts[ttfIndex];

    m_scale = stbtt_ScaleForPixelHeight(&font.m_info, fontSize);
    m_ascent = (int)((float)font.m_ascent * m_scale);

    m_glyphWidth = Math::roundUpToPowerOf2(
        (int)((float)(fontSet->m_fonts[ttfIndex].m_ascent - fontSet->m_fonts[ttfIndex].m_descent) * m_scale));
    m_glyphHeight = m_glyphWidth;
    m_glyphsPerRow = 20;
    m_glyphsPerColumn = 20;
    m_textureWidth = m_glyphWidth * m_glyphsPerRow;
    m_textureHeight = m_glyphHeight * m_glyphsPerColumn;

    m_glyphBitmap = new unsigned char[m_glyphWidth * m_glyphHeight];
    m_glyphBitmapRGBA = new unsigned char[m_glyphWidth * m_glyphHeight * 4];

    m_maxGlyphs = m_glyphsPerRow * m_glyphsPerColumn;
    m_glyphSlots = new GlyphInfo*[m_maxGlyphs];
    memset(m_glyphSlots, 0, m_maxGlyphs * sizeof(GlyphInfo*));

    m_glyphUseCounter = 0;
    m_textureDirty = false;

    size_t textureBytes = m_textureWidth * m_textureHeight * 4;
    m_textureData = new unsigned char[textureBytes];
    memset(m_textureData, 0, textureBytes);

    m_textureIndex = -1;
}

fuiTTFFontSet::FontSizeData::~FontSizeData()
{
    for (boost::unordered_map<int, GlyphInfo*>::iterator it = m_glyphs.begin(); it != m_glyphs.end(); ++it)
    {
        delete it->second;
    }

    m_glyphs.clear();

    delete[] m_glyphBitmap;
    m_glyphBitmap = NULL;

    delete[] m_glyphBitmapRGBA;
    m_glyphBitmapRGBA = NULL;

    delete[] m_glyphSlots;
    m_glyphSlots = NULL;

    delete[] m_textureData;
    m_textureData = NULL;
}

int fuiTTFFontSet::FontSizeData::getTextureIndex()
{
    if (m_textureIndex == -1)
    {
        m_textureIndex = gRenderer->TextureCreate();
        gRenderer->TextureBind(m_textureIndex);
        gRenderer->TextureData(m_textureWidth, m_textureHeight, m_textureData, 0, C4JRender::eTextureFormat_RGBA8);
    }

    return m_textureIndex;
}

fuiTTFFontSet::GlyphInfo* fuiTTFFontSet::FontSizeData::createGlyph(int codepoint)
{
    float xPos = 2.0f;
    float shiftX = xPos - floorf(xPos);

    int advanceWidth;
    int leftSideBearing;

    stbtt_GetCodepointHMetrics(&m_fontSet->m_fonts[m_ttfIndex].m_info, codepoint, &advanceWidth, &leftSideBearing);

    int x0;
    int y0;
    int x1;
    int y1;

    stbtt_GetCodepointBitmapBoxSubpixel(&m_fontSet->m_fonts[m_ttfIndex].m_info, codepoint, m_scale, m_scale, shiftX,
                                        0.0f, &x0, &y0, &x1, &y1);

    GlyphInfo* glyph = new GlyphInfo;

    glyph->m_codepoint = codepoint;
    glyph->m_slot = -1;
    glyph->m_advanceWidth = advanceWidth;
    glyph->m_leftSideBearing = (float)leftSideBearing * m_scale;
    glyph->m_x0 = x0;
    glyph->m_y0 = y0;
    glyph->m_x1 = x1;
    glyph->m_y1 = y1;
    glyph->m_lastUsed = 0;

    m_glyphs[codepoint] = glyph;

    return glyph;
}

fuiTTFFontSet::GlyphInfo* fuiTTFFontSet::FontSizeData::getGlyphInfo(int codepoint)
{
    boost::unordered_map<int, GlyphInfo*>::iterator it = m_glyphs.find(codepoint);

    if (it == m_glyphs.end())
    {
        fuiTTFFontSet* fontSet = m_fontSet;

        if (fontSet->getStbGlyphIndex(codepoint, m_ttfIndex) != 0)
        {
            return createGlyph(codepoint);
        }

        if (codepoint == fontSet->m_missingGlyphChar)
        {
            return NULL;
        }

        return getGlyphInfo(fontSet->m_missingGlyphChar);
    }

    return it->second;
}

void fuiTTFFontSet::FontSizeData::getGlyphTexPos(unsigned short codepoint, float& texX, float& texY)
{
    GlyphInfo* glyph = getGlyphInfo(codepoint);

    int slot = glyph->m_slot;
    int row = slot / m_glyphsPerRow;
    int column = slot - row * m_glyphsPerRow;

    texX = (float)(column * m_glyphWidth) / (float)m_textureWidth;
    texY = (float)(row * m_glyphHeight) / (float)m_textureHeight;
}

void fuiTTFFontSet::FontSizeData::getGlyphWidthHeight(float& texWidth, float& texHeight)
{
    texWidth = (float)m_glyphWidth / (float)m_textureWidth;
    texHeight = (float)m_glyphHeight / (float)m_textureHeight;
}

float fuiTTFFontSet::FontSizeData::renderGlyph(unsigned short codepoint, float x, float y, unsigned int colour,
                                               std::vector<fuiFontSet::FontVert> (&verts)[4])
{
    float glyphWidth = (float)m_glyphWidth;
    float glyphHeight = (float)m_glyphHeight;

    float texX;
    float texY;

    getGlyphTexPos(codepoint, texX, texY);

    GlyphInfo* glyph = getGlyphInfo(codepoint);

    glyph->m_lastUsed = m_glyphUseCounter++;

    float texWidth;
    float texHeight;

    getGlyphWidthHeight(texWidth, texHeight);

    if (glyph->m_slot < 0)
    {
        return 0.0f;
    }

    float left = x + 1.0f + glyph->m_leftSideBearing;
    float top = y + 1.0f + (float)(m_ascent + glyph->m_y0);
    float right = left + glyphWidth;
    float bottom = top + glyphHeight;

    fuiFontSet::FontVert vert;

    vert.x = left;
    vert.y = top;
    vert.u = texX;
    vert.v = texY;
    vert.colour = colour;
    verts[m_ttfIndex].push_back(vert);

    vert.x = right;
    vert.y = top;
    vert.u = texX + texWidth;
    vert.v = texY;
    vert.colour = colour;
    verts[m_ttfIndex].push_back(vert);

    vert.x = right;
    vert.y = bottom;
    vert.u = texX + texWidth;
    vert.v = texY + texHeight;
    vert.colour = colour;
    verts[m_ttfIndex].push_back(vert);

    vert.x = left;
    vert.y = bottom;
    vert.u = texX;
    vert.v = texY + texHeight;
    vert.colour = colour;
    verts[m_ttfIndex].push_back(vert);

    return (float)glyph->m_advanceWidth * m_scale;
}

void fuiTTFFontSet::FontSizeData::setPixel(int x, int y, unsigned char value)
{
    int offset = (y * m_textureWidth + x) * 4;

    m_textureData[offset] = value;
    m_textureData[offset + 1] = value;
    m_textureData[offset + 2] = value;
    m_textureData[offset + 3] = value;
}

void fuiTTFFontSet::FontSizeData::loadGlyphImage(GlyphInfo* glyph)
{
    if (glyph->m_slot >= 0)
    {
        return;
    }

    unsigned short codepoint = (unsigned short)glyph->m_codepoint;
    fuiTTFFontSet* fontSet = m_fontSet;

    if (stbtt_FindGlyphIndex(&fontSet->m_fonts[m_ttfIndex].m_info, codepoint) == 0)
    {
        codepoint = fontSet->m_missingGlyphChar;
    }

    m_textureDirty = true;

    memset(m_glyphBitmap, 0, m_glyphWidth * m_glyphHeight);

    stbtt_MakeCodepointBitmap(&m_fontSet->m_fonts[m_ttfIndex].m_info, m_glyphBitmap, glyph->m_x1 - glyph->m_x0,
                              glyph->m_y1 - glyph->m_y0, m_glyphWidth, m_scale, m_scale, codepoint);

    int x;
    int y;
    for (y = 0; y < m_glyphHeight; y++)
    {
        for (x = 0; x < m_glyphWidth; x++)
        {
            int pixel = y * m_glyphWidth + x;

            m_glyphBitmapRGBA[pixel * 4] = m_glyphBitmap[pixel];
            m_glyphBitmapRGBA[pixel * 4 + 1] = m_glyphBitmap[pixel];
            m_glyphBitmapRGBA[pixel * 4 + 2] = m_glyphBitmap[pixel];
            m_glyphBitmapRGBA[pixel * 4 + 3] = m_glyphBitmap[pixel];
        }
    }

    long long oldestUse = m_glyphUseCounter;
    int slot = -1;

    for (int i = 0; i < m_maxGlyphs; i++)
    {
        GlyphInfo* slotGlyph = m_glyphSlots[i];

        if (slotGlyph == NULL)
        {
            slot = i;
            break;
        }

        if (slotGlyph->m_lastUsed < oldestUse)
        {
            oldestUse = slotGlyph->m_lastUsed;
            slot = i;
        }
    }

    GlyphInfo* evicted = m_glyphSlots[slot];

    if (evicted != NULL)
    {
        evicted->m_lastUsed = -1;
        evicted->m_slot = -1;
    }

    m_glyphSlots[slot] = glyph;

    glyph->m_lastUsed = m_glyphUseCounter++;
    glyph->m_slot = slot;

    int row = slot / m_glyphsPerRow;
    int destX = (slot - row * m_glyphsPerRow) * m_glyphWidth;
    int destY = row * m_glyphHeight;

    for (y = 0; y < m_glyphHeight; y++)
    {
        for (x = 0; x < m_glyphWidth; x++)
        {
            setPixel(destX + x, destY + y, m_glyphBitmap[y * m_glyphWidth + x]);
        }
    }
}

void fuiTTFFontSet::FontSizeData::loadGlyph(int codepoint)
{
    GlyphInfo* glyph = getGlyphInfo(codepoint);

    glyph->m_lastUsed = m_glyphUseCounter++;

    loadGlyphImage(glyph);
}

void fuiTTFFontSet::FontSizeData::uploadTextureData()
{
    if (m_textureDirty)
    {
        gRenderer->TextureBind(getTextureIndex());
        gRenderer->TextureDataUpdate(0, 0, m_textureWidth, m_textureHeight, m_textureData, 0);
        m_textureDirty = false;
    }
}
