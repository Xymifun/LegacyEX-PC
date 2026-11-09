#include "game/ui/FJ_DebugBuildTools.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_DebugBuildTools::FJ_DebugBuildTools(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DebugBuildTools::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugBuildTools(renderNode);
}

unsigned int FJ_DebugBuildTools::getType()
{
    return 51;
}
