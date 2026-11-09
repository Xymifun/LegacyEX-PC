#include "game/ui/FJ_TouchControlFrontend.hpp"

#include <string>

FJ_TouchControlFrontend::FJ_TouchControlFrontend(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    Init();
}

FJ_TouchControlFrontend* FJ_TouchControlFrontend::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_TouchControlFrontend(pRenderNode);
}

void FJ_TouchControlFrontend::Init()
{
    FJ_FuiNode* placeholder = getChildByName(std::string("placeholder"))->asMovieClip();
    placeholder->setVisible(false);
}

unsigned int FJ_TouchControlFrontend::getType()
{
    return 0x66;
}
