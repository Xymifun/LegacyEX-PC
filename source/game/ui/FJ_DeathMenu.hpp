#ifndef GAME_UI_FJ_DEATHMENU_HPP
#define GAME_UI_FJ_DEATHMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_DeathMenu : public FJ_Document
{
public:
    explicit FJ_DeathMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
