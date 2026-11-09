#include "game/ui/FJ_SettingsAudioMenu.hpp"

FJ_SettingsAudioMenu::FJ_SettingsAudioMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_SettingsAudioMenu* FJ_SettingsAudioMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SettingsAudioMenu(renderNode);
}

unsigned int FJ_SettingsAudioMenu::getType()
{
    return 0x44;
}
