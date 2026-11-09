#include "game/ui/FJ_Cursor.hpp"

#include <string>

#include "game/ui/FJ_Document.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_Cursor::FJ_Cursor(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

void FJ_Cursor::TopAlignPanel()
{
    int panelBottom = (int)(getParent()->getY() + getY() - m_textPanel->getHeight());
    int safeTop = static_cast<FJ_Document*>(getParent()->getParent())->getISafeZoneTop();
    m_topAlignment = panelBottom <= safeTop ? safeTop - panelBottom : 0;
}

void FJ_Cursor::AlignPanel(bool bRight)
{
    fuiRect charBounds = m_textField->getCharBoundaries(0);
    int charHeight = static_cast<int>(charBounds.yMax - charBounds.yMin);

    if (bRight)
    {
        m_textPanel->setX((float)Math::round(m_cursorImage->getWidth() * 0.5f * 0.8f));
        m_textPanel->setY((float)m_topAlignment - m_textPanel->getHeight());
        m_textField->setX((float)Math::round(m_textPanel->getX() + charHeight * 0.35f));
        m_textField->setY((float)Math::round(m_textPanel->getY() + charHeight * 0.5f));
        return;
    }

    m_textPanel->setX((float)-Math::round(m_cursorImage->getWidth() * 0.5f * 0.8f)
                      - m_textPanel->getChildAt(0)->getWidth());
    m_textPanel->setY((float)m_topAlignment - m_textPanel->getHeight());
    m_textField->setX((float)Math::round(m_textPanel->getX() + charHeight * 0.35));
    m_textField->setY((float)Math::round(m_textPanel->getY() + (float)(charHeight / 2)));
}

void FJ_Cursor::UpdatePanelAlignment()
{
    if (!m_textField->getText().empty())
    {
        int right = (int)(m_panelOffsetX + getX() + getWidth());
        float stageWidth = getStage()->getStageWidth();
        FJ_FuiNode* document = getParent()->getParent();
        int safeRight = static_cast<FJ_Document*>(document)->getISafeZoneRight();
        int limit = (int)(stageWidth - safeRight);
        if (document != 0)
        {
            TopAlignPanel();
            AlignPanel(right <= limit);
        }
    }
}

void FJ_Cursor::SetLabel(const std::wstring& label)
{
    m_textField->setHTMLText(label);
    m_textField->setAutoSize(TextFieldAutoSizeLeft);
    m_textPanel->setVisible(!label.empty());
    if (m_textPanel->isVisible())
    {
        AlignPanel(true);
        fuiRect charBounds = m_textField->getCharBoundaries(0);
        int charHeight = static_cast<int>(charBounds.yMax - charBounds.yMin);
        m_textPanel->setScaleX(1.0f);
        m_textPanel->setScaleY(1.0f);
        int width = Math::round(m_textField->getWidth() + charHeight * 0.8f);
        m_textPanel->getChildAt(0)->setWidth((float)width);
        m_textPanel->getChildAt(0)->setHeight(m_textField->getHeight() + charHeight);
        UpdatePanelAlignment();
    }
}

void FJ_Cursor::ASConstructor()
{
    FJ_FuiNode* textContainer = getChildByName(std::string("FJ_TextContainer"))->asMovieClip();
    FJ_FuiNode* textFieldContainer = textContainer->getChildAt(0)->asMovieClip();
    m_textField = static_cast<fuiRenderNodeEditText*>(
        textFieldContainer->getRenderNodeChildByName(std::string("FJ_TextField")));
    addChild(m_textField);
    m_textField->setY(textContainer->getY());
    removeChild(textContainer, true);

    m_textPanel = getChildByName(std::string("PointerTextPanel"))->asMovieClip();
    m_pointerTextPanel = getChildByName(std::string("iggy_pointerIcon"))->asMovieClip();
    m_cursorImage = getChildByName(std::string("CursorImage"))->asMovieClip();
    m_topAlignment = 0;
    m_panelOffsetX = 0;
    SetLabel(std::wstring(L""));
}
