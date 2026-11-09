#include "game/ui/FJ_ListButtonController_Small.hpp"

FJ_ListButtonController_Small::FJ_ListButtonController_Small(fuiRenderNode* renderNode)
    : FJ_ListButtonController(renderNode)
{
}

FJ_ListButtonController* FJ_ListButtonController_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonController_Small(renderNode);
}

uint32_t FJ_ListButtonController_Small::getType()
{
    return 0x802D;
}
