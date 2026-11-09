#include "game/ui/FJ_SaveMenu.hpp"

FJ_SaveMenu::FJ_SaveMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SaveMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SaveMenu(renderNode);
}

unsigned int FJ_SaveMenu::getType()
{
    return 67;
}
