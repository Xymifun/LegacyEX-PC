#ifndef GAME_UI_FJ_LIST_MULTILIST_HPP
#define GAME_UI_FJ_LIST_MULTILIST_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_MultiList : public FJ_List
{
public:
    explicit FJ_List_MultiList(fuiRenderNode* renderNode);

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void addNewItem_Label(const std::wstring& label);
    void addNewItem_Button(const std::wstring& label, int id, bool useEllipsis);
    void addNewItem_MenuButton(const std::wstring& label, int id);
    void addNewItem_ControllerButton(const std::wstring& label, const std::wstring& secondaryLabel, bool html, int id);
    void addNewItem_CheckBox(const std::wstring& label, int id, bool checked);
    void addNewItem_Slider(const std::wstring& label, int id, int minValue, int maxValue, int value, bool repeat,
                           int repeatDelay, int repeatStep);
    void addNewItem_TextInput(const std::wstring& label, int id, int charLimit);
    virtual void HandleCharInput(unsigned int character);
    void SetCheckBox(int index, bool checked);
    uint8_t GetCheckBox(int index);
    void SetSliderValue(int index, int value);
    uint32_t GetSliderValue(int index);
    void SetSecondaryLabel(int index, const std::wstring& label, bool html, bool autoSize);
    void ToggleButtonOutline(int index, bool show);
    void SetPossibleLabels(int index, std::vector<std::wstring> labels);

private:
    std::vector<std::vector<std::wstring> > m_labelLists;
};

#endif
