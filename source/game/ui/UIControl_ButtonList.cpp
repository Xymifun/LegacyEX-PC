#include "game/ui/UIControl_ButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButton.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/FJ_List_MenuButton.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_ButtonList::UIControl_ButtonList()
{
}

bool UIControl_ButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_ButtonList;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_ButtonList::addItem(const std::wstring& label, int index)
{
    if (m_scene->m_movie != NULL)
    {
        if (m_fuiRenderNode->getFuiNode()->asFJ_List_ListButton() != NULL)
        {
            m_fuiRenderNode->getFuiNode()->asFJ_List_ListButton()->addNewItem(label, index);
        }
        else if (m_fuiRenderNode->getFuiNode()->asFJ_List_MenuButton() != NULL)
        {
            m_fuiRenderNode->getFuiNode()->asFJ_List_MenuButton()->addNewItem(label, index);
        }
        else if (m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonPlayerList() != NULL)
        {
            m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonPlayerList()->addNewItem(label, index, 1, 1);
        }
        else if (m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard() != NULL)
        {
            m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonScoreBoard()->addNewItem(label, index, 1, 1);
        }
    }

    addElement(UIControl::eUIControlType_Button, -1);
}

void UIControl_ButtonList::addItem(const std::wstring& label)
{
    addItem(label, m_itemIndex);
}

void UIControl_ButtonList::setButtonLabel(int item, const std::wstring& label)
{
    SetItemLabel(item, UIString(label), true);
}
