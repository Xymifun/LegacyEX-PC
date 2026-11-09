#include "game/ui/FJ_DeathMenu.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_DeathMenu::FJ_DeathMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DeathMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DeathMenu(renderNode);
}

unsigned int FJ_DeathMenu::getType()
{
    return 50;
}
