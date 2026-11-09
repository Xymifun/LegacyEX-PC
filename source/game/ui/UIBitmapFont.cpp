#include "game/ui/UIBitmapFont.hpp"

#include <string>

#include "game/client/render/CFontData.hpp"
#include "game/client/render/SFontData.hpp"
#include "game/core/BufferedImage.hpp"

UIBitmapFont::UIBitmapFont(SFontData& fontData) : UIAbstractBitmapFont(fontData.fontName)
{
    m_numGlyphs = fontData.numGlyphs;
    BufferedImage image(fontData.textureFileName, true, true, L"");
    m_fontData = new CFontData(fontData, image.getData());
}

UIBitmapFont::~UIBitmapFont()
{
    m_fontData->release();
    delete m_fontData;
}

CFontData* UIBitmapFont::getFontData()
{
    return m_fontData;
}
