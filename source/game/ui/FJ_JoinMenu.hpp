#ifndef GAME_UI_FJ_JOINMENU_HPP
#define GAME_UI_FJ_JOINMENU_HPP

#include <stdint.h>

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_JoinMenu : public FJ_Document
{
public:
    FJ_JoinMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
