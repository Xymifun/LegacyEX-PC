#include "game/ui/FJ_MainMenu.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_MainMenu::FJ_MainMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_MainMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_MainMenu(renderNode);
}

unsigned int FJ_MainMenu::getType()
{
    return 1;
}
