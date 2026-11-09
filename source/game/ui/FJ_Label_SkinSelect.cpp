#include "game/ui/FJ_Label_SkinSelect.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_SkinSelect::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SkinSelect(renderNode);
}

unsigned int FJ_Label_SkinSelect::getType()
{
    return 0x8078;
}
