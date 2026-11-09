#include "game/ui/UIControl_PlayerButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_PlayerButtonList::PendingAddItem::PendingAddItem(UIControl_PlayerButtonList* list, int type,
                                                           std::wstring label, int playerIcon, int voipIcon)
    : UIControl_List::PendingUpdate<UIControl_PlayerButtonList>(list, type),
      m_label(label),
      m_playerIcon(playerIcon),
      m_voipIcon(voipIcon)
{
}

unsigned int UIControl_PlayerButtonList::PendingAddItem::OnTick()
{
    m_list->addItem(m_label, m_playerIcon, m_voipIcon, true);
    return 1;
}

void UIControl_PlayerButtonList::setVOIPIcon(int item, int icon)
{
    if (m_scene->m_movie == NULL)
        return;

    FJ_List_ListButtonPlayerList* list = static_cast<FJ_List_ListButtonPlayerList*>(m_fuiRenderNode->getFuiNode());
    list->SetVOIPIcon(item, icon + 1);
}

void UIControl_PlayerButtonList::addItem(const std::wstring& label, int playerIcon, int voipIcon, bool immediate)
{
    if (immediate)
    {
        if (m_scene->m_movie != NULL)
        {
            static_cast<FJ_List_ListButtonPlayerList*>(m_fuiRenderNode->getFuiNode())
                ->addNewItem(label, m_itemIndex, playerIcon + 1, voipIcon + 1);
        }
        addElement(UIControl::eUIControlType_Button, -1);
        return;
    }

    m_pendingUpdates.push_back(new PendingAddItem(this, 0, label, playerIcon, voipIcon));
}

bool UIControl_PlayerButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_PlayerButtonList;
    return UIControl_ButtonList::setupControl(scene, node, name);
}

void UIControl_PlayerButtonList::setPlayerIcon(int item, int icon)
{
    if (m_scene->m_movie == NULL)
        return;

    FJ_List_ListButtonPlayerList* list = static_cast<FJ_List_ListButtonPlayerList*>(m_fuiRenderNode->getFuiNode());
    list->SetPlayerIcon(item, icon + 1);
}
