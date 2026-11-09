#ifndef GAME_UI_FJ_SETTINGSUIMENU_HPP
#define GAME_UI_FJ_SETTINGSUIMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SettingsUIMenu : public FJ_Document
{
public:
    FJ_SettingsUIMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
