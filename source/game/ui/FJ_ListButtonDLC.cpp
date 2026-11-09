#include "game/ui/FJ_ListButtonDLC.hpp"

#include "game/ui/FJ_FuiNode.hpp"

FJ_ListButtonDLC::FJ_ListButtonDLC(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
    m_tickNode = getChildByName("DLCTick")->asMovieClip();
    m_tickNode->setVisible(false);
}

void FJ_ListButtonDLC::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_textOnly = true;
    m_tickNode->setScaleX(m_tickNode->getScaleX() / getScaleX());
    m_tickNode->setScaleY(m_tickNode->getScaleY() / getScaleY());
    m_ellipsisWidthOverride = 0;
    FJ_Button::Init(name, id);
}

void FJ_ListButtonDLC::ShowTick(bool show)
{
    m_tickNode->setVisible(show);
    if (m_textOnly)
    {
        int width = 0;
        if (show)
            width = static_cast<int>(m_tickNode->getX() * getScaleX());
        m_ellipsisWidthOverride = width;
        SetLabel(GetLabel());
    }
}
