#include "game/ui/FJ_HtmlText_WithPanel.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"

FJ_HtmlText_WithPanel::FJ_HtmlText_WithPanel(fuiRenderNode* renderNode) : FJ_HtmlText(renderNode)
{
    m_panelNode = NULL;
}

FJ_HtmlText_WithPanel::~FJ_HtmlText_WithPanel()
{
    if (m_panelNode != NULL)
    {
        if (m_panelNode->getParent() != NULL)
            m_panelNode->getParent()->removeChild(m_panelNode, true);
    }
    m_panelOffset = 20;
}

unsigned int FJ_HtmlText_WithPanel::getType()
{
    return 0x80a6;
}

void FJ_HtmlText_WithPanel::setVisible(bool visible)
{
    FJ_HtmlText::setVisible(visible);
    if (m_panelNode != NULL)
        m_panelNode->setVisible(visible);
}

void FJ_HtmlText_WithPanel::setY(float y)
{
    float deltaY = y - getY();
    FJ_HtmlText::setY(y);
    if (m_panelNode != NULL)
        m_panelNode->setY(m_panelNode->getY() + deltaY);
}

void FJ_HtmlText_WithPanel::Init(const std::wstring& label)
{
    float width = getWidth();
    float height = getHeight();

    FJ_FuiNode* panel = getChildByName(std::string("HtmlTextPanel"));
    m_panelNode = panel->getChildAt(0);
    panel->removeChild(m_panelNode, false);
    removeChild(panel, true);

    getParent()->addChild(m_panelNode);
    m_panelNode->setX(getX());
    m_panelNode->setY(getY());
    m_panelNode->setWidth(width);
    m_panelNode->setHeight(height);

    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->removeChild(m_pText, false);
    static_cast<fuiRenderNodeTimeline*>(getParent()->m_pRenderNode->asTimeline())->addChild(m_pText);

    m_pText->setX(getX() + static_cast<float>(m_panelOffset));
    m_pText->setY(getY() + static_cast<float>(m_panelOffset));
    m_pText->setWidth(width - static_cast<float>(m_panelOffset * 2));
    m_pText->setHeight(height - (static_cast<float>(m_panelOffset) + m_pUpArrow->getHeight() * 1.4f));
    m_pText->setAutoSize(TextFieldAutoSizeLeft);
    m_pText->m_wordWrap = true;
    m_pText->m_multiline = true;

    m_fScrollBottom = m_fScrollBottom - m_fScrollTop;
    m_fScrollLeft = 0.0f;
    m_fScrollTop = 0.0f;
    m_bScrollingEnabled = m_bUseScrollRect;
    m_fWidth = width - static_cast<float>(m_panelOffset * 2);
    float scrollHeight = height - (static_cast<float>(m_panelOffset) + m_pUpArrow->getHeight() * 1.4f);
    m_fScrollBottom = m_fScrollTop + scrollHeight;
    m_iRealWidth = static_cast<int>(m_fWidth - m_fScrollLeft);
    m_iRealHeight = static_cast<int>(m_fScrollBottom - m_fScrollTop);

    if (m_bScrollingEnabled)
    {
        fuiRect scrollRect;
        scrollRect.xMin = m_fScrollLeft;
        scrollRect.xMax = m_fWidth;
        scrollRect.yMin = m_fScrollTop;
        scrollRect.yMax = m_fScrollBottom;
        m_pText->setScrollRect(scrollRect);
    }

    SetLabel(label);
    HandleScrollListArrows(true);
}
