#include "game/ui/FJ_Label_BigBlack.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_BigBlack::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_BigBlack(renderNode);
}

unsigned int FJ_Label_BigBlack::getType()
{
    return 0x805a;
}
