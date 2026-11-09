#ifndef GAME_UI_FJ_LIST_MENUBUTTON_HPP
#define GAME_UI_FJ_LIST_MENUBUTTON_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_MenuButton : public FJ_List
{
public:
    explicit FJ_List_MenuButton(fuiRenderNode* renderNode);
    virtual uint32_t getType();

    void addNewItem(std::wstring label, int id);
    static FJ_List* Create(fuiRenderNode* renderNode);

private:
    bool m_useEllipsis;
};

#endif
