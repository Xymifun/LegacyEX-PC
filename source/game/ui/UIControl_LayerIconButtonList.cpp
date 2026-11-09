#include "game/ui/UIControl_LayerIconButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButtonLayerIcon.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_LayerIconButtonList::UIControl_LayerIconButtonList() : UIControl_List()
{
}

bool UIControl_LayerIconButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_LayerIconButtonList;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_LayerIconButtonList::UseIggyIcons()
{
    static_cast<FJ_List_ListButtonLayerIcon*>(m_fuiRenderNode->getFuiNode())->UseIggyIcons();
}

void UIControl_LayerIconButtonList::AddNewItem(const std::wstring& label, const std::wstring& secondaryText, int id,
                                               const std::wstring& textureName)
{
    static_cast<FJ_List_ListButtonLayerIcon*>(m_fuiRenderNode->getFuiNode())
        ->addNewItem(label, secondaryText, id, textureName);
    addElement(UIControl::eUIControlType_Button, -1);
}

void UIControl_LayerIconButtonList::SetLayerLabel(int index, const std::wstring& label)
{
    static_cast<FJ_List_ListButtonLayerIcon*>(m_fuiRenderNode->getFuiNode())->SetLayerLabel(index, label);
}

void UIControl_LayerIconButtonList::SetTextureName(int index, const std::wstring& textureName)
{
    static_cast<FJ_List_ListButtonLayerIcon*>(m_fuiRenderNode->getFuiNode())->SetTextureName(index, textureName);
}

void UIControl_LayerIconButtonList::SetItem(int index, const std::wstring& label, const std::wstring& layerLabel,
                                            const std::wstring& textureName)
{
    SetItemLabel(index, UIString(label), true);
    SetLayerLabel(index, layerLabel);
    SetTextureName(index, textureName);
}
