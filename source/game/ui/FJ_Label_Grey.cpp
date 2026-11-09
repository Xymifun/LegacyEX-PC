#include "game/ui/FJ_Label_Grey.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_Grey::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Grey(pRenderNode);
}

unsigned int FJ_Label_Grey::getType()
{
    return 0x806b;
}
