#include "game/ui/FJ_ListLabel_SmallBlack.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_ListLabel_SmallBlack::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListLabel_SmallBlack(renderNode);
}

unsigned int FJ_ListLabel_SmallBlack::getType()
{
    return 0x808f;
}
