#include "game/ui/FJ_MenuButton_Small.hpp"

FJ_MenuButton_Small::FJ_MenuButton_Small(fuiRenderNode* renderNode) : FJ_MenuButton(renderNode)
{
}

FJ_MenuButton* FJ_MenuButton_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_MenuButton_Small(renderNode);
}

uint32_t FJ_MenuButton_Small::getType()
{
    return 0x8012;
}
