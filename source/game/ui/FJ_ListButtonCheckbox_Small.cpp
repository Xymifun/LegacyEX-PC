#include "game/ui/FJ_ListButtonCheckbox_Small.hpp"

FJ_ListButtonCheckbox_Small::FJ_ListButtonCheckbox_Small(fuiRenderNode* renderNode) : FJ_ListButtonCheckbox(renderNode)
{
}

FJ_ListButtonCheckbox* FJ_ListButtonCheckbox_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonCheckbox_Small(renderNode);
}

uint32_t FJ_ListButtonCheckbox_Small::getType()
{
    return 0x8021;
}
