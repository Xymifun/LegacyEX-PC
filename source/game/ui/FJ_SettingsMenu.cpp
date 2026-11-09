#include "game/ui/FJ_SettingsMenu.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_SettingsMenu::FJ_SettingsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SettingsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SettingsMenu(renderNode);
}

unsigned int FJ_SettingsMenu::getType()
{
    return 0x12;
}
