#ifndef GAME_UI_FJ_MAINMENU_HPP
#define GAME_UI_FJ_MAINMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_MainMenu : public FJ_Document
{
public:
    FJ_MainMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
