#include "game/ui/FJ_Label_SkinSelectCenter.hpp"

FJ_Label* FJ_Label_SkinSelectCenter::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_SkinSelectCenter(pRenderNode);
}

unsigned int FJ_Label_SkinSelectCenter::getType()
{
    return 0x8079;
}
