#include "game/ui/FJ_UpsellButton_Normal.hpp"

FJ_UpsellButton_Normal::FJ_UpsellButton_Normal(fuiRenderNode* renderNode) : FJ_UpsellButton(renderNode)
{
}

FJ_UpsellButton* FJ_UpsellButton_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_UpsellButton_Normal(renderNode);
}

unsigned int FJ_UpsellButton_Normal::getType()
{
    return 0x802f;
}
