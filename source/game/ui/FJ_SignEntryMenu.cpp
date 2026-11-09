#include "game/ui/FJ_SignEntryMenu.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_SignEntryMenu::FJ_SignEntryMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SignEntryMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SignEntryMenu(renderNode);
}

unsigned int FJ_SignEntryMenu::getType()
{
    return 0x4a;
}
