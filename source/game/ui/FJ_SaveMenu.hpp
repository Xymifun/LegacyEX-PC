#ifndef GAME_UI_FJ_SAVEMENU_HPP
#define GAME_UI_FJ_SAVEMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SaveMenu : public FJ_Document
{
public:
    FJ_SaveMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
