#include "game/ui/FJ_ListButtonController_Normal.hpp"

FJ_ListButtonController_Normal::FJ_ListButtonController_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonController(renderNode)
{
}

FJ_ListButtonController* FJ_ListButtonController_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonController_Normal(renderNode);
}

uint32_t FJ_ListButtonController_Normal::getType()
{
    return 0x802c;
}
