#include "game/ui/FJ_LayoutButton.hpp"

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"

FJ_LayoutButton::FJ_LayoutButton(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
}

FJ_Button* FJ_LayoutButton::Create(fuiRenderNode* renderNode)
{
    return new FJ_LayoutButton(renderNode);
}

uint32_t FJ_LayoutButton::getType()
{
    return 0x801e;
}

void FJ_LayoutButton::Init(const std::wstring& name, int id)
{
    FJ_Base::SetLabelAlignment(2);
    FJ_Button::Init(name, id);
}
