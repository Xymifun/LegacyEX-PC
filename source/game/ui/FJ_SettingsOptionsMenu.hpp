#ifndef GAME_UI_FJ_SETTINGSOPTIONSMENU_HPP
#define GAME_UI_FJ_SETTINGSOPTIONSMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SettingsOptionsMenu : public FJ_Document
{
public:
    FJ_SettingsOptionsMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
