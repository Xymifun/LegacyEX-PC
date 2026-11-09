#include "game/ui/FJ_Label_BigWhite.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_BigWhite::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_BigWhite(pRenderNode);
}

unsigned int FJ_Label_BigWhite::getType()
{
    return 0x805c;
}
