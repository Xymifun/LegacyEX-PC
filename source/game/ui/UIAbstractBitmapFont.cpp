#include "game/ui/UIAbstractBitmapFont.hpp"

UIAbstractBitmapFont::UIAbstractBitmapFont(const std::string& fontName) : m_fontName(fontName), m_bRegistered(false)
{
}

UIAbstractBitmapFont::~UIAbstractBitmapFont()
{
}

void UIAbstractBitmapFont::registerFont()
{
}
