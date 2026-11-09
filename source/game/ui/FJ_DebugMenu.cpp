#include "game/ui/FJ_DebugMenu.hpp"

FJ_DebugMenu::FJ_DebugMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DebugMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugMenu(renderNode);
}

unsigned int FJ_DebugMenu::getType()
{
    return 55;
}
