#include "game/ui/FJ_Label_SkinSelectCenter_Off.hpp"

FJ_Label* FJ_Label_SkinSelectCenter_Off::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_SkinSelectCenter_Off(pRenderNode);
}

unsigned int FJ_Label_SkinSelectCenter_Off::getType()
{
    return 0x807a;
}
