#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_ListButton.hpp"
#include "game/ui/FJ_MenuButton.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/FJ_TextInput.hpp"

#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_ListButtonController.hpp"
#include "game/ui/fuiRenderNode.hpp"


FJ_List_MultiList::FJ_List_MultiList(fuiRenderNode* renderNode) : FJ_List(renderNode)
{
}

FJ_List* FJ_List_MultiList::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_MultiList(renderNode);
}

uint32_t FJ_List_MultiList::getType()
{
    return 0x8031;
}

void FJ_List_MultiList::addNewItem_Label(const std::wstring& label)
{
    FJ_Label* item = static_cast<FJ_Label*>(
        m_checkboxTick ? FJ_Factory::createObject("fourj.Labels.FJ_ListLabel_SmallBlack", m_pRenderNode, true, NULL)
                       : FJ_Factory::createObject("fourj.Labels.FJ_ListLabel_Black", m_pRenderNode, true, NULL));
    addNewListItem(item, label, -1);
    item->Init(label);
}

void FJ_List_MultiList::addNewItem_Button(const std::wstring& label, int id, bool useEllipsis)
{
    FJ_Base* node = static_cast<FJ_Base*>(
        m_checkboxTick ? FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Small", m_pRenderNode, true, NULL)
                       : FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Medium", m_pRenderNode, true, NULL));
    node->asFJ_ListButton()->m_listButtonFlag = true;
    node->asFJ_ListButton()->UseEllipsis(useEllipsis);
    if (m_labelLists.size() > static_cast<unsigned int>(id) && m_labelLists[static_cast<unsigned int>(id)].size() != 0)
    {
        node->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(node, label, id);
    static_cast<FJ_ListButton*>(node)->Init(label, id);
}

void FJ_List_MultiList::addNewItem_MenuButton(const std::wstring& label, int id)
{
    FJ_MenuButton* node = static_cast<FJ_MenuButton*>(
        m_checkboxTick ? FJ_Factory::createObject("fourj.Buttons.FJ_MenuButton_Small", m_pRenderNode, true, NULL)
                       : FJ_Factory::createObject("fourj.Buttons.FJ_MenuButton_Normal", m_pRenderNode, true, NULL));
    if (m_labelLists.size() > static_cast<unsigned int>(id) && m_labelLists[static_cast<unsigned int>(id)].size() != 0)
    {
        node->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(node, label, id);
    node->Init(label, id);
}

void FJ_List_MultiList::addNewItem_ControllerButton(const std::wstring& label, const std::wstring& secondaryLabel,
                                                    bool html, int id)
{
    FJ_ListButtonController* item = static_cast<FJ_ListButtonController*>(
        m_checkboxTick
            ? FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonController_Small", m_pRenderNode, true, NULL)
            : FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonController_Normal", m_pRenderNode, true, NULL));
    if (m_labelLists.size() > static_cast<unsigned int>(id) && m_labelLists[static_cast<unsigned int>(id)].size() != 0)
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetSecondaryText(secondaryLabel, true, html);
}

void FJ_List_MultiList::addNewItem_CheckBox(const std::wstring& label, int id, bool checked)
{
    FJ_CheckBox* item = static_cast<FJ_CheckBox*>(
        m_checkboxTick ? FJ_Factory::createObject("fourj.Checkbox.FJ_CheckBox_Small", m_pRenderNode, true, NULL)
                       : FJ_Factory::createObject("fourj.Checkbox.FJ_CheckBox_Normal", m_pRenderNode, true, NULL));
    if (m_labelLists.size() > static_cast<unsigned int>(id) && m_labelLists[static_cast<unsigned int>(id)].size() != 0)
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id, checked);
}

void FJ_List_MultiList::addNewItem_Slider(const std::wstring& label, int id, int minValue, int maxValue, int value,
                                          bool repeat, int repeatDelay, int repeatStep)
{
    FJ_Slider* item = static_cast<FJ_Slider*>(
        m_checkboxTick ? FJ_Factory::createObject("fourj.FJ_Slider_Small", m_pRenderNode, true, NULL)
                       : FJ_Factory::createObject("fourj.FJ_Slider_Normal", m_pRenderNode, true, NULL));
    if (m_labelLists.size() > static_cast<unsigned int>(id) && m_labelLists[static_cast<unsigned int>(id)].size() != 0)
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id, minValue, maxValue, value, repeat, repeatDelay, repeatStep);
}

void FJ_List_MultiList::addNewItem_TextInput(const std::wstring& label, int id, int charLimit)
{
    FJ_TextInput* item =
        m_checkboxTick ? FJ_Factory::createObject("fourj.TextInput.FJ_TextInput_Small", m_pRenderNode, true, NULL)
                             ->asFJ_TextInput()
                       : FJ_Factory::createObject("fourj.TextInput.FJ_TextInput_Normal", m_pRenderNode, true, NULL)
                             ->asFJ_TextInput();
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetCharLimit(charLimit);
}

void FJ_List_MultiList::HandleCharInput(unsigned int character)
{
    FJ_TextInput* input = ReturnItem(GetActiveItem())->asFJ_TextInput();
    if (input != NULL)
        input->HandleDirectInput(character);
}

void FJ_List_MultiList::SetCheckBox(int index, bool checked)
{
    FJ_CheckBox* checkBox = ReturnItem(index)->asFJ_CheckBox();
    if (checkBox != NULL)
        checkBox->SetCheckBox(checked);
}

uint8_t FJ_List_MultiList::GetCheckBox(int index)
{
    FJ_CheckBox* checkBox = ReturnItem(index)->asFJ_CheckBox();
    if (checkBox != NULL)
        return checkBox->getChecked();
    return 0;
}

void FJ_List_MultiList::SetSliderValue(int index, int value)
{
    FJ_Slider* slider = ReturnItem(index)->asFJ_Slider();
    if (slider != NULL)
        slider->SetSliderValue(value);
}

uint32_t FJ_List_MultiList::GetSliderValue(int index)
{
    FJ_Slider* slider = ReturnItem(index)->asFJ_Slider();
    uint32_t value = 0;
    if (slider != NULL)
        value = slider->getSliderCurrent();
    return value;
}

void FJ_List_MultiList::SetSecondaryLabel(int index, const std::wstring& label, bool html, bool autoSize)
{
    FJ_ListButtonController* button = ReturnItem(index)->asFJ_ListButtonController();
    if (button != NULL)
        button->SetSecondaryText(label, html, autoSize);
}

void FJ_List_MultiList::ToggleButtonOutline(int index, bool show)
{
    FJ_Button* button = ReturnItem(index)->asFJ_Button();
    if (button != NULL)
        button->ToggleOutline(show);
}

void FJ_List_MultiList::SetPossibleLabels(int index, std::vector<std::wstring> labels)
{
    if (m_labelLists.size() <= static_cast<unsigned int>(index))
        m_labelLists.resize(index + 1);
    m_labelLists[index] = labels;
}
