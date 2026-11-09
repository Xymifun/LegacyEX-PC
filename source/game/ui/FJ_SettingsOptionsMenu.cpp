#include "game/ui/FJ_SettingsOptionsMenu.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_SettingsOptionsMenu::FJ_SettingsOptionsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SettingsOptionsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SettingsOptionsMenu(renderNode);
}

unsigned int FJ_SettingsOptionsMenu::getType()
{
    return 72;
}
