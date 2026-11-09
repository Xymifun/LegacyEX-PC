#include "game/ui/FJ_MenuButton.hpp"

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"

FJ_MenuButton::FJ_MenuButton(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
}

void FJ_MenuButton::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(2);
    FJ_Button::Init(name, id);
}

void FJ_MenuButton::UseEllipsis(bool useEllipsis)
{
    m_textOnly = useEllipsis;
}
