#include "game/ui/FJ_MenuButton_Normal.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_MenuButton_Normal::FJ_MenuButton_Normal(fuiRenderNode* renderNode) : FJ_MenuButton(renderNode)
{
}

FJ_MenuButton* FJ_MenuButton_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_MenuButton_Normal(renderNode);
}

uint32_t FJ_MenuButton_Normal::getType()
{
    return 0x8011;
}
