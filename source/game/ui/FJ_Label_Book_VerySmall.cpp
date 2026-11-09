#include "game/ui/FJ_Label_Book_VerySmall.hpp"

FJ_Label* FJ_Label_Book_VerySmall::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Book_VerySmall(pRenderNode);
}

unsigned int FJ_Label_Book_VerySmall::getType()
{
    return 0x8065;
}
