#ifndef GAME_UI_FJ_SETTINGSCONTROLMENU_HPP
#define GAME_UI_FJ_SETTINGSCONTROLMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SettingsControlMenu : public FJ_Document
{
public:
    FJ_SettingsControlMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
