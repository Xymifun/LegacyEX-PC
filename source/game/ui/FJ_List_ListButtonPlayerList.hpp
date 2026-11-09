#ifndef GAME_UI_FJ_LIST_LISTBUTTONPLAYERLIST_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONPLAYERLIST_HPP

#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonPlayerList : public FJ_List
{
public:
    explicit FJ_List_ListButtonPlayerList(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void addNewItem(const std::wstring& label, int id, int playerIcon, int voipIcon);
    void SetPlayerIcon(int item, int icon);
    void SetVOIPIcon(int item, int icon);
};

#endif
