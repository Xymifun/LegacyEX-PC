#ifndef GAME_UI_FJ_HELPANDOPTIONSMENU_HPP
#define GAME_UI_FJ_HELPANDOPTIONSMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_HelpAndOptionsMenu : public FJ_Document
{
public:
    FJ_HelpAndOptionsMenu(fuiRenderNode* renderNode);
    static FJ_HelpAndOptionsMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
