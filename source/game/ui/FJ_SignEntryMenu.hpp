#ifndef GAME_UI_FJ_SIGNENTRYMENU_HPP
#define GAME_UI_FJ_SIGNENTRYMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SignEntryMenu : public FJ_Document
{
public:
    explicit FJ_SignEntryMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
