#include "game/ui/FJ_LayoutButton_Small.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LayoutButton_Small::FJ_LayoutButton_Small(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
}

FJ_Button* FJ_LayoutButton_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_LayoutButton_Small(renderNode);
}

uint32_t FJ_LayoutButton_Small::getType()
{
    return 0x801f;
}

void FJ_LayoutButton_Small::Init(const std::wstring& name, int id)
{
    FJ_Base::SetLabelAlignment(2);
    FJ_Button::Init(name, id);
}
