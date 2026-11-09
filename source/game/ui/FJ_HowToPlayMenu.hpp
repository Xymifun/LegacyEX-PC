#ifndef GAME_UI_FJ_HOWTOPLAYMENU_HPP
#define GAME_UI_FJ_HOWTOPLAYMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_HowToPlayMenu : public FJ_Document
{
public:
    explicit FJ_HowToPlayMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode) {}
    virtual void handlePress(FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
