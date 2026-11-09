#include "game/ui/UIControl_CheckboxButtonList.hpp"

#include "game/ui/FJ_List_ListButtonCheckbox.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_CheckboxButtonList::UIControl_CheckboxButtonList()
{
}

bool UIControl_CheckboxButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_CheckboxButtonList;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_CheckboxButtonList::UseIggyIcons()
{
    static_cast<FJ_List_ListButtonCheckbox*>(m_fuiRenderNode->getFuiNode())->UseIggyIcons();
}

void UIControl_CheckboxButtonList::AddNewItem(const std::wstring& label, int id, bool checked,
                                              const std::wstring& textureName)
{
    FJ_List_ListButtonCheckbox* list = static_cast<FJ_List_ListButtonCheckbox*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem(label, id, checked, textureName);
    addElement(static_cast<UIControl::eUIControlType>(21), -1);
}

void UIControl_CheckboxButtonList::SetCheckbox(int index, bool checked)
{
    static_cast<FJ_List_ListButtonCheckbox*>(m_fuiRenderNode->getFuiNode())->SetCheckBox(index, checked);
}

uint8_t UIControl_CheckboxButtonList::GetCheckbox(int index)
{
    return static_cast<FJ_List_ListButtonCheckbox*>(m_fuiRenderNode->getFuiNode())->GetCheckBox(index);
}
