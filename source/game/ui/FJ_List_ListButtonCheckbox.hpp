#ifndef GAME_UI_FJ_LIST_LISTBUTTONCHECKBOX_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONCHECKBOX_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonCheckbox : public FJ_List
{
public:
    explicit FJ_List_ListButtonCheckbox(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void UseIggyIcons();
    void addNewItem(const std::wstring& label, int id, bool checked, const std::wstring& textureName);
    void SetCheckBox(int index, bool checked);
    uint8_t GetCheckBox(int index);
    void SetTextureName(int index, const std::wstring& textureName);

private:
    bool m_useIggyIcons;
};

#endif
