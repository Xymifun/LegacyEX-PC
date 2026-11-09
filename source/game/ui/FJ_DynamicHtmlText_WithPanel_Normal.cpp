#include "game/ui/FJ_DynamicHtmlText_WithPanel_Normal.hpp"

FJ_DynamicHtmlText* FJ_DynamicHtmlText_WithPanel_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_DynamicHtmlText_WithPanel_Normal(pRenderNode);
}

unsigned int FJ_DynamicHtmlText_WithPanel_Normal::getType()
{
    return 0x80ad;
}
