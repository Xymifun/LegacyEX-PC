#include "game/ui/fuiBitmapFont.hpp"

#include <string>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/CFontData.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/client/render/SFontData.hpp"
#include "game/ui/fui.hpp"

fuiBitmapFont::fuiBitmapFont(SFontData& fontData, unsigned short missingGlyphChar)
    : fuiFontSet(fontData.fontName.c_str()),
      m_bitmapFont(fontData),
      m_missingGlyphChar(missingGlyphChar)
{
    m_sFontData = &fontData;
    BufferedImage image(fontData.textureFileName, true, true, L"");

    int* pixels = image.getData();
    const int width = static_cast<int>(image.getWidth());
    const int height = static_cast<int>(image.getHeight());

    m_textureIndex = static_cast<unsigned int>(gRenderer->TextureCreate());
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    for (unsigned int i = 0; i < image.getWidth() * image.getHeight(); ++i)
    {
        const unsigned int argb = static_cast<unsigned int>(pixels[i]);
        unsigned char* rgba = reinterpret_cast<unsigned char*>(&pixels[i]);
        rgba[0] = static_cast<unsigned char>(argb >> 16);
        rgba[1] = static_cast<unsigned char>(argb >> 8);
        rgba[2] = static_cast<unsigned char>(argb);
        rgba[3] = static_cast<unsigned char>(argb >> 24);
    }
#else
    fui::EndianSwapImageData(pixels, image.getWidth() * image.getHeight());
#endif
    gRenderer->TextureBind(static_cast<int>(m_textureIndex));
    gRenderer->TextureSetTextureLevels(1);
    gRenderer->TextureData(width, height, pixels, 0, C4JRender::eTextureFormat_RGBA8);
    m_registered = false;
}

int fuiBitmapFont::getTextureIndex(float fontSize, int ttfIndex)
{
    return static_cast<int>(m_textureIndex);
}

float fuiBitmapFont::getGlyphScale(float fontSize)
{
    for (unsigned int i = 0; i < m_glyphScaleData.size(); ++i)
    {
        if (m_glyphScaleData[i].fontSize == fontSize)
        {
            return m_glyphScaleData[i].scale;
        }
    }

    float scale = 1.0f;
    const float inverseWidthScale = 1.0f / m_bitmapFont.getFontData()->getFontData().glyphWidthScale;
    while ((scale + 0.5f) * inverseWidthScale < fontSize)
    {
        scale += 1.0f;
    }

    GlyphScaleData data;
    data.fontSize = fontSize;
    data.scale = scale;
    m_glyphScaleData.push_back(data);
    return scale;
}

void fuiBitmapFont::getGlyphTexPos(unsigned short codepoint, float& texX, float& texY)
{
    const unsigned short glyphId = m_bitmapFont.getFontData()->getGlyphId(codepoint);
    int row = 0;
    int column = 0;
    m_bitmapFont.getFontData()->getPos(glyphId, row, column);

    const float x = static_cast<float>(column * m_sFontData->glyphWidth) / static_cast<float>(m_sFontData->imageWidth);
    const float y = static_cast<float>(row * m_sFontData->glyphHeight) / static_cast<float>(m_sFontData->imageHeight);
    texX = x;
    texY = y;
}

void fuiBitmapFont::getGlyphWidthHeight(float& texWidth, float& texHeight)
{
    texWidth = static_cast<float>(m_sFontData->glyphWidth) / static_cast<float>(m_sFontData->imageWidth);
    texHeight = static_cast<float>(m_sFontData->glyphHeight) / static_cast<float>(m_sFontData->imageHeight);
}

float fuiBitmapFont::renderGlyph(unsigned short codepoint, float x, float y, float fontSize, unsigned int colour,
                                 std::vector<fuiFontSet::FontVert> (&verts)[4])
{
    CFontData* fontData = m_bitmapFont.getFontData();
    const unsigned short glyphId = fontData->getGlyphId(codepoint);
    const float glyphScale = getGlyphScale(fontSize);
    const float right = x + static_cast<float>(m_bitmapFont.getFontData()->getFontData().glyphWidth) * glyphScale;
    const float bottom = y + static_cast<float>(m_bitmapFont.getFontData()->getFontData().glyphHeight) * glyphScale;

    float texX;
    float texY;
    getGlyphTexPos(codepoint, texX, texY);

    float texWidth;
    float texHeight;
    getGlyphWidthHeight(texWidth, texHeight);

    fuiFontSet::FontVert vert;
    vert.x = x;
    vert.y = y;
    vert.u = texX;
    vert.v = texY;
    vert.colour = colour;
    verts[0].push_back(vert);

    vert.x = right;
    vert.u = texX + texWidth;
    verts[0].push_back(vert);

    vert.x = right;
    vert.y = bottom;
    vert.v = texY + texHeight;
    verts[0].push_back(vert);

    vert.x = x;
    vert.u = texX;
    verts[0].push_back(vert);

    return static_cast<float>(fontData->getWidth(glyphId)) * glyphScale;
}

float fuiBitmapFont::getRenderedGlyphWidth(unsigned short codepoint, float fontSize)
{
    const unsigned short glyphId = m_bitmapFont.getFontData()->getGlyphId(codepoint);
    const float glyphScale = getGlyphScale(fontSize);
    return static_cast<float>(static_cast<int>(m_bitmapFont.getFontData()->getWidth(glyphId))) * glyphScale;
}

float fuiBitmapFont::getRenderedGlyphHeight(unsigned short codepoint, float fontSize)
{
    const float glyphScale = getGlyphScale(fontSize);
    return static_cast<float>(m_bitmapFont.getFontData()->getFontData().glyphHeight) * glyphScale - 2.0f;
}

bool fuiBitmapFont::isBitmapFont()
{
    return true;
}

void fuiBitmapFont::preRender()
{
}

void fuiBitmapFont::registerFont()
{
    if (!m_registered)
    {
        gFui->installBitmapFont(this);
        m_registered = true;
    }

    gFui->setFontIndirection(m_name.c_str(), m_name.c_str());
}

int fuiBitmapFont::getFallbackFontTexture(float fontSize)
{
    return -1;
}

void fuiBitmapFont::loadGlyph(int codepoint, float fontSize)
{
    m_bitmapFont.getFontData()->getGlyphId(static_cast<unsigned int>(codepoint));
}

bool fuiBitmapFont::isGlyphLoaded(int codepoint, float fontSize)
{
    m_bitmapFont.getFontData()->getGlyphId(static_cast<unsigned int>(codepoint));
    return true;
}

bool fuiBitmapFont::hasGlyph(int codepoint)
{
    return m_bitmapFont.getFontData()->getGlyphId(static_cast<unsigned int>(codepoint)) != 0;
}
