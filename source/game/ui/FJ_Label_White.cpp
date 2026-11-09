#include "game/ui/FJ_Label_White.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_White::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_White(renderNode);
}

unsigned int FJ_Label_White::getType()
{
    return 0x8088;
}
