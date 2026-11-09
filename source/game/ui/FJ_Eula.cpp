#include "game/ui/FJ_Eula.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Eula::FJ_Eula(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_Eula::Create(fuiRenderNode* renderNode)
{
    return new FJ_Eula(renderNode);
}

unsigned int FJ_Eula::getType()
{
    return 14;
}
