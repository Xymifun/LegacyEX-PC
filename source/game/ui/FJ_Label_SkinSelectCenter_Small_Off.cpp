#include "game/ui/FJ_Label_SkinSelectCenter_Small_Off.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_SkinSelectCenter_Small_Off::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SkinSelectCenter_Small_Off(renderNode);
}

unsigned int FJ_Label_SkinSelectCenter_Small_Off::getType()
{
    return 0x807c;
}
