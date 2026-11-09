#ifndef GAME_UI_FJ_PAUSEMENU_HPP
#define GAME_UI_FJ_PAUSEMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_PauseMenu : public FJ_Document
{
public:
    FJ_PauseMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
