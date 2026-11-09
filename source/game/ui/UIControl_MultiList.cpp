#include "game/ui/UIControl_MultiList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_MultiList::UIControl_MultiList()
{
}

void UIControl_MultiList::AddNewButton(const std::wstring& label, int id, bool useEllipsis)
{
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem_Button(label, id, useEllipsis);
    UIControl::eUIControlType type = UIControl::eUIControlType_Button;
    addElement(type, id);
}

void UIControl_MultiList::AddNewLabel(const std::wstring& label, int id)
{
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem_Label(label);
    UIControl::eUIControlType type = UIControl::eUIControlType_Label;
    addElement(type, id);
}

void UIControl_MultiList::AddNewCheckbox(const std::wstring& label, int id, bool checked)
{
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem_CheckBox(label, id, checked);
    UIControl::eUIControlType type = UIControl::eUIControlType_CheckBox;
    addElement(type, id);
}

void UIControl_MultiList::AddNewControllerButton(const std::wstring& label, const std::wstring& secondaryLabel,
                                                 bool html, int id)
{
    static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())
        ->addNewItem_ControllerButton(label, secondaryLabel, html, id);
    addElement(UIControl::eUIControlType_Button, id);
}

void UIControl_MultiList::AddNewSlider(const std::wstring& label, int id, int minValue, int maxValue, int value,
                                       bool repeat, int repeatDelay, int repeatStep)
{
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem_Slider(label, id, minValue, maxValue, value, repeat, repeatDelay, repeatStep);
    UIControl::eUIControlType type = UIControl::eUIControlType_Slider;
    addElement(type, id);
}

void UIControl_MultiList::SetCheckboxValue(int id, bool checked, bool immediate)
{
    if (immediate)
    {
        int index = getListIndex(id);
        static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())->SetCheckBox(index, checked);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetChecked(this, id, checked));
}

bool UIControl_MultiList::IsCheckbox(int id)
{
    return istype(UIControl::eUIControlType_CheckBox, id);
}

bool UIControl_MultiList::GetCheckboxValue(int id)
{
    int index = getListIndex(id);
    return static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())->GetCheckBox(index);
}

void UIControl_MultiList::AddNewTextInput(const std::wstring& label, int id, int charLimit)
{
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem_TextInput(label, id, charLimit);
    UIControl::eUIControlType type = UIControl::eUIControlType_TextInput;
    addElement(type, id);
}

uint32_t UIControl_MultiList::GetSliderValue(int id)
{
    int index = getListIndex(id);
    return static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())->GetSliderValue(index);
}

bool UIControl_MultiList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_MultiList;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_MultiList::setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256])
{
    std::vector<std::wstring> possibleLabels;
    for (unsigned int i = 0; i < labelCount; ++i)
        possibleLabels.push_back(std::wstring(labels[i]));

    static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())->SetPossibleLabels(firstLabel, possibleLabels);
}

void UIControl_MultiList::AddNewMenuButton(const std::wstring& label, int id)
{
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem_MenuButton(label, id);
    UIControl::eUIControlType type = UIControl::eUIControlType_Button;
    addElement(type, id);
}

bool UIControl_MultiList::IsSlider(int id)
{
    return istype(UIControl::eUIControlType_Slider, id);
}

void UIControl_MultiList::HighlightItem(unsigned int id, bool update)
{
    int index = getListIndex(id);
    setCurrentSelection(index, update, true);
}

void UIControl_MultiList::SetSliderValue(int id, int value, bool immediate)
{
    if (immediate)
    {
        int index = getListIndex(id);
        static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())->SetSliderValue(index, value);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetSlider(this, id, value));
}

void UIControl_MultiList::SetItemOpacity(int id, float opacity, bool immediate)
{
    if (immediate)
    {
        int index = getListIndex(id);
        if (m_fuiRenderNode != NULL)
        {
            FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
            if (node != NULL)
                static_cast<FJ_List_MultiList*>(node)->SetItemAlpha(index, opacity);
        }
    }
    else
    {
        m_pendingUpdates.push_back(new PendingSetOpacity(this, id, opacity));
    }
}

void UIControl_MultiList::SetSecondaryLabel(int id, UIString label, bool html, bool autoSize, bool immediate)
{
    int index = getListIndex(id);
    const std::wstring& text = label.getString();
    FJ_List_MultiList* list = static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode());
    list->SetSecondaryLabel(index, text, html, autoSize);
}

void UIControl_MultiList::ToggleButtonOutline(int id, bool outline, bool immediate)
{
    if (immediate)
    {
        int index = getListIndex(id);
        static_cast<FJ_List_MultiList*>(m_fuiRenderNode->getFuiNode())->ToggleButtonOutline(index, outline);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetToggleButtonOutline(this, id, outline));
}

UIControl_MultiList::PendingSetChecked::~PendingSetChecked()
{
}

unsigned int UIControl_MultiList::PendingSetChecked::OnTick()
{
    m_list->SetCheckboxValue(m_item, m_checked, true);
    return 1;
}

UIControl_MultiList::PendingSetOpacity::~PendingSetOpacity()
{
}

unsigned int UIControl_MultiList::PendingSetOpacity::OnTick()
{
    m_list->SetItemOpacity(m_item, m_opacity, true);
    return 1;
}

UIControl_MultiList::PendingSetSlider::~PendingSetSlider()
{
}

unsigned int UIControl_MultiList::PendingSetSlider::OnTick()
{
    m_list->SetSliderValue(m_item, m_value, true);
    return 1;
}

UIControl_MultiList::PendingSetToggleButtonOutline::~PendingSetToggleButtonOutline()
{
}

unsigned int UIControl_MultiList::PendingSetToggleButtonOutline::OnTick()
{
    m_list->ToggleButtonOutline(m_item, m_outline, true);
    return 1;
}
