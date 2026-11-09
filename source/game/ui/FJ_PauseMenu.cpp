#include "game/ui/FJ_PauseMenu.hpp"

FJ_PauseMenu::FJ_PauseMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_PauseMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_PauseMenu(renderNode);
}

unsigned int FJ_PauseMenu::getType()
{
    return 66;
}
