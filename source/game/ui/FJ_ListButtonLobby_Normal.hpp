#ifndef GAME_UI_FJ_LISTBUTTONLOBBY_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONLOBBY_NORMAL_HPP

#include "game/ui/FJ_ListButtonLobby.hpp"

class fuiRenderNode;

class FJ_ListButtonLobby_Normal : public FJ_ListButtonLobby
{
public:
    explicit FJ_ListButtonLobby_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonLobby* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
