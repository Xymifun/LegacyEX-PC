#include "game/ui/FJ_ListButton.hpp"

FJ_ListButton::FJ_ListButton(fuiRenderNode* renderNode) : FJ_Button(renderNode), m_listButtonFlag(false)
{
    m_textOnly = true;
    m_labelAlignment = 1;
}

void FJ_ListButton::Init(const std::wstring& name, int id)
{
    if (m_labelAlignment == 1)
        m_labelAlignment = 4;
    if (m_listButtonFlag)
        m_labelAlignment = 2;
    SetLabelAlignment(m_labelAlignment);
    FJ_Button::Init(name, id);
}

void FJ_ListButton::UseEllipsis(bool useEllipsis)
{
    m_textOnly = useEllipsis;
}
