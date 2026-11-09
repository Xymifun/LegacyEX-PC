#include "game/ui/FJ_SettingsUIMenu.hpp"

FJ_SettingsUIMenu::FJ_SettingsUIMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SettingsUIMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SettingsUIMenu(renderNode);
}

unsigned int FJ_SettingsUIMenu::getType()
{
    return 73;
}
