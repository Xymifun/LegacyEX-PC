#include "game/ui/FJ_HtmlTextPanel.hpp"

#include <string>
#include <utility>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_HtmlTextPanel::FJ_HtmlTextPanel(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    FJ_FuiNode* textContainer = getChildByName(std::string("FJ_TextContainer"))->asMovieClip();
    FJ_FuiNode* textFieldContainer = textContainer->getChildAt(0)->asMovieClip();
    m_textField = static_cast<fuiRenderNodeEditText*>(
        textFieldContainer->getRenderNodeChildByName(std::string("FJ_TextField")));
    addChild(m_textField);
    m_textField->setY(textContainer->getY());
    removeChild(textContainer, true);

    m_pointerTextPanel = getChildByName(std::string("PointerTextPanel"))->asMovieClip();
    m_fixedWidthEnabled = false;
    m_fixedWidth = 0;
    SetLabel(std::wstring(L""));
}

void FJ_HtmlTextPanel::SetLabel(const std::wstring& label)
{
    m_textField->setHTMLText(label);
    if (m_fixedWidthEnabled)
    {
        m_textField->setAutoSize(TextFieldAutoSizeCenter);
        m_textField->m_wordWrap = true;
        m_textField->setWidth(static_cast<float>(m_fixedWidth));
    }
    else
    {
        m_textField->setAutoSize(TextFieldAutoSizeLeft);
    }

    if (m_fixedWidthEnabled)
    {
        m_textField->setHeight(m_textField->getTextHeight() + 5.0f);
    }

    m_pointerTextPanel->setVisible(!label.empty());
    if (m_pointerTextPanel->isVisible())
    {
        fuiRect charBounds = m_textField->getCharBoundaries(0);
        int charHeight = static_cast<int>(charBounds.yMax - charBounds.yMin);

        m_textField->setX(m_pointerTextPanel->getX() + 4.0f);
        m_pointerTextPanel->getChildAt(0)->setWidth(
            static_cast<float>(Math::round(m_textField->getWidth() + charHeight * 0.8f)));
        m_pointerTextPanel->getChildAt(0)->setHeight(m_textField->getHeight() + charHeight);
        m_textField->setX(static_cast<float>(Math::round(m_pointerTextPanel->getX() + charHeight * 0.35f)));
        m_textField->setY(static_cast<float>(Math::round(m_pointerTextPanel->getY() + charHeight / 2)));
    }
}

FJ_HtmlTextPanel* FJ_HtmlTextPanel::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HtmlTextPanel(pRenderNode);
}

unsigned int FJ_HtmlTextPanel::getType()
{
    return 0x6f;
}

void FJ_HtmlTextPanel::SetFixedWidth(int width)
{
    m_fixedWidth = width;
    m_fixedWidthEnabled = true;
}
