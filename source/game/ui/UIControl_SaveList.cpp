#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButtonIconLeft.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

bool UIControl_SaveList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_SaveList;
    return UIControl_ButtonList::setupControl(scene, node, name);
}

void UIControl_SaveList::addItem(const std::wstring& label, const std::wstring& textureName, int index)
{
    std::wstring textureName1 = L"";
    FJ_List_ListButtonIconLeft* list = static_cast<FJ_List_ListButtonIconLeft*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem(label, m_itemIndex, textureName, textureName1);
}

void UIControl_SaveList::addItem(const std::wstring& label, const std::wstring& textureName)
{
    addItem(label, textureName, m_itemIndex);
    addElement(UIControl::eUIControlType_Button, -1);
}

void UIControl_SaveList::setTextureName(int item, const std::wstring& textureName)
{
    FJ_List_ListButtonIconLeft* list = static_cast<FJ_List_ListButtonIconLeft*>(m_fuiRenderNode->getFuiNode());
    list->SetTextureName(item, textureName);
}

void UIControl_SaveList::addItem(const std::wstring& label)
{
    addItem(label, L"");
}
