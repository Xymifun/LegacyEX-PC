#ifndef GAME_UI_FJ_LIST_LISTBUTTONDLC_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONDLC_HPP

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonDLC : public FJ_List
{
public:
    explicit FJ_List_ListButtonDLC(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void addNewItem(const std::wstring& label, int id, bool showTick);
    void ShowTick(int index, bool showTick);
};

#endif
