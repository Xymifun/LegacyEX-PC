#ifndef GAME_UI_FJ_SETTINGSGRAPHICSMENU_HPP
#define GAME_UI_FJ_SETTINGSGRAPHICSMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SettingsGraphicsMenu : public FJ_Document
{
public:
    FJ_SettingsGraphicsMenu(fuiRenderNode* renderNode);
    static FJ_SettingsGraphicsMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
