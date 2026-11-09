#include "game/ui/FJ_SettingsGraphicsMenu.hpp"

FJ_SettingsGraphicsMenu::FJ_SettingsGraphicsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_SettingsGraphicsMenu* FJ_SettingsGraphicsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SettingsGraphicsMenu(renderNode);
}

unsigned int FJ_SettingsGraphicsMenu::getType()
{
    return 0x46;
}
