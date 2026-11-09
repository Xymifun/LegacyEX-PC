#ifndef GAME_UI_FJ_CURSOR_NORMAL_HPP
#define GAME_UI_FJ_CURSOR_NORMAL_HPP

#include "game/ui/FJ_Cursor.hpp"

class fuiRenderNode;

class FJ_Cursor_Normal : public FJ_Cursor
{
public:
    FJ_Cursor_Normal(fuiRenderNode* pRenderNode);
    static FJ_Cursor* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
