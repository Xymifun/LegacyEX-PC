#ifndef GAME_UI_FJ_LIST_LISTBUTTONLOBBY_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONLOBBY_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonLobby : public FJ_List
{
public:
    explicit FJ_List_ListButtonLobby(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void addNewItem(const std::wstring& label, int id, bool showTick, int playerIcon, int voipIcon);
    void SetPlayerIcon(int item, int icon);
    void SetVOIPIcon(int item, int icon);
    void ShowTick(int item, bool showTick);
};

#endif
