#include "game/ui/UIControl_LeftIconButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButtonIconLeft.hpp"
#include "game/ui/UIControl_List.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_LeftIconButtonList::UIControl_LeftIconButtonList()
{
}

bool UIControl_LeftIconButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_LeftIconButtonList;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_LeftIconButtonList::UseIggyIcons()
{
    static_cast<FJ_List_ListButtonIconLeft*>(m_fuiRenderNode->getFuiNode())->UseIggyIcons();
}

void UIControl_LeftIconButtonList::AddNewItem(const std::wstring& label, int id, const std::wstring& textureName)
{
    {
        const std::wstring textureName1(L"");
        static_cast<FJ_List_ListButtonIconLeft*>(m_fuiRenderNode->getFuiNode())
            ->addNewItem(label, id, textureName, textureName1);
    }
    addElement(UIControl::eUIControlType_Button, -1);
}

void UIControl_LeftIconButtonList::AddNewItem(const std::wstring& label, int id, const std::wstring& textureName,
                                              const std::wstring& textureName1)
{
    static_cast<FJ_List_ListButtonIconLeft*>(m_fuiRenderNode->getFuiNode())
        ->addNewItem(label, id, textureName, textureName1);
    addElement(UIControl::eUIControlType_Button, -1);
}
