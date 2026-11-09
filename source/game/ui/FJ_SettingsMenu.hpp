#ifndef GAME_UI_FJ_SETTINGSMENU_HPP
#define GAME_UI_FJ_SETTINGSMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SettingsMenu : public FJ_Document
{
public:
    explicit FJ_SettingsMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
