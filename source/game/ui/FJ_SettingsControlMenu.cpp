#include "game/ui/FJ_SettingsControlMenu.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_SettingsControlMenu::FJ_SettingsControlMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SettingsControlMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SettingsControlMenu(renderNode);
}

unsigned int FJ_SettingsControlMenu::getType()
{
    return 69;
}
