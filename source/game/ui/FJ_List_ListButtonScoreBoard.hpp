#ifndef GAME_UI_FJ_LIST_LISTBUTTONSCOREBOARD_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONSCOREBOARD_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonScoreBoard : public FJ_List
{
public:
    explicit FJ_List_ListButtonScoreBoard(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void SetNumColumns(int numColumns);
    void addNewItem(const std::wstring& label, int id, int playerIcon, int voipIcon);
    void SetPlayerIcon(int item, int icon);
    void SetVOIPIcon(int item, int icon);
    void SetSlotText(int item, int slot, const std::wstring& text);
    void SetSlotTextureName(int item, int slot, const std::wstring& textureName);
    int GetButtonHeight();

private:
    int m_numColumns;
    int m_buttonHeight;
};

#endif
