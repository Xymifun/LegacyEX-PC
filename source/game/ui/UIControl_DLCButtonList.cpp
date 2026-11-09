#include "game/ui/UIControl_DLCButtonList.hpp"
#include "game/ui/FJ_List_ListButtonDLC.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/StringConversion.hpp"

bool UIControl_DLCButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_DLCButtonList;
    return UIControl_ButtonList::setupControl(scene, node, name);
}

void UIControl_DLCButtonList::addItem(const std::string& label, bool enabled, int index)
{
    std::wstring wideLabel = convStringToWstring(std::string(label.c_str()));
    FJ_List_ListButtonDLC* list = static_cast<FJ_List_ListButtonDLC*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem(wideLabel, index, enabled);
    addElement(UIControl::eUIControlType_Button, index);
}

void UIControl_DLCButtonList::showTick(int item, bool show)
{
    FJ_List_ListButtonDLC* list = static_cast<FJ_List_ListButtonDLC*>(m_fuiRenderNode->getFuiNode());
    list->ShowTick(item, show);
}
