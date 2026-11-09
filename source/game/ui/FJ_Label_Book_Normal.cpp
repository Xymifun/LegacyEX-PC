#include "game/ui/FJ_Label_Book_Normal.hpp"

FJ_Label_Book_Normal::FJ_Label_Book_Normal(fuiRenderNode* renderNode) : FJ_Label_Book(renderNode)
{
}

FJ_Label_Book_Normal* FJ_Label_Book_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Book_Normal(renderNode);
}

unsigned int FJ_Label_Book_Normal::getType()
{
    return 0x8063;
}
