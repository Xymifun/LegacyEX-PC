#ifndef GAME_UI_FJ_LISTBUTTONLOBBY_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONLOBBY_SMALL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonLobby.hpp"

class fuiRenderNode;

class FJ_ListButtonLobby_Small : public FJ_ListButtonLobby
{
public:
    explicit FJ_ListButtonLobby_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonLobby* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
