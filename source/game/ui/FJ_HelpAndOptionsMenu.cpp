#include "game/ui/FJ_HelpAndOptionsMenu.hpp"

FJ_HelpAndOptionsMenu::FJ_HelpAndOptionsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_HelpAndOptionsMenu* FJ_HelpAndOptionsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_HelpAndOptionsMenu(renderNode);
}

unsigned int FJ_HelpAndOptionsMenu::getType()
{
    return 0x11;
}
