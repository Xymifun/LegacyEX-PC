#include "game/ui/FJ_ListLabel_Black.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_ListLabel_Black::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListLabel_Black(renderNode);
}

unsigned int FJ_ListLabel_Black::getType()
{
    return 0x808e;
}
