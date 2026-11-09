#include "game/ui/FJ_Label_SkinSelectCenter_Small.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_SkinSelectCenter_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_SkinSelectCenter_Small(pRenderNode);
}

unsigned int FJ_Label_SkinSelectCenter_Small::getType()
{
    return 0x807b;
}
